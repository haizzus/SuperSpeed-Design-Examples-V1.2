#include "RecycleBinConsumer.h"

namespace Engine 
{

	/// <summary>
	/// Initializes a new instance of the <see cref="RecycleBinConsumer"/> class.
	/// </summary>
	/// <param name="source">The source.</param>
	/// <param name="BytesPerRead">The bytes per read.</param>
	/// <param name="abortFlag">The abort flag.</param>
	RecycleBinConsumer::RecycleBinConsumer(CircularBuffer<UINT8> *source, int BytesPerRead, bool *abortFlag) : 
		AbstractConsumer(source, BytesPerRead, abortFlag)
	{
	}

	/// <summary>
	/// Finalizes an instance of the <see cref="RecycleBinConsumer"/> class.
	/// </summary>
	RecycleBinConsumer::~RecycleBinConsumer(void)
	{
	}

	/// <summary>
	/// Consumes this instance.
	/// </summary>
	void RecycleBinConsumer::Consume()
	{
		std::unique_ptr<UINT8 []> ReadBuffer(new UINT8[BytesPerRead]);

		do {
			DWORD readResult = MySource->Read(ReadBuffer.get(),  BytesPerRead, 1000);
			if(!readResult)
				AddConsumedUnits(BytesPerRead);
		}
		while(!Abort());
	}
 
}