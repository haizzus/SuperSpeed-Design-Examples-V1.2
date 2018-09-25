//	ConvertData.cpp : A simple Windows console program that converts a binary CollectedData file to a text file
//
//	I expect the source file to be clicked, dragged and then dropped onto the Get1MB.exe icon
//	Get1M will create a new file in the same directory with an extension of .1MB rather than .bin
//
//	Problem is, its pretty huge and Excel can't open it, even if I limit it to < 100MB
//	I recommend using CheckData instead since this programmatically does what I expected the developer to do manually
//
//	This program is built with an embedded static MFC library so that it will run run on all Win32 platforms
//
//	john@usb-by-example.com
//

#include "stdafx.h"

unsigned long FileBuffer[1024 * 1024 * 10 / 4];		// Process file 10MB at a time

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE ReadFileHandle;
	FILE* WriteFilePtr;
	DWORD BytesRead = 1;
	DWORD BytesConverted = 0;
	BOOL Success;
	DWORD i;
	int Limit = 0;
	char* ExtensionPtr;

	printf("\nConvertData V0.3\n");

// Get the mame of the file that needs to be converted
	if (argc != 2) printf("\nUsage: ConvertData <CollectData.bin>");
	else
	{
		ReadFileHandle = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (ReadFileHandle == INVALID_HANDLE_VALUE) printf("\nCould not open %s", argv[1]);
		else 
		{
			// Open the output file, change.bin extention to .txt
			ExtensionPtr = strstr(argv[1], ".bin");
			if (ExtensionPtr == 0) printf("Source Filename is assumed to have a .bin extension");
			else
			{
				strcpy(ExtensionPtr, ".txt");
				WriteFilePtr = fopen(argv[1], "w");
				if (WriteFilePtr == NULL) printf("\nCould not open %s", argv[1]);
				else
				{
					while (BytesRead && (Limit++ < 9))
					{
						Success = ReadFile(ReadFileHandle, FileBuffer, sizeof(FileBuffer), &BytesRead, 0);
						for (i=0; i<BytesRead/4; i++) fprintf(WriteFilePtr, "%d\n", FileBuffer[i]);
						BytesConverted += BytesRead;
						printf("Converted %d bytes\r", BytesConverted);
					
					}
					printf("\nDONE");
					fclose(WriteFilePtr);
				}
			}
		}
		CloseHandle(ReadFileHandle);
	}
	printf("\nUse CR to EXIT\n");
	// The DOS box typically exits so fast that the developer doesn't see any messages
	// Hold the box open until the user enters a character, any character
	while (!_kbhit()) { }
	return 0;
}

