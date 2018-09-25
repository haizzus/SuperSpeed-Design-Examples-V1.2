#include <new>
#include <cassert>
#include <cstdint>
#include <Windows.h>

#pragma once

using namespace std;

// Use the following diagram to help you think about a circular buffer:
// B = beginning of allocated buffer memory
// E = end of allocated buffer memory
// H = head of queue - the place where new data will be written
// T = tail of queue - the place from which queued data will be removed
//
// This is an empty queue - head = tail = beginning.  This is the only 
// time the head and tail pointers can be the same
//
// Case 1: T == H
//
// |---------------------------------------------------------------------|
// B,H,T                                                                 E
//
//
// This is a queue with data in it where wrapping has not occurred.  
// Valid data is between the head and tail pointers.
// 
// Case 2: T < H
// 
// |---------------------------------------------------------------------|
// B             T---------------------H                                 E
//
//
// This is a queue with data in it where wrapping has occurred.  
// Valid data is still between the _Tail and _Head, but wraps around the 
// end of the queue
//
// Case 3: T > H
//
// |---------------------------------------------------------------------|
// B--------------------H                     T--------------------------E

template<typename T>
class CircularBuffer 
{
    typedef T * CircularBufferIter;
    typedef const T * ConstCicularBufferIter;

private:
    T * _Begin, * _End;
    T * _Head, * _Tail;
    int64_t _PendingReadSizeRequest;
    int64_t _PendingWriteSizeRequest;

    mutable CRITICAL_SECTION _Lock;
    CONDITION_VARIABLE _QueueNotEmpty;
    CONDITION_VARIABLE _QueueNotFull;


    void CheckQueueConditions();
    CircularBuffer(const CircularBuffer &);

public:
    CircularBuffer(int64_t items);
    virtual ~CircularBuffer();

	virtual DWORD Write(const T * begin, const T * end, DWORD timeout = INFINITE); 
	virtual DWORD Read(T * out,  int64_t elements, DWORD timeout = INFINITE);

    virtual void Clear();

    virtual int64_t ElementsInBuffer() const;
    virtual int64_t SpaceAvailable() const;
};

template<typename T>
CircularBuffer<T>::CircularBuffer(int64_t items) 
	:_PendingReadSizeRequest(0), _PendingWriteSizeRequest(0)
{
    _Begin = new T[items + 1];   // + 1 because head == tail implies an empty buffer, so we need one more slot for data
    _End = _Begin + items + 1;
    _Head = _Tail = _Begin;

    InitializeConditionVariable(&_QueueNotEmpty);
    InitializeConditionVariable(&_QueueNotFull);
    InitializeCriticalSection(&_Lock);

    assert(_Head == _Tail);
    assert(_Head == _Begin);
}

template<typename T>
void CircularBuffer<T>::CheckQueueConditions()
{
    assert(_Head >= _Begin);
    assert(_Head <= _End);
    assert(_Tail >= _Begin);
    assert(_Tail <= _End);
}

template<typename T>
CircularBuffer<T>::~CircularBuffer()
{
    delete[] _Begin;
    _Begin = _End = _Head = _Tail = 0;
}

template<typename T>
DWORD CircularBuffer<T>::Write(const T * begin, const T * end, DWORD timeout = INFINITE)
{
    int64_t elements = end - begin;

    // Write request cannot be larger than the size of the queue
    assert(elements <= static_cast<int64_t>(_End - _Begin));

    bool elementsAvailableForPendingRead = false;

	{ // Critical section locked code
        //AutoLock lock(&_Lock);
        EnterCriticalSection(&_Lock);

        CheckQueueConditions();

        while(SpaceAvailable() < elements)
        {
            _PendingWriteSizeRequest = elements;
            SleepConditionVariableCS(&_QueueNotFull, &_Lock, timeout);
            if(SpaceAvailable() < elements)
            {
                _PendingWriteSizeRequest = 0;
                LeaveCriticalSection(&_Lock);
                return ERROR_TIMEOUT;
            }
        } 

		_PendingWriteSizeRequest = 0;

        // Everything fits in the buffer with no wrap around required, and becuase we checked
        // for space, we know that head will never collide with tail
        if(_Head + elements <= _End)
        {
            assert(_Head + elements <= _End);
            //copy(begin, end, _Head);
            memcpy(_Head, begin, elements * sizeof(T));
            _Head += elements;
            if(_Head == _End)
                _Head = _Begin;
        }
        // Otherwise, we'll have to wrap
        else 
        {
            int64_t partialCount = (_End - _Head);
            assert(partialCount < elements);
            // Write from head to end
            assert(_Head + partialCount == _End);
            //copy(begin, begin + partialCount, _Head);
            memcpy(_Head, begin, partialCount * sizeof(T));

            // Wrap head back to start of queue
            _Head = _Begin;

            // Copy remaining to head and push forward
            assert(_Head + (elements - partialCount) < _Tail);
            //copy(begin + partialCount, end, _Head);
            memcpy(_Head, begin + partialCount, (elements - partialCount) * sizeof(T));
            _Head += (elements - partialCount);
        }

        // Data loss if complete wraparound.  It's a logic error, so flag here
        assert(_Head != _Tail);

        elementsAvailableForPendingRead = (_PendingReadSizeRequest > 0) && (ElementsInBuffer() >= _PendingReadSizeRequest);

        CheckQueueConditions();

        LeaveCriticalSection(&_Lock);
	} // Critical sectionlock unlocks here.  NO MORE ACCESS TO CLASS INSTANCE VARIABLES BELOW THIS LINE!

	if(elementsAvailableForPendingRead)
        WakeConditionVariable(&_QueueNotEmpty);

#ifdef DEBUG
    wstringstream s;
    s << hex << "B: " << _Begin << "  E: " << _End << "  H: " << _Head << "  T: " << _Tail << endl;
	::OutputDebugString(s.str().c_str());
#endif

    return 0;
}

