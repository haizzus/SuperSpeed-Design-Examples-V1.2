#include <algorithm>
#include <cassert>
#include "OverlappedIO.h"

namespace Engine
{
	/// <summary>
	/// Initializes a new instance of the <see cref="OverlappedIO"/> class.
	/// </summary>
	/// <param name="endPt">The end pt.</param>
	/// <param name="length">The length.</param>
	/// <param name="ppx">The PPX.</param>
	/// / The length
	OverlappedIO::OverlappedIO(CCyUSBEndPoint *endPt, int length, int ppx) : 
		buffer(new UINT8[length]), PPX(ppx), Length(length), context(0), EndPt(endPt)
	{
		overlap.hEvent = CreateEvent(0, false, false, 0);
	}

	/// <summary>
	/// Finalizes an instance of the <see cref="OverlappedIO"/> class.
	/// </summary>
	OverlappedIO::~OverlappedIO()
	{
		AbortTransfer();
		CloseHandle(overlap.hEvent);
		overlap.hEvent = 0;
	}

	/// <summary>
	/// The end pt
	/// </summary>
	/// <returns>bool.</returns>
	bool OverlappedIO::BeginTransfer()
	{
		assert(!context);
		if(!context)
		{
			context = EndPt->BeginDataXfer(buffer.get(), Length, &overlap);
			if(!EndPt->NtStatus && !EndPt->UsbdStatus) // BeginDataXfer failed
				transferBegun = true;

			return transferBegun;
		}
		else
			return false;		// last transfer was never cleaned up
	}

	/// <summary>
	/// Begins the transfer.
	/// </summary>
	/// <param name="b">The attribute.</param>
	/// <param name="e">The decimal.</param>
	/// <returns>bool.</returns>
	bool OverlappedIO::BeginTransfer(UINT8 *b, UINT8 *e)
	{
		assert(!context);

		if(!context)
		{
			if(b && e)
				FillBuffer(b, e);

			return BeginTransfer();
		}
		else 
			return false;
	}

	/// <summary>
	/// Waits for transfer.
	/// </summary>
	/// <returns>bool.</returns>
	bool OverlappedIO::WaitForTransfer()
	{
		if(context)
		{
			ULONG TimeOut = 2000;
			bool retVal = EndPt->WaitForXfer(&overlap, TimeOut);
			return retVal;
		}
		else
			return true;	// No transfer was active
	}

	/// <summary>
	/// Finishes the transfer.
	/// </summary>
	/// <param name="rLen">Length of the argument.</param>
	/// <returns>bool.</returns>
	bool OverlappedIO::FinishTransfer(long & rLen)
	{
		if(context)
		{
			bool retVal = EndPt->FinishDataXfer(buffer.get(), rLen, &overlap, context);
			context = 0;
			return retVal;
		}
		else
		{
			rLen = 0;
			return true;	
		}
	}

	/// <summary>
	/// Aborts the transfer.
	/// </summary>
	void OverlappedIO::AbortTransfer()
	{
		if(!context)
			return;

		if (!EndPt->WaitForXfer(&overlap, 100)) 
		{
			EndPt->Abort();
			if (EndPt->LastError == ERROR_IO_PENDING)
				WaitForSingleObject(overlap.hEvent, INFINITE);
		}
	}

	/// <summary>
	/// Fills the buffer.
	/// </summary>
	/// <param name="begin">The begin.</param>
	/// <param name="end">The end.</param>
	void OverlappedIO::FillBuffer(PUCHAR begin, PUCHAR end)
	{
		assert(!context);
		int64_t elementsToCopy = min(end - begin, Length);
		memcpy(buffer, begin, elementsToCopy);
	}

	/// <summary>
	/// Enforces the valid PPX.
	/// </summary>
	/// <param name="PPX">The PPX.</param>
	/// <param name="EndPt">The end pt.</param>
	/// <returns>int.</returns>
	int EnforceValidPPX(int PPX, const CCyUSBEndPoint &EndPt)
	{
		// Limit total transfer length to 4MByte
		int len = ((EndPt.MaxPktSize) * PPX);

		int maxLen = 0x400000;  //4MByte
		if (len > maxLen)
		{

			PPX = maxLen / (EndPt.MaxPktSize);
			if((PPX%8)!=0)
				PPX -= (PPX%8);
		}

		return PPX;
	}
}
