#include <Windows.h>
#include "AbstractProducer.h"

namespace Engine 
{
	/// <summary>
	/// Initializes a new instance of the <see cref="AbstractProducer"/> class.
	/// </summary>
	/// <param name="sink">The sink.</param>
	/// <param name="bytesPerWrite">The bytes per write.</param>
	/// <param name="abortFlag">The abort flag.</param>
	AbstractProducer::AbstractProducer(CircularBuffer<UINT8> * sink, DWORD bytesPerWrite, bool *abortFlag) : 
		UnitsProduced(0),
		MySink(sink), 
		BytesPerWrite(bytesPerWrite),
		AbortFlag(abortFlag)
	{
	}


	/// <summary>
	/// Finalizes an instance of the <see cref="AbstractProducer"/> class.
	/// </summary>
	AbstractProducer::~AbstractProducer(void)
	{
	}
	
	/// <summary>
	/// Totals the production.
	/// </summary>
	/// <returns>long long.</returns>
	long long AbstractProducer::TotalProduction() const
	{
		return UnitsProduced;
	}

	/// <summary>
	/// Adds the produced units.
	/// </summary>
	/// <param name="thisProductionRun">The this production run.</param>
	void AbstractProducer::AddProducedUnits(int64_t thisProductionRun)
	{
		InterlockedAdd64(&UnitsProduced, thisProductionRun);
	}

	/// <summary>
	/// Aborts this instance.
	/// </summary>
	/// <returns>bool.</returns>
	bool AbstractProducer::Abort() const
	{
		return *AbortFlag;
	}
}
