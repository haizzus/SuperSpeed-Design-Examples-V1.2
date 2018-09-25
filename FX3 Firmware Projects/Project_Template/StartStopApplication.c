/*
 * StartStopApplication.c - this is where resources such as Endpoints and DMA channels, required by the Application
 * 							are created and deleted in response to events from USB
 *
 *  Created on: Feb 18, 2014
 *      Author: John
 */

#include "Application.h"

// Declare external data
extern CyU3PEvent glApplicationEvent;			// An Event to allow threads to communicate


const char* BusSpeed[] = { "Not Connected ", "Full ", "High ", "Super" };


void StartApplication(void)
// USB has been enumerated, time to start the application running
{
    // Display the enumerated device bus speed
    CyU3PDebugPrint(4, "\r\nRunning at %sSpeed\r\n", BusSpeed[CyU3PUsbGetSpeed()]);

    // This is where Endpoints are initialized and DMA channels are set up

    // Now signal the Application so that it can run
    CyU3PEventSet(&glApplicationEvent, USB_CONNECTION_ACTIVE, CYU3P_EVENT_OR);
}

void StopApplication(void)
// USB connection has been lost, time to stop the application running
{
	CyU3PDebugPrint(4, "\r\nStopping application");

    // Signal the Application so that it can stop
    CyU3PEventSet(&glApplicationEvent, ~USB_CONNECTION_ACTIVE, CYU3P_EVENT_AND);

    // Close down and disable the endpoints then close the DMA channels
}

