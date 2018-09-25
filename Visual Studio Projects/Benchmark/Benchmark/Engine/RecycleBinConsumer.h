#pragma once
// ***********************************************************************
// Assembly         : Engine
// Author           : Bob Beauchaine
// Created          : 06-15-2014
//
// Last Modified By : Bob
// Last Modified On : 07-06-2014
// ***********************************************************************
// <copyright file="RecycleBinConsumer.h" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************
#include <memory>
#include "abstractconsumer.h"

/// <summary>
/// The Engine namespace.
/// </summary>
namespace Engine 
{
	/// <summary>
	/// Class RecycleBinConsumer.  Takes data out of a buffer and sends it into the void
	/// </summary>
	class RecycleBinConsumer :
		public AbstractConsumer
	{
	public:
		RecycleBinConsumer(CircularBuffer<UINT8> *source, int BytesPerRead, bool *abortFlag);
		virtual ~RecycleBinConsumer(void);

		virtual void Consume();
	};
}

