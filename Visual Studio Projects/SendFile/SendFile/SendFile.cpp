// SendFile.cpp : This program looks for a "Streamer" FX3 device then sends a file to it
//
//	You should click, drag and drop the file onto SendFile.exe
//
//	John@usb-by-example.com

#include "stdafx.h"

unsigned char FileBuffer[16 * 1024];		// Send the file in 16KB chunks

int _tmain(int argc, _TCHAR* argv[])
{
	CCyUSBDevice *USBDevice;
	CCyFX3Device *FX3Device;
	HANDLE DeviceHandle, FileHandle;
	int i, Success, Seconds;
	int BulkLoopDevice = -1;
	DWORD BytesRead, TotalBytes;
	LONG BytesWritten;
	BOOL Found = false;

	printf("\nSendFile V0.3\n");

	// Get the name of the file that needs to be downloaded
	if (argc != 2) printf("\nUsage: SendFile <filename>");
	else
	{
		FileHandle = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (FileHandle == INVALID_HANDLE_VALUE) printf("\nCould not open %s", argv[1]);
		else
		{
			Success = ReadFile(FileHandle, FileBuffer, sizeof(FileBuffer), &BytesRead, 0);
			if (!Success) printf("\nCould not read from %s", argv[1]);
			else
			{
				TotalBytes = BytesRead;
				// Look for a Streamer device
				USBDevice = new CCyUSBDevice(&DeviceHandle, CYUSBDRV_GUID, true);
				printf("\nLooking for a Streamer device " );
				for (Seconds = 0; Seconds<10; Seconds++)
				{
					for (i = 0; i<USBDevice->DeviceCount(); i++)
					{
						USBDevice->Open(i);
						if ((USBDevice->VendorID == 0x04B4) && (USBDevice->ProductID == 0x00F1))
						{
							printf("found\n" );
							Found = true;
							while (BytesRead)
							{
								BytesWritten = (LONG)BytesRead;
								USBDevice->BulkOutEndPt->XferData(FileBuffer, BytesWritten);
								TotalBytes += BytesWritten;
								printf("Sent %d bytes to FX3\r", TotalBytes);
								ReadFile(FileHandle, FileBuffer, sizeof(FileBuffer), &BytesRead, 0);
							}
							CloseHandle(FileHandle);
							Seconds = 10; // Force Exit
						}
						USBDevice->Close();
					}
					Sleep(1000);
					if (!Found) printf(". ");
				}
				if (Found) printf("\n"); else printf("\nCould not find a Streamer device");
			}
		}
	}

	printf("\nUse CR to EXIT\n");
	// The DOS box typically exits so fast that the developer doesn't see any messages
	// Hold the box open until the user enters a character, any character
	while (!_kbhit()) { }

	return 0;
}
