#include <memory>
#include "FileProducer.h"

namespace Engine
{

	/// <summary>
	/// Initializes a new instance of the <see cref="FileProducer"/> class.
	/// </summary>
	/// <param name="sink">The sink.</param>
	/// <param name="file">The file.</param>
	/// <param name="bytesPerWrite">The bytes per write.</param>
	/// <param name="abortFlag">The abort flag.</param>
	FileProducer::FileProducer(CircularBuffer<UINT8> *sink, HANDLE file, int bytesPerWrite, bool *abortFlag) :
		AbstractProducer(sink, bytesPerWrite, abortFlag),
		File(file)
	{
	}


	/// <summary>
	/// Finalizes an instance of the <see cref="FileProducer"/> class.
	/// </summary>
	FileProducer::~FileProducer(void)
	{
		if(File)
			CloseHandle(File);
	}

	/// <summary>
	/// Produces this instance.
	/// </summary>
	void FileProducer::Produce()
	{
		int loops =0;
		std::unique_ptr<UINT8> WriteBuffer(new UINT8[BytesPerWrite]);
		do {
			DWORD bytesActuallyRead = 0;
			ReadFile(File, WriteBuffer.get(), BytesPerWrite, &bytesActuallyRead, 0);
			if(bytesActuallyRead)
			{
				loops++;
				DWORD result = MySink->Write(WriteBuffer.get(), WriteBuffer.get() + bytesActuallyRead, 1000);
				if(!result)
					AddProducedUnits(bytesActuallyRead);
			}
			else
			{
				SetFilePointer(File, 0, 0, FILE_BEGIN);
			}
		}
		while(!Abort());
	}
}