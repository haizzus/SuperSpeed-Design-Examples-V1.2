#include <list>
#include <algorithm>
#include <memory>
#include <functional>
#include "OverlappedIO.h"
#include "USBConsumer.h"

namespace Engine 
{

	/// <summary>
	/// Initializes a new instance of the <see cref="USBConsumer"/> class.
	/// </summary>
	/// <param name="source">The source.</param>
	/// <param name="EndPt">The end pt.</param>
	/// <param name="queues">The queues.</param>
	/// <param name="packetsPerTransfer">The packets per transfer.</param>
	/// <param name="abortFlag">The abort flag.</param>
	/// / The queues
	USBConsumer::USBConsumer(CircularBuffer<UINT8> *source, CCyUSBEndPoint *endPt, int queues, int packetsPerTransfer, bool *abortFlag) :
		AbstractConsumer(source, 0, abortFlag),
		EndPt(endPt),
		Queues(queues), 
		PPX(packetsPerTransfer)
	{
	}

	/// <summary>
	/// Finalizes an instance of the <see cref="USBConsumer"/> class.
	/// </summary>
	USBConsumer::~USBConsumer(void)
	{
		long dummyArg;
		std::for_each(overlaps.begin(), overlaps.end(), std::mem_fun(&OverlappedIO::AbortTransfer));
		std::for_each(overlaps.begin(), overlaps.end(), std::bind(&OverlappedIO::FinishTransfer, std::placeholders::_1, std::ref(dummyArg)));
		std::for_each(overlaps.begin(), overlaps.end(), Deleter<OverlappedIO>());
	}

	/// <summary>
	/// Consumes this instance.
	/// </summary>
	/// Function which will be started by the producer thread to create USB data and place it in a circular
	/// buffer for comsumption by another thread.
	void USBConsumer::Consume()
	{

		PPX = EnforceValidPPX(PPX, *EndPt); // Each xfer request will get PPX isoc packets
		long length = EndPt->MaxPktSize * PPX;

		// Tell the driver how big a transfer to use.
		EndPt->SetXferSize(length);

		// Allocate all the overlapped I/O buffers for the queues
		for (int i=0; i< Queues; i++) 
		{
			overlaps.push_back(new OverlappedIO(EndPt, length, PPX));
		} 

		// Queue-up the first batch of transfer requests.  This may stall for a while until 
		// the queue fills
		for(OverlapIter j = overlaps.begin(); j != overlaps.end(); ++j)
		{
			DWORD readResult = MySource->Read((*j)->Buffer(),  length, 1000);
			if(!readResult)
			{
				(*j)->BeginTransfer();
				AddConsumedUnits(length);
			}
		}

		OverlapIter i = overlaps.begin();

		// Run until the producer thread is signaled to die
		while(!Abort())
		{
			long bytesThisTransfer;
			OverlappedIO &io = **i;

			// Wait for each overlapped I/O to finish.  Must be done in order or data will get scrambled.
			if(!io.WaitForTransfer())
			{
				io.AbortTransfer();
			}

			// Complete the overlapped I/O and update the producer state, then write
			// the contents to our circular queue.
			// bytesThisTransfer might wind up being less than requested if the driver decides
			// to transfer less
			if(io.FinishTransfer(bytesThisTransfer))
				AddConsumedUnits(bytesThisTransfer);
						
			DWORD readResult = ERROR_TIMEOUT;	
			while(readResult)
			{
				readResult = MySource->Read(io.Buffer(), length, 1000);
				if(Abort())
					break;
			}

			if(!Abort())
				io.BeginTransfer();

			i++;

			if(i == overlaps.end())
				i = overlaps.begin();
		}
	}
}
