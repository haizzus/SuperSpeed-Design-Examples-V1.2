#include <sstream>
#include <vector>
#include <algorithm>
#include <functional>
#include "USBProducer.h"
#include "OverlappedIO.h"

namespace Engine {



	USBProducer::USBProducer(CircularBuffer<UINT8> *sink, CCyUSBEndPoint *endPt, int queues, int packetsPerTransfer, bool *abortFlag) :
		AbstractProducer(sink, 0, abortFlag),
		EndPt(endPt),
		Queues(queues),
		PPX(packetsPerTransfer)
	{
	}


	USBProducer::~USBProducer(void)
	{
		// Memory clean-up
		long dummyArg;
		std::for_each(overlaps.begin(), overlaps.end(), mem_fun(&OverlappedIO::AbortTransfer));
		std::for_each(overlaps.begin(), overlaps.end(), std::bind(&OverlappedIO::FinishTransfer, std::placeholders::_1, std::ref(dummyArg)));
		std::for_each(overlaps.begin(), overlaps.end(), Deleter<OverlappedIO>());
	}


	/// Function which will be started by the producer thread to create USB data and place it in a circular
	/// buffer for comsumption by another thread.
	void USBProducer::Produce()
	{
		long BytesXferred = 0;

		PPX = EnforceValidPPX(PPX, *EndPt); // Each xfer request will get PPX isoc packets
		long length = EndPt->MaxPktSize * PPX;

		// Tell the driver how big a transfer to use.
		EndPt->SetXferSize(length);

		// Allocate all the overlapped I/O buffers for the queues
		for (int i=0; i< Queues; i++) 
		{
			overlaps.push_back(new OverlappedIO(EndPt, length, PPX));
		}

		// Queue-up the first batch of transfer requests
		std::for_each(overlaps.begin(), overlaps.end(), std::mem_fun(&OverlappedIO::BeginTransfer));

		OverlapIter i = overlaps.begin();

		// Run until the producer thread is signaled to die
		while(!Abort())
		{
			OverlappedIO * io = *i;

			long bytesThisTransfer = length;	// Reset this each time through because
												// FinishDataTransfer may modify it

			// Wait for each overlapped I/O to finish.  Must be done in order or data will get scrambled.
			bool waitSuccess = io->WaitForTransfer();
			if(!waitSuccess)
				io->AbortTransfer();

			// Complete the overlapped I/O and update the producer state, then write
			// the contents to our circular queue.
			// bytesThisTransfer might wind up being less than requested if the driver decides
			// to transfer less
			if (io->FinishTransfer(bytesThisTransfer))
			{			
				DWORD readResult = ERROR_TIMEOUT;	

				if(bytesThisTransfer)
				{
					AddProducedUnits(bytesThisTransfer);
					while(readResult == ERROR_TIMEOUT) 
					{
						readResult = MySink->Write(io->Buffer(), io->Buffer() + bytesThisTransfer, 1000);
						if(Abort())
							break;
					}
				}
			} 

			// Re-submit this queue element to keep the overlapped I/O queue full
			io->BeginTransfer();

			i++;

			if(i == overlaps.end())
				i = overlaps.begin();
		}

	}
}

