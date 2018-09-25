// CheckData.cpp : A simple Windows console program that steps through a datafile checking for non-monatonic data
//
//	I expect the source file to be clicked, dragged and then dropped onto the Get1MB.exe icon
//	Get1M will create a new file in the same directory with an extension of .1MB rather than .bin
//
//	This program is built with an embedded static MFC library so that it will run run on all Win32 platforms
//
//	john@usb-by-example.com
//
// 
#include "stdafx.h"

DWORD FileBuffer[1024 * 1024 * 10 / 4];		// Process file 10MB at a time
DWORD* FilePtr;
DWORD Value;

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE ReadFileHandle;
	DWORD BytesRead = 1;
	DWORD TotalSamples = 0;
	DWORD CorrectValue = 0;
	DWORD Increment = 1;
	DWORD MissedSamples, i;
	DWORD TotalMissedSamples = 0;
	BOOL FirstTime = true;
	BOOL Success;
	
	printf("\nCheckData V0.3\n");

// Get the name of the file that needs to be checked
	if (argc != 2) printf("\nUsage: CheckData <CollectData.bin>");
	else
	{
		ReadFileHandle = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (ReadFileHandle == INVALID_HANDLE_VALUE) printf("\nCould not open %s", argv[1]);
		else 
		{
			while (BytesRead)
			{
				Success = ReadFile(ReadFileHandle, FileBuffer, sizeof(FileBuffer), &BytesRead, 0);
				for (i=0; i<BytesRead/4; i++)
				{
					if (FirstTime)
					{
						FirstTime = false;
						CorrectValue = FileBuffer[0];
					}
					else CorrectValue++;
					TotalSamples++;
					if (FileBuffer[i] != CorrectValue)
					{
						MissedSamples = FileBuffer[i] - CorrectValue;
						Value = FileBuffer[i];
						FilePtr = &FileBuffer[i];
						TotalMissedSamples += MissedSamples;
						printf("\nGap of %d at index %d", MissedSamples, TotalSamples);
						CorrectValue = FileBuffer[i];		// Fixup for next sample
					}
				}		
			}
			if (TotalSamples) printf("\nDONE\nTotalMissedSamples = %d of %d TotalSamples = %d%%", TotalMissedSamples, TotalSamples, (TotalMissedSamples*100)/TotalSamples);
		}
		CloseHandle(ReadFileHandle);
	}

	printf("\nUse CR to EXIT\n");
	// The DOS box typically exits so fast that the developer doesn't see any messages
	// Hold the box open until the user enters a character, any character
	while (!_kbhit()) { }
	return 0;
}

