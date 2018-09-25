// ProgramCPLD.cpp : This program looks for a "BootLoader" FX3 then loads ProgramCPLD.img
//	  onto it, then downloads the requested .xsvf file which is programmed into the CPLD.
//
//	You should click, drag and drop an .xsvf file onto ProgramCPLD.exe
//
//	John@usb-by-example.com

#include "stdafx.h"

unsigned char FileBuffer[128 * 1024];		// All images so far have been about 70KB

int _tmain(int argc, _TCHAR* argv[])
{
	CCyUSBDevice *USBDevice;
	CCyFX3Device *FX3Device;
	HANDLE DeviceHandle, FileHandle;
	int i, Success, Seconds, Count;
	int BulkLoopDevice = -1;
	DWORD FileSize;
	LONG BytesWritten;
	BOOL Continue = false;
	char* ExtensionPtr;

	printf("\nProgramCPLD V0.4\n");

	// Get the mame of the file that needs to be downloaded
	if (argc != 2) printf("\nUsage: ProgramCPLD <filename.xsvf>");
	else {
		FileHandle = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (FileHandle == INVALID_HANDLE_VALUE) printf("\nCould not open %s", argv[1]);
		else {
			ExtensionPtr = strstr(argv[1], ".xsvf");
			if (ExtensionPtr == 0) printf("\nCPLD files should have a .xsvf extension");
			else {
				Success = ReadFile(FileHandle, FileBuffer, sizeof(FileBuffer), &FileSize, 0);
				if (!Success) printf("\nCould not read from %s", argv[1]);
				else {
					if (FileSize == sizeof(FileBuffer)) printf("\nInternal Buffer too small, increase!");
					else {
						printf("\n%d bytes read from %s\n", FileSize, argv[1]);
						Continue = true;
						}
					}
				}
			CloseHandle(FileHandle);
			}
		}
	if (Continue) {
		// Look for a BootLoader device
		FX3_FWDWNLOAD_ERROR_CODE DownloadStatus = FAILED;
		for (Seconds = 30; Seconds > 0; Seconds--)
		{
			if (Seconds == 20) printf("If SuperSpeed Explorer Board is connected, press it's RESET button\n");
			FX3Device = new CCyFX3Device();
			printf("Waiting for a BootLoader %d  \r", Seconds);
			Count = FX3Device->DeviceCount();
			for (i = 0; i<Count; i++)
			{
				if (FX3Device->Open(i))
				{
					if (FX3Device->IsBootLoaderRunning())
					{
						printf("Waiting for a BootLoader found, downloading");
						DownloadStatus = FX3Device->DownloadFw("CPLD_Programmer.img", RAM);
					}
					FX3Device->Close();
				}
			}
			Sleep(1000);
			delete FX3Device;
			if (DownloadStatus == SUCCESS) break;
		}
		if (DownloadStatus != SUCCESS) printf("\nFirmware download failed (%d)", DownloadStatus);
		else
		{
			// Wait for the FX3 to come back as a Bulk Loop device (0x04B4, 0x00F0)
			USBDevice = new CCyUSBDevice(&DeviceHandle, CYUSBDRV_GUID, true);
			BytesWritten = 0;
			for (Seconds = 0; Seconds<10; Seconds++)
			{
				for (i = 0; i<USBDevice->DeviceCount(); i++)
				{
					USBDevice->Open(i);
//					printf("\nSeconds = %d, Device = %d, VID = %X, PID = %X", Seconds, i, USBDevice->VendorID, USBDevice->ProductID);
					if ((USBDevice->VendorID == 0x04B4) && (USBDevice->ProductID == 0x00F0))
					{
						BytesWritten = (LONG)FileSize;
						if (USBDevice->BulkOutEndPt) Success = USBDevice->BulkOutEndPt->XferData(FileBuffer, BytesWritten);
						if (Success) printf("\nProgramming");
					}
					USBDevice->Close();
				}
				if (BytesWritten) break;
				Sleep(1000);
			}
		}
	}
	// Wait for programming to be complete - FX3 board will be RESET on completion, so look for a BootLoader device again
	for (Seconds = 30; Seconds > 0; Seconds--)
	{
		FX3Device = new CCyFX3Device();
		printf(".");
		Count = FX3Device->DeviceCount();
		for (i = 0; i<Count; i++)
		{
			if (FX3Device->Open(i))
			{
				if (FX3Device->IsBootLoaderRunning())
				{
					printf("Complete\n");
					Seconds = 0;	// Force EXIT
				}
				FX3Device->Close();
			}
		}
		Sleep(1000);
		delete FX3Device;
	}

	printf("\nUse CR to EXIT\n");
	// The DOS box typically exits so fast that the developer doesn't see any messages
	// Hold the box open until the user enters a character, any character
	while (!_kbhit()) { }

	return 0;
}

