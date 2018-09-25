// ***********************************************************************
// Assembly         : Engine
// Author           : Bob Beauchaine
// Created          : 06-10-2014
//
// Last Modified By : Bob
// Last Modified On : 07-06-2014
// ***********************************************************************
// <copyright file="AbstractConsumer.h" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************
#pragma once
#include <Windows.h>
#include "CircularBuffer.h"

/// <summary>
/// The Engine namespace.
/// </summary>
namespace Engine 
{
	/// <summary>
	/// Class AbstractConsumer.  Provides the interface for all consumers
	/// </summary>
	class AbstractConsumer
	{
	private:
		/// <summary>
		/// Total units consumed since The Beginning of Time
		/// </summary>
		int64_t UnitsConsumed;
		/// <summary>
		/// The abort flag.  This will be set to true by the engine when it's time to go home
		/// </summary>
		bool *AbortFlag;
	protected:
		/// <summary>
		/// My source.  A consumer consumes bytes from a source, and this is it
		/// </summary>
		CircularBuffer<UINT8> *MySource;
		/// <summary>
		/// Bytes per read.  For those consumers that have an option, this tells them now many
		/// bytes should be pulled from the source every iteration.  Some consumers will ignore this value
		/// </summary>
		DWORD BytesPerRead;

		void AddConsumedUnits(int64_t thisProductionRun);

		/// Typically, everyone can share the abort property
		/// This is a readonly (as in, should I abort?) function
		bool Abort() const;
	public:
		AbstractConsumer(CircularBuffer<UINT8> *source, int bytesPerRead, bool *abortFlag);
		virtual ~AbstractConsumer(void);

		/// Ever consumer must provide a virtual Consume() function
		virtual void Consume() = 0;

		/// Other useful functions that should be used by derived classes
		int64_t TotalConsumption() const;
	};
}

