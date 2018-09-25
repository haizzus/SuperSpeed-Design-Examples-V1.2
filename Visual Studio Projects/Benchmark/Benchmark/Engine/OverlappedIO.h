#pragma once
// ***********************************************************************
// Assembly         : Engine
// Author           : Bob Beauchaine
// Created          : 06-29-2014
//
// Last Modified By : Bob
// Last Modified On : 07-06-2014
// ***********************************************************************
// <copyright file="OverlappedIO.h" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************
#include <windows.h>
#include <list>
#include <memory>
#include "CyAPI.h"

/// <summary>
/// The Engine namespace.
/// </summary>
namespace Engine
{
	/// <summary>
	/// Struct Deleter.  This is a convenience functor to be used in things like for_each statements
	/// to delete every element in a container of pointers
	/// </summary>
	template<typename T>
	struct Deleter {
		/// <summary>
		/// Operator()s the specified argument.
		/// </summary>
		/// <param name="arg">The argument.</param>
		void operator()(T * arg) { delete arg; }
	};

	/// <summary>
	/// Class OverlappedIO.
	/// </summary>
	class OverlappedIO
	{
	private:
		/// <summary>
		/// The overlaped buffer.  Stores incoming data for an IN endpoint and 
		/// outgoing data for an OUT endpoint
		/// </summary>
		std::unique_ptr<UCHAR []> buffer;
		/// <summary>
		/// The PPX
		/// </summary>
		int PPX;
		/// <summary>
		/// The length of the buffer, in bytes
		/// </summary>
		long Length;
		/// <summary>
		/// The context.  Returned by the Cypress driver when an asychronous I/O operation is started
		/// </summary>
		PUCHAR context;
		/// <summary>
		/// The overlapped structure Windows uses to track this read/write
		/// </summary>
		OVERLAPPED overlap;
		/// <summary>
		/// The Cypress endpoint associated with this I/O
		/// </summary>
		CCyUSBEndPoint *EndPt;
		/// <summary>
		/// Has a transfer (i.e, BeginTransfer()) been started on this object and not yet completed?
		/// Used to automatically track wait and finish operations
		/// </summary>
		bool transferBegun;

		/// Hidden default construction and copy
		OverlappedIO();
		OverlappedIO & operator=(OverlappedIO &);
	public:

		OverlappedIO(CCyUSBEndPoint *endPt, int length, int ppx);
		~OverlappedIO();

		PUCHAR Buffer() { return buffer.get(); }
		void FillBuffer(PUCHAR begin, PUCHAR end);
		bool BeginTransfer();
		bool BeginTransfer(UINT8 *b, UINT8 *e);
		void AbortTransfer();
		bool WaitForTransfer();
		bool FinishTransfer(long & rLen);
	};

	typedef	std::list<OverlappedIO *>::iterator OverlapIter;
	int EnforceValidPPX(int PPX, const CCyUSBEndPoint &EndPt);

}
