#pragma once 
#include <Windows.h>
#include <cliext/vector>
#include <cliext/list>
#include <cliext/utility>
#include "CyAPI.h"
#include "AbstractConsumer.h"
#include "AbstractProducer.h"
#include "CircularBuffer.h"


using namespace System;
using namespace System::Threading;
using namespace System::IO;
using namespace cliext;

namespace Engine {

	public enum class ProducerTypes {
		USBProducer,
		MemoryProducer,
		DiskProducer
	};

	public enum class ConsumerTypes {
		USBConsumer,
		MemoryConsumer,
		DiskConsumer
	};

	public ref struct Source
	{
		ProducerTypes producer;
		int bytesPerWrite;
		String^ fileName;
	};

	public ref struct Sink 
	{
		ConsumerTypes consumer;
		int bytesPerRead;
		String^ fileName;
	};

	public ref struct USBParameters
	{
		int VendorID;
		int ProductID;
		int PacketsPerTransfer;
		int ParallelTransfers;
	};

	public ref struct SourceSinkPair 
	{
		String^ identifier;
		Source^ producer;
		Sink^ consumer;
		int bufferSize;
	};

	private ref struct ProducerConsumerPair
	{
		AbstractProducer *producer;
		AbstractConsumer *consumer;
		CircularBuffer<UINT8> *buffer;
		bool *abortFlag;

		ProducerConsumerPair() : producer(0), consumer(0), buffer(0), abortFlag(0) {}
		~ProducerConsumerPair() { delete producer; delete consumer; delete buffer; delete abortFlag; producer = 0; consumer = 0; buffer = 0; abortFlag = 0;} 
	};


	public ref class USBEngine
	{
	private:
		vector<ProducerConsumerPair^ > Factories;
		void fireFactoryUpdate(long long consumedUnits);
		cliext::list<Thread^> runningThreads;
		bool running;

		void CreateFactories(SourceSinkPair^ Specs);
		void ReportProgress();
		CCyUSBDevice * CypressDevice;

		CCyUSBDevice * GetCypressDevice();
	public:

		delegate void FactoryUpdate(long long consumedUnits);
		event FactoryUpdate^ factoryEvent;

		property SourceSinkPair^ UploadPipe;
		property SourceSinkPair^ DownloadPipe;
		property USBParameters^ LinkParameters;
		
		USBEngine();
		~USBEngine();

		void Run(); 
		void Stop();
	};
}
