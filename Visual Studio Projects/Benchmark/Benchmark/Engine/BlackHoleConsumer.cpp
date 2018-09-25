#include "BlackHoleConsumer.h"

namespace Engine {

	BlackHoleConsumer::BlackHoleConsumer(HANDLE handle, int bytesPerRead) : 
		AbstractConsumer(handle, bytesPerRead), 
		ReadBuffer(new UINT8[bytesPerRead])
	{
	}


	BlackHoleConsumer::~BlackHoleConsumer(void)
	{
		delete[] ReadBuffer;
	}

	void BlackHoleConsumer::Consume()
	{
		DWORD bytesActuallyRead = 0;
		ReadFile(MySource, ReadBuffer, BytesPerRead, &bytesActuallyRead, 0);

		AddConsumedUnits(bytesActuallyRead);
	}
}