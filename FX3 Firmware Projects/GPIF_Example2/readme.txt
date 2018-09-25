
                        	USB Design By Example
                                    FX3 SDK

EXAMPLE 1: Hello World
----------------------

  This example illustrates the use of the FX3 firmware APIs to implement
  a simple 'Hello World' example including both Console Out and In

  The device does not enumerate, it just talks to an attached UART Console.

  Files:

    1 cyfx_gcc_startup.S*	: Start-up code for the ARM-9 core on the FX3 device

    2 HelloWorld.h 			: Constant definitions for this application

    3 HelloWorld.c     		: C source file containing the example

    4 cyfxtx.c*				: ThreadX RTOS wrappers and utility functions required
      by the FX3 API library.

    5 makefile*				: GNU make compliant build script for compiling this example

    
* there should be no need to edit these files but please look what is there!

