/*
 * ThreadLocalData.cpp
 *
 *  Created on: Jun 11, 2013
 *      Author: onder
 */
#include "ThreadLocalData.h"
extern  TLS_KEY tls_key;

/*
 * currentVectorClock => threads vector clock
 * currentReaderLocksetId, currentWriterLocksetId => lockset ids
 * next* => these values are utilized when for instance a lock,mallock,barrier
 *  function calls are executed, which are explained in pthreadcallinstrumentation.cpp
 *
 * */
ThreadLocalData::ThreadLocalData(THREADID tid)
{
	currentVectorClock = new VectorClock(tid);
	currentReaderLocksetId = 0;
	currentWriterLocksetId = 0;
	nextWaitCondVariableAddr = 0;
	nextBarrierAddr = 0;
	currentSegment = NULL;
	totalSegmentCount = 0;
	InitLock(&threadLock);
	createNewSegment = false;
	isAlive = true;
	nextMallocSize = 0;
	nextReallocAddr = 0;
	nextReallocSize = 0;
	nextSemaphoreAddr = 0;
}


ThreadLocalData* getTLS(THREADID tid)
{
	return static_cast<ThreadLocalData*>(PIN_GetThreadData(tls_key,tid));
}


