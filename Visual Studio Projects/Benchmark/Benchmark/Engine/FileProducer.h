#pragma once
// ***********************************************************************
// Assembly         : Engine
// Author           : Bob Beauchaine
// Created          : 06-21-2014
//
// Last Modified By : Bob
// Last Modified On : 06-22-2014
// ***********************************************************************
// <copyright file="FileProducer.h" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************
#include "abstractproducer.h"

/// <summary>
/// The Engine namespace.
/// </summary>
namespace Engine
{
	/// <summary>
	/// Class FileProducer.  This is a concrete producer class that reads bytes from
	/// a file and shoves them into a buffer
	/// </summary>
	class FileProducer :
		public AbstractProducer
	{
	protected:
		/// <summary>
		/// The file
		/// </summary>
		HANDLE File;
	public:
		FileProducer(CircularBuffer<UINT8> *sink, HANDLE file, int bytesPerWrite, bool *abortFlag);
		virtual ~FileProducer(void);

		virtual void Produce();
	};

}

