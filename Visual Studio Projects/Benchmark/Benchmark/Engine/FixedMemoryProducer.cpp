#include "FixedMemoryProducer.h"
#include <algorithm>
 
namespace Engine 
{

	/// <summary>
	/// Initializes a new instance of the <see cref="FixedMemoryProducer"/> class.
	/// </summary>
	/// <param name="sink">The sink.</param>
	/// <param name="bytesPerWrite">The bytes per write.</param>
	/// <param name="fixedValue">The fixed value.</param>
	/// <param name="abortFlag">The abort flag.</param>
	FixedMemoryProducer::FixedMemoryProducer(CircularBuffer<UINT8> * sink, int bytesPerWrite, UINT8 fixedValue, bool *abortFlag) :
		AbstractProducer(sink, bytesPerWrite, abortFlag),
		WriteBuffer(new UINT8[bytesPerWrite])
	{
		std::fill(WriteBuffer.get(), WriteBuffer.get() + bytesPerWrite, fixedValue);
	}


	/// <summary>
	/// Finalizes an instance of the <see cref="FixedMemoryProducer"/> class.
	/// </summary>
	FixedMemoryProducer::~FixedMemoryProducer(void)
	{
		WriteBuffer = 0;
	}

	/// <summary>
	/// Produces this instance.
	/// </summary>
	void FixedMemoryProducer::Produce()
	{
		do {
			DWORD result = MySink->Write(WriteBuffer.get(), WriteBuffer.get() + BytesPerWrite, 1000);
			if(!result)
				AddProducedUnits(BytesPerWrite);
		}
		while(!Abort());

		DWORD error = GetLastError();
	}
}


