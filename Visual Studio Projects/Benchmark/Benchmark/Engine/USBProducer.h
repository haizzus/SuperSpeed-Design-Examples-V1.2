#pragma once
#include <list>
#include "abstractproducer.h"
#include "CyAPI.h"
#include "OverlappedIO.h"

namespace Engine 
{


	class USBProducer :
		public AbstractProducer
	{
	protected:
		CCyUSBEndPoint *EndPt;
		int Queues; 
		int PPX;
		std::list<OverlappedIO *> overlaps;

	public:
		USBProducer(CircularBuffer<UINT8> *sink, CCyUSBEndPoint *EndPt, int queues, int packetsPerTransfer, bool *abortFlag);
		virtual ~USBProducer(void);

		virtual void Produce();
	};
}

