/*
 * ThreadLocalData.h
 *
 *  Created on: Jun 11, 2013
 *      Author: onder
 */

#ifndef THREADLOCALDATA_H_
#define THREADLOCALDATA_H_

#include "definitions.h"
#include "VectorClock.h"





class ThreadLocalData
{
public:
	ThreadLocalData(THREADID tid);


	VectorClock* currentVectorClock;
	ADDRINT nextLockAddr;
	ADDRINT nextWaitCondVariableAddr;
	ADDRINT nextWaitCondVariableLockAddr;
	ADDRINT nextBarrierAddr;
	ADDRINT nextSemaphoreAddr;

	PIN_LOCK threadLock;
	BOOL isAlive;
	ADDRINT nextMallocSize;
	ADDRINT nextReallocAddr;
	ADDRINT nextReallocSize;

};

ThreadLocalData* getTLS(THREADID tid);


#endif /* THREADLOCALDATA_H_ */
