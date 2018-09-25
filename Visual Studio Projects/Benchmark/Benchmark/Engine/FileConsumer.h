#pragma once
// ***********************************************************************
// Assembly         : Engine
// Author           : Bob
// Created          : 06-21-2014
//
// Last Modified By : Bob
// Last Modified On : 07-06-2014
// ***********************************************************************
// <copyright file="FileConsumer.h" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************
#include <Windows.h>
#include "abstractconsumer.h"

/// <summary>
/// The Engine namespace.
/// </summary>
namespace Engine 
{

	/// <summary>
	/// Class FileConsumer.  This is a concrete consumer that takes bytes out of
	/// a buffer and puts them into a file
	/// </summary>
	class FileConsumer :
		public AbstractConsumer
	{
	protected:
		/// <summary>
		/// The file
		/// </summary>
		HANDLE File;
	public:
		FileConsumer(CircularBuffer<UINT8> *source, int bytesPerRead, HANDLE file, bool *abortFlag);
		virtual ~FileConsumer(void);

		virtual void Consume();
	};
}

