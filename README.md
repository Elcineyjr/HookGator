<h1 align="center">HookGator</h1>
<p align="center">
  <i>A hooking engine for 64 bit processes in a Windows environment</i>
  <br/><br/>
  
  <img width="240" src="https://user-images.githubusercontent.com/22310158/180611014-87bcdfac-cae9-46bf-8de1-285a82ed3b52.png" />
  <br/>
</p>

# How does it work?

It monitors the creation of a process (specified in the command line when running the program) through the WMI - Windows Management Instrumentation - Interface.

Upon detection of a new process, the hooking engine will be injected into it.

# What is the hooking engine?

The hooking engine is a DLL whose sole purpose it to apply inline patches into native windows API functions. Thereby, changing the behavior of the API.
