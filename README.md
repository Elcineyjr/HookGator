# HookGator
This is an implementation of a hooking engine for 64 bit processes in a Windows environment

# How does it work?

It monitors the creation of a process (specified in the command line when running the program) through the WMI - Windows Management Instrumentation - Interface.

Upon detection of a new process, the hooking engine will be injected into it.

# What is the hooking engine?

The hooking engine is a DLL whose sole purpose it to apply inline patches into native windows API functions. Thereby, changing the behavior of the API.
