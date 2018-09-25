#pragma once
// ***********************************************************************
// Assembly         : Engine
// Author           : Bob
// Created          : 06-10-2014
//
// Last Modified By : Bob
// Last Modified On : 07-06-2014
// ***********************************************************************
// <copyright file="FixedMemoryProducer.h" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************
#include <memory>
#include "abstractproducer.h"

/// <summary>
/// The Engine namespace.
/// </summary>
namespace Engine {

	/// <summary>
	/// Class FixedMemoryProducer.  A producer that takes a constant value and
	/// continuously pushes it into its buffer
	/// </summary>
	class FixedMemoryProducer :
		public AbstractProducer
	{
	protected:
		/// <summary>
		/// The write buffer.  Pre-allocated 
		/// </summary>
		std::unique_ptr<UINT8[]> WriteBuffer;
	public:
		FixedMemoryProducer(CircularBuffer<UINT8> *sink, int bytesPerWrite, UINT8 fixedValue, bool *abortFlag);
		virtual ~FixedMemoryProducer(void);

		virtual void Produce();
	};

}
