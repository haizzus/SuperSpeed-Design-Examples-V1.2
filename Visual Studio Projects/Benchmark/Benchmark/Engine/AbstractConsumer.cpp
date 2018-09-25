#include "AbstractConsumer.h"
#include <Windows.h>

namespace Engine 
{

	/// <summary>
	/// Initializes a new instance of the <see cref="AbstractConsumer"/> class.
	/// </summary>
	/// <param name="source">The source.</param>
	/// <param name="bytesPerRead">The bytes per read.</param>
	/// <param name="abortFlag">The abort flag.</param>
	AbstractConsumer::AbstractConsumer(CircularBuffer<UINT8> * source, int bytesPerRead, bool *abortFlag) : 
		MySource(source), 
		BytesPerRead(bytesPerRead), 
		UnitsConsumed(0),
		AbortFlag(abortFlag)
	{
	}


	/// <summary>
	/// Finalizes an instance of the <see cref="AbstractConsumer"/> class.
	/// </summary>
	AbstractConsumer::~AbstractConsumer(void)
	{
	}
	
	/// <summary>
	/// Totals the consumption.
	/// </summary>
	/// <returns>long long.</returns>
	long long AbstractConsumer::TotalConsumption() const
	{
		return UnitsConsumed;
	}

	/// <summary>
	/// Adds the consumed units.
	/// </summary>
	/// <param name="thisProductionRun">The this production run.</param>
	void AbstractConsumer::AddConsumedUnits(long long thisProductionRun)
	{
		InterlockedAdd64(&UnitsConsumed, thisProductionRun);
	}

	/// <summary>
	/// Aborts this instance.
	/// </summary>
	/// <returns>bool.</returns>
	bool AbstractConsumer::Abort() const
	{
		return *AbortFlag;
	}

}
