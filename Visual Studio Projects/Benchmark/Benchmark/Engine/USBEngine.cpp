// ***********************************************************************
// Assembly         : Engine
// Author           : Bob Beauchaine
// Created          : 06-10-2014
//
// Last Modified By : Bob
// Last Modified On : 07-06-2014
// ***********************************************************************
// <copyright file="USBEngine.cpp" company="">
//     Copyright (c) . All rights reserved.
// </copyright>
// <summary></summary>
// ***********************************************************************
#include <utility>
#include <msclr/marshal.h>
#include <vcclr.h>
#include "USBEngine.h"
#include "FixedMemoryProducer.h"
#include "USBProducer.h"
#include "USBConsumer.h"
#include "RecycleBinConsumer.h"
#include "FileConsumer.h"
#include "FileProducer.h"

/// <summary>
/// The Engine namespace.
/// </summary>
namespace Engine {

	/// <summary>
	/// Class ConsumerThread.  A net object used to execute any consumer on its own thread
	/// </summary>
	ref class ConsumerThread
	{
	private:
		/// <summary>
		/// My consumer, derived from AbstractConsumer
		/// </summary>
		AbstractConsumer *MyConsumer;
	public:
		/// <summary>
		/// Initializes a new instance of the <see cref="ConsumerThread"/> class.
		/// </summary>
		/// <param name="consumer">The consumer.</param>
		ConsumerThread(AbstractConsumer *consumer) : MyConsumer(consumer) {}

		/// <summary>
		/// Runs the consumer.  Returns after the consumer's abort flag has been set
		/// </summary>
		void Consume()
		{
			MyConsumer->Consume();
		}
	};

	/// <summary>
	/// Class ProducerThread.  A net object used to execute any producer on its own thread
	/// </summary>
	ref class ProducerThread
	{
	private:
		/// <summary>
		/// My producer, derived from AbstractProducer
		/// </summary>
		AbstractProducer *MyProducer;
	public:
		/// <summary>
		/// Initializes a new instance of the <see cref="ProducerThread"/> class.
		/// </summary>
		/// <param name="Producer">The producer.</param>
		ProducerThread(AbstractProducer *Producer) : MyProducer(Producer) {}

		/// <summary>
		/// Runs produce until the abort flag for the producer has been set
		/// </summary>
		void Produce()
		{
			MyProducer->Produce();
		}
	};
    
	CCyUSBDevice * USBEngine::GetCypressDevice()
	{
		if(CypressDevice)
		{
			CypressDevice->Close();
			delete CypressDevice;
		}

		CCyUSBDevice * USBDevice = new CCyUSBDevice();

		if (USBDevice == NULL) 
			return NULL;

		return USBDevice;
	}

	USBEngine::USBEngine()  : running(false), CypressDevice(0)
	{
	}
	
	USBEngine::~USBEngine()
	{
		delete CypressDevice;
	}

	void USBEngine::ReportProgress()
	{
		TimeSpan^ timeout = gcnew TimeSpan(0, 0, 1);
		const int oneMegaByte = 1024*1024;
		long long lastReport = 0;

		while(running)
		{
			System::Threading::Thread::Sleep(*timeout);
			if(running)
			{
				// Watch first factory only, over time, everyone steady states to the same overall throughput
				long long consumption = Factories.begin()->consumer->TotalConsumption()/oneMegaByte;
				fireFactoryUpdate(consumption - lastReport);
				lastReport = consumption;
			}
		}
	}

	void USBEngine::Run()
	{
		if(running)
			return;

		running = true;

		CypressDevice = GetCypressDevice();

		if(DownloadPipe)
			CreateFactories(DownloadPipe);
		if(UploadPipe)
			CreateFactories(UploadPipe);

		for(cliext::vector<ProducerConsumerPair^ >::iterator i = Factories.begin(); i != Factories.end(); ++i)
		{
			ConsumerThread^ c = gcnew ConsumerThread(i->consumer);
			ProducerThread^ p = gcnew ProducerThread(i->producer);
			Thread^ t1 = gcnew Thread(gcnew ThreadStart(c, &ConsumerThread::Consume));
			Thread^ t2 = gcnew Thread(gcnew ThreadStart(p, &ProducerThread::Produce));

			t1->Start();
			t2->Start();

			runningThreads.push_back(t1);
			runningThreads.push_back(t2);
		}

		Thread^ monitor = gcnew Thread(gcnew ThreadStart(this, &USBEngine::ReportProgress));
		monitor->Start();
		runningThreads.push_back(monitor);
	}

