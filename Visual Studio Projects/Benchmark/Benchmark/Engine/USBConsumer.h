#pragma once
// ***********************************************************************
// Assembly         : Engine
// Author           : Bob Beauchaine
// Created          : 06-15-2014
//
// Last Modified By : Bob
// Last Modified On : 07-06-2014
// ***********************************************************************
// <copyright file="USBConsumer.h" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************
#include <list>
#include "abstractconsumer.h"
#include "CyAPI.h"

/// <summary>
/// The Engine namespace.
/// </summary>
namespace Engine 
{
	/// <summary>
	/// Class USBConsumer.  A consumer that takes bytes from a buffer and pipes them out a USB hose
	/// </summary>
	class USBConsumer :
		public AbstractConsumer
	{
	private:
		/// <summary>
		/// The Cypress end point associated with this consumer - that is, the place where bytes are
		/// stuffed once they are read from the buffer
		/// </summary>
		CCyUSBEndPoint *EndPt;
		/// <summary>
		/// The number of parallel overlapped I/O queues to use in streaming data
		/// </summary>
		int Queues; 
		/// <summary>
		/// The PPX
		/// </summary>
		int PPX;
		/// <summary>
		/// The list of OverlappedIO objects to use to realize the asynchronous I/O
		/// </summary>
		std::list<OverlappedIO *> overlaps;
	public:
		USBConsumer(CircularBuffer<UINT8> *source, CCyUSBEndPoint *EndPt, int queues, int packetsPerTransfer, bool *abortFlag);
		virtual ~USBConsumer(void);

		virtual void Consume();
	};
}

