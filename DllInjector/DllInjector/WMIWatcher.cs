﻿using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Management; 
using System.Security.Principal;
using System.Threading;
using Mono.Options;

namespace Injector
{
    class WMIWatcher
    {
        private static List<string> processList = new List<string>();
        private static string dllPath;

        static void Main(string[] args)
        {
            RunThisAsAdmin(string.Join(" ", args));

            OptionSet option_set = new OptionSet()
            {
                {"p=|process=", "Process name to protect", v => {processList.Add(v.ToLower()); } },
                {"d=|dll=", "Path to dll to inject", v => { dllPath = v; } }
            };

            try
            {
                option_set.Parse(args);

                new Thread(WaitForProcess) { IsBackground = true, Name = "worker" }.Start();
                Console.WriteLine("Waiting for process events");
                do
                {
                    Thread.Sleep(5000);
                } while (true);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Failed to setup injector ${ex.Message}");
            }

            
        }

        private static void RunThisAsAdmin(string args)
        {
            if (!IsAdministrator())
            {
                var exe = Process.GetCurrentProcess().MainModule.FileName;
                var startInfo = new ProcessStartInfo(exe, args)
                {
                    UseShellExecute = true,
                    Verb = "runas",
                    WindowStyle = ProcessWindowStyle.Normal,
                    CreateNoWindow = false
                };
                Process.Start(startInfo);
                Process.GetCurrentProcess().Kill();
            }
        }
        private static bool IsAdministrator()
        {
            var identity = WindowsIdentity.GetCurrent();
            var principal = new WindowsPrincipal(identity);
            return principal.IsInRole(WindowsBuiltInRole.Administrator);
        }

        static void WaitForProcess()
        {
            try
            {
                var startWatch = new ManagementEventWatcher(new WqlEventQuery("SELECT * FROM Win32_ProcessStartTrace"));
                startWatch.EventArrived += new EventArrivedEventHandler(startWatch_EventArrived);
                startWatch.Start();
                Console.ForegroundColor = ConsoleColor.Green;
                Console.WriteLine("+ Started Process in GREEN");

                var stopWatch = new ManagementEventWatcher(new WqlEventQuery("SELECT * FROM Win32_ProcessStopTrace"));
                stopWatch.EventArrived += new EventArrivedEventHandler(stopWatch_EventArrived);
                stopWatch.Start();
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("- Stopped Process in RED");
                Console.WriteLine();
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Yellow;
                Console.WriteLine(ex);
            }
        }

        static void stopWatch_EventArrived(object sender, EventArrivedEventArgs e)
        {
            var proc = GetProcessInfo(e);
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine("{0} ({1}) {2} [{3}]", proc.ProcessName, proc.PID, proc.CommandLine, proc.User);
        }

        static void startWatch_EventArrived(object sender, EventArrivedEventArgs e)
        {
            try
            {
                var proc = GetProcessInfo(e);
                if (processList.Contains(proc.ProcessName.ToLower()))
                {
                    Console.ForegroundColor = ConsoleColor.Green;
                    Console.WriteLine("Injecting process - {0} ({1}) {2} [{3}]", proc.ProcessName, proc.PID, proc.CommandLine, proc.User);
                    DllInject.Inject(proc.PID, dllPath);
                }
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Yellow;
                Console.WriteLine(ex);
            }
        }

        static ProcessInfo GetProcessInfo(EventArrivedEventArgs e)
        {
            var p = new ProcessInfo();
            var pid = 0;
            int.TryParse(e.NewEvent.Properties["ProcessID"].Value.ToString(), out pid);
            p.PID = pid;
            p.ProcessName = e.NewEvent.Properties["ProcessName"].Value.ToString();
            try
            {
                using (var searcher = new ManagementObjectSearcher("SELECT * FROM Win32_Process WHERE ProcessId = " + pid))
                using (var results = searcher.Get())
                {
                    foreach (ManagementObject result in results)
                    {
                        try
                        {
                            p.CommandLine += result["CommandLine"].ToString() + " ";
                        }
                        catch { }
                        try
                        {
                            var user = result.InvokeMethod("GetOwner", null, null);
                            p.UserDomain = user["Domain"].ToString();
                            p.UserName = user["User"].ToString();
                        }
                        catch { }
                    }
                }
                if (!string.IsNullOrEmpty(p.CommandLine))
                {
                    p.CommandLine = p.CommandLine.Trim();
                }
            }
            catch (ManagementException) { }
            return p;
        }

        internal class ProcessInfo
        {
            public string ProcessName { get; set; }
            public int PID { get; set; }
            public string CommandLine { get; set; }
            public string UserName { get; set; }
            public string UserDomain { get; set; }
            public string User
            {
                get
                {
                    if (string.IsNullOrEmpty(UserName))
                    {
                        return "";
                    }
                    if (string.IsNullOrEmpty(UserDomain))
                    {
                        return UserName;
                    }
                    return string.Format("{0}\\{1}", UserDomain, UserName);
                }
            }
        }
    }
}

