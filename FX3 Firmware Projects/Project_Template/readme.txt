                        	USB Design By Example

Project Template
----------------

  This example contains place holders for the modules needed for a project
  Typically:
  	1, 10 and 11 are not touched
  	2, 3, 6 and 7 are edited once
  	4, 5, 8 and 9 are edited and debugged multiple times to get the project working!

  Files:

    1 cyfx_gcc_startup.S*		: Start-up code for the ARM-9 core on the FX3 device

    2 StartUp.c 				: Initialize the FX3 IO structure and start the RTOS
    
    3 StartStopApplication.c	: StartUp and SpinDown the Application following a USB event

    4 RunApplication.c     		: This does the actual work of your application
    
    5 Application.h				: Declare constants and options here
    
    6 USB_Descriptors.c			: Define how the USB Host will see this device
    
    7 USB_Handler.c				: Respond to USB Events and non-standard USB Requests
    
    8 DebugConsole.c			: Used to help debug your application
    
    9 Support.c					: Put helper routines here so that Application.c is more readable

   10 cyfxtx.c*					: ThreadX RTOS wrappers and utility functions required by the FX3 API library.

   11 makefile*					: GNU make compliant build script for compiling this example

    
* there should be no need to edit these files but please look what is there!

