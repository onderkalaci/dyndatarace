/*
 * ThreadLocalData.cpp
 *
 *  Created on: Jun 11, 2013
 *      Author: onder
 */
#include "ThreadLocalData.h"
extern  TLS_KEY tls_key;

ThreadLocalData::ThreadLocalData(THREADID tid)
{
	currentVectorClock = new VectorClock(tid);
	nextLockAddr = 0;
	nextWaitCondVariableAddr = 0;
	nextBarrierAddr = 0;
	InitLock(&threadLock);
	isAlive = true;
	nextMallocSize = 0;
	nextReallocAddr = 0;
	nextWaitCondVariableLockAddr = 0;
	nextSemaphoreAddr = 0;
}


ThreadLocalData* getTLS(THREADID tid)
{
	ThreadLocalData* tld = static_cast<ThreadLocalData*>(PIN_GetThreadData(tls_key,tid));
	return tld;
}


