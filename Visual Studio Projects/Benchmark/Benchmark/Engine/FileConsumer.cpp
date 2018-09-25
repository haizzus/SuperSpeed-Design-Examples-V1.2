#include <memory>
#include "FileConsumer.h"

namespace Engine
{

	/// <summary>
	/// Initializes a new instance of the <see cref="FileConsumer"/> class.
	/// </summary>
	/// <param name="source">The source.</param>
	/// <param name="bytesPerRead">The bytes per read.</param>
	/// <param name="file">The file.</param>
	/// <param name="abortFlag">The abort flag.</param>
	FileConsumer::FileConsumer(CircularBuffer<UINT8> *source, int bytesPerRead, HANDLE file, bool *abortFlag)  :
		AbstractConsumer(source, bytesPerRead, abortFlag),
		File(file)
	{
	}


	/// <summary>
	/// Finalizes an instance of the <see cref="FileConsumer"/> class.
	/// </summary>
	FileConsumer::~FileConsumer(void)
	{
		if(File)
			CloseHandle(File);
	}

	/// <summary>
	/// Consumes this instance.
	/// </summary>
	void FileConsumer::Consume()
	{
		std::unique_ptr<UINT8> buffer(new UINT8[BytesPerRead]);

		do {
			DWORD readResult = MySource->Read(buffer.get(),  BytesPerRead, 1000);
			if(!readResult)
			{
			    DWORD bytesWritten;
				BOOL writeResult = WriteFile(File, buffer.get(), BytesPerRead, &bytesWritten, 0);
				if(writeResult)
					AddConsumedUnits(bytesWritten);
				else
					int error = GetLastError();
			}
		}
		while(!Abort());
	}
}