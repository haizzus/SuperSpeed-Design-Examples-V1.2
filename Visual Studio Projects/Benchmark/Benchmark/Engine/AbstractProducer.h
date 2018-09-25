#pragma once
// ***********************************************************************
// Assembly         : Engine
// Author           : Bob Beauchaine
// Created          : 06-10-2014
//
// Last Modified By : Bob
// Last Modified On : 07-06-2014
// ***********************************************************************
// <copyright file="AbstractProducer.h" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************
#include <Windows.h>
#include "CircularBuffer.h"

/// <summary>
/// The Engine namespace.
/// </summary>
namespace Engine {

	/// <summary>
	/// Class AbstractProducer.  This is the template from which all real producers derive.  
	/// Producers put bytes into a sink
	/// </summary>
	class AbstractProducer
	{
	private:
		/// <summary>
		/// The units produced since this base class was initialized
		/// </summary>
		int64_t UnitsProduced;
		/// <summary>
		/// The abort flag
		/// </summary>
		bool *AbortFlag;
	protected:
		/// <summary>
		/// My sink - the place into which I place bytes
		/// </summary>
		CircularBuffer<UINT8> *MySink;
		/// <summary>
		/// The bytes per write
		/// </summary>
		DWORD BytesPerWrite;

		bool Abort() const;

		void AddProducedUnits(long long thisProductionRun);
	public:
		AbstractProducer(CircularBuffer<UINT8> *sink, DWORD bytesPerWrite, bool *abortFlag);
		virtual ~AbstractProducer(void);

		/// Every concrete producer must override this
		virtual void Produce() = 0;

		int64_t TotalProduction() const;
	};
}