	/// <summary>
	/// Creates the factories.
	/// </summary>
	/// <param name="Specs">The specs.</param>
	void USBEngine::CreateFactories(SourceSinkPair^ Specs)
	{

		CircularBuffer<UINT8> *buffer = new CircularBuffer<UINT8>(Specs->bufferSize);

		if(!buffer)
		{
			throw("Insufficient memory to create buffer");
		}

		ProducerConsumerPair^ newPair = gcnew ProducerConsumerPair;

		bool * abortFlag = new bool(false);

		newPair->buffer = buffer;
		newPair->abortFlag = abortFlag;
		HANDLE file;
		IntPtr ptr;
		CCyUSBEndPoint * inEndPoint = CypressDevice ? CypressDevice->BulkInEndPt : 0;
		CCyUSBEndPoint * outEndPoint = CypressDevice ? CypressDevice->BulkOutEndPt : 0;
		DWORD errorCode;

		switch(Specs->producer->producer)
		{
		case ProducerTypes::USBProducer:
			if(!inEndPoint)
				throw("Bad USB in endpoint");
			newPair->producer = new USBProducer(buffer, inEndPoint, LinkParameters->ParallelTransfers, LinkParameters->PacketsPerTransfer, abortFlag);
			break;
		case ProducerTypes::MemoryProducer:
			newPair->producer = new FixedMemoryProducer(buffer, Specs->producer->bytesPerWrite, 0xA0, abortFlag);
			break;
		case ProducerTypes::DiskProducer:
			ptr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(Specs->producer->fileName);
			LPCSTR fileName = (LPCSTR)ptr.ToPointer();
			file = CreateFile(fileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
			if(INVALID_HANDLE_VALUE == file)
			{
				errorCode = GetLastError();
				throw("Failed to open source file for reading");
			}
			newPair->producer = new FileProducer(buffer, file, Specs->producer->bytesPerWrite, abortFlag);
			break;
		}


		switch(Specs->consumer->consumer)
		{
		case ConsumerTypes::USBConsumer:
			if(!outEndPoint)
				throw("Bad USB out endpoint");
			newPair->consumer = new USBConsumer(buffer, outEndPoint, LinkParameters->ParallelTransfers, LinkParameters->PacketsPerTransfer, abortFlag);
			break;
		case ConsumerTypes::MemoryConsumer:
			newPair->consumer = new RecycleBinConsumer(buffer, Specs->consumer->bytesPerRead, abortFlag);
			break;
		case ConsumerTypes::DiskConsumer:
			ptr = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(Specs->consumer->fileName);
			LPCSTR fileName = (LPCSTR)ptr.ToPointer();
			file = CreateFile(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,0);
			if(INVALID_HANDLE_VALUE == file)
			{
				errorCode = GetLastError();
				throw("Failed to open sink file for writing");
			}
			
			newPair->consumer = new FileConsumer(buffer, Specs->consumer->bytesPerRead, file, abortFlag);
			System::Runtime::InteropServices::Marshal::FreeHGlobal(ptr);
			break;
		}

		Factories.push_back(newPair);
	}

	/// <summary>
	/// Stops this instance.
	/// </summary>
	void USBEngine::Stop()
	{
		if(!running)
			return;

		running = false;

		cliext::vector<ProducerConsumerPair^ >::iterator factories;
		for(factories = Factories.begin(); factories != Factories.end(); ++factories)
		{
			*(factories->abortFlag) = true;
		}

		int threadCount = runningThreads.size();
		for(cliext::list<Thread^>::iterator thread = runningThreads.begin(); thread != runningThreads.end(); ++thread)
			(*thread)->Join();

		runningThreads.clear();

		for(factories = Factories.begin(); factories != Factories.end(); ++factories)
		{
			delete *factories;
		}
		Factories.clear();

	}

	/// <summary>
	/// Fires the factory update.
	/// </summary>
	/// <param name="consumedUnits">The consumed units.</param>
	void USBEngine::fireFactoryUpdate(long long consumedUnits)
	{
		factoryEvent(consumedUnits);
	}
}
