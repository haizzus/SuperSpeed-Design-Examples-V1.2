//	Get1MB.cpp : A simple Windows console program that extracts the first 1MB from a CollectData.bin file
//	These smaller files are easier to manipulate and check compared with the GB files that are created
//
//	I expect the source file to be clicked, dragged and then dropped onto the Get1MB.exe icon
//	Get1M will create a new file in the same directory with an extension of .1MB rather than .bin
//
//	This program is built with an embedded static MFC library so that it will run run on all Win32 platforms
//
//	john@usb-by-example.com
//

#include "stdafx.h"

DWORD FileBuffer[1024 * 1024 / 4];		// Process file 1MB at a time


int _tmain(int argc, char* argv[])
{
	HANDLE ReadFileHandle, WriteFileHandle;
	DWORD BytesRead, BytesWritten;
	char* CharPtr;
	BOOL Success;

	printf("\nGet1MB V0.3\n");

// Get the mame of the file that needs to be checked
	if (argc != 2) printf("\nUsage: Get1MB <Filename.bin>");
	else
	{
		ReadFileHandle = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (ReadFileHandle == INVALID_HANDLE_VALUE) printf("\nCould not open %s", argv[1]);
		else 
		{
			Success = ReadFile(ReadFileHandle, FileBuffer, sizeof(FileBuffer), &BytesRead, 0);
			if (BytesRead == sizeof(FileBuffer))
			{
				CloseHandle(ReadFileHandle);
				CharPtr = strstr(argv[1], ".bin");
				if (CharPtr == 0) printf("Source Filename is assumed to have a .bin extension");
				else
				{
					strcpy(CharPtr, ".1MB");
					WriteFileHandle = CreateFile(argv[1], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
					if (WriteFileHandle == INVALID_HANDLE_VALUE) printf("\nCould not open %s", argv[1]);
					else
					{
						Success = WriteFile(WriteFileHandle, FileBuffer, sizeof(FileBuffer), &BytesWritten, 0);
						if (BytesWritten == sizeof(FileBuffer)) printf("\n1MB written to %s", argv[1]);
						CloseHandle(WriteFileHandle);
					}
				}
			}
		}
		
	}

	printf("\nUse CR to EXIT\n");
	// The DOS box typically exits so fast that the developer doesn't see any messages
	// Hold the box open until the user enters a character, any character
	while (!_kbhit()) { }
	return 0;
}