template<typename T>
DWORD CircularBuffer<T>::Read(T * out,  int64_t elements, DWORD timeout = INFINITE)
{
    bool roomForPendingWrite = false;

	{   
		// Critical section locked code
        //AutoLock lock(&_Lock);

        EnterCriticalSection(&_Lock);

        CheckQueueConditions();

        while(ElementsInBuffer() < elements)
		{
			_PendingReadSizeRequest = elements;
			if(false == SleepConditionVariableCS(&_QueueNotEmpty, &_Lock, timeout))
			{
			    _PendingReadSizeRequest = 0;
                LeaveCriticalSection(&_Lock);
                return ERROR_TIMEOUT;
			}
		}

        _PendingReadSizeRequest = 0;
            
        if(static_cast<int64_t>(_End - _Tail) >= elements)
        {
            assert(_Tail + elements <= _End);
            //copy(_Tail, _Tail + elements, out);
            memcpy(out, _Tail, elements * sizeof(T));
            _Tail += elements;
            if(_Tail == _End)
                _Tail = _Begin;
        }
        else
        {
            int64_t firstHalf = _End - _Tail;
            int64_t secondHalf = elements - firstHalf;

            assert(_Tail + firstHalf <= _End);
            //copy(_Tail, _Tail + firstHalf, out);
            memcpy(out, _Tail, firstHalf * sizeof(T));
            _Tail = _Begin;
            assert(_Tail + secondHalf <= _End);
            //copy(_Tail, _Tail + secondHalf, out + firstHalf);
            memcpy(out + firstHalf, _Tail, secondHalf * sizeof(T));
            _Tail += secondHalf;
        }


		roomForPendingWrite = (_PendingWriteSizeRequest > 0) && (SpaceAvailable() >= _PendingWriteSizeRequest);

        CheckQueueConditions();

		LeaveCriticalSection(&_Lock);
	} // Critical sectionlock unlocks here.  NO MORE ACCESS TO CLASS INSTANCE VARIABLES BELOW THIS LINE!

    if(roomForPendingWrite)
		WakeConditionVariable(&_QueueNotFull);

    return 0;
}

template<typename T>
int64_t CircularBuffer<T>::ElementsInBuffer() const
{
    int64_t elements = 0;

    EnterCriticalSection(&_Lock);

    // Case 1
    if(_Tail == _Head)
        elements = 0;
    // Case 3
	else if (_Tail > _Head) 
        elements = (_End - _Tail) + (_Head - _Begin);
    // Case 2
	else
        elements = _Head - _Tail;

    LeaveCriticalSection(&_Lock);

    return elements;
}

template<typename T>
int64_t CircularBuffer<T>::SpaceAvailable() const
{
    int64_t space = 0;
    EnterCriticalSection(&_Lock);

    // Case 1 
    if(_Tail == _Head)
        space = _End - _Begin - 1;
    // Case 3
	else if (_Head < _Tail)
        space = _Tail - _Head - 1;
    // Case 2
	else
        space = (_End - _Head) + (_Tail - _Begin) - 1;

    LeaveCriticalSection(&_Lock);

    return space;
}

template<typename T>
void CircularBuffer<T>::Clear()
{
    EnterCriticalSection(&_Lock);

    // Forced return to case 1
    _Head = _Tail = _Begin;

    LeaveCriticalSection(&_Lock);
}

   
