#pragma once

#include "AbstractConsumer.h"

namespace Engine {

	class BlackHoleConsumer :
		public AbstractConsumer
	{
	protected:
		UINT8 * ReadBuffer;
	public:
		BlackHoleConsumer(HANDLE handle, int BytesPerRead);
		virtual ~BlackHoleConsumer(void);

		virtual void Consume();
	};

}

