/*
 * USB Handler.c
 *
 */

#include "Application.h"

// Declare external functions
extern void StartApplication(void);


// Spin up USB, let the USB driver handle enumeration
CyU3PReturnStatus_t InitializeUSB(void)
{
	// This is a place holder for the routine that connects to USB
	// It is not used in this example
	// Fake a connection and positive status
	StartApplication();
	return CY_U3P_SUCCESS;
}




