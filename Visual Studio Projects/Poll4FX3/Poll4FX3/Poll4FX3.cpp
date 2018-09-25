// Poll4FX3.cpp : This simple Windows console program polls for 30 seconds looking for an attached FX3 board.
//
//	This program is built with an embedded static MFC library so that it will run run on all Win32 platforms
//
//	john@usb-by-example.com
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CCyUSBDevice *USBDevice;
	int Seconds;

	printf("\nPoll for FX3 device V0.3\n");

	for (Seconds=30; Seconds>0; Seconds--)
	{
		USBDevice = new CCyUSBDevice(NULL, CYUSBDRV_GUID, true);
		if (USBDevice->DeviceCount())
		{
			printf("\nFound %d device(s)", USBDevice->DeviceCount());
			break;
		}
		else
		{
			delete USBDevice;
			printf("%d \r", Seconds);
		}
		Sleep(1000);
	}
	if (Seconds == 0) printf("Sorry, no FX3 devices found");
	printf("\nUse CR to EXIT\n");
	// The DOS box typically exits so fast that the developer doesn't see any messages
	// Hold the box open until the user enters a character, any character
	while (!_kbhit()) { }
	return 0;
}

