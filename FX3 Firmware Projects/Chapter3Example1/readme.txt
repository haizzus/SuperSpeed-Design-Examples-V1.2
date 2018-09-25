
                        	USB Design By Example
                            	FX3 Example

Chapter 3: Example 1
--------------------

  This is the simplest example that I could think of!
  Pressing the button changes the blink rate of the LED

  The device does not attach to USB.

  Files:

    1 cyfx_gcc_startup.S*	: Start-up code for the ARM-9 core on the FX3 device

    2 Application.h 		: Constant definitions for this application

    3 Application.c     	: C source file containing the example

    4 cyfxtx.c*				: ThreadX RTOS wrappers and utility functions required
      						  by the FX3 API library.

    5 makefile*				: GNU make compliant build script for compiling this example

    
* there should be no need to edit these files but please look what is there!

