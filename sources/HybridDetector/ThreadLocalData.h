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
#include "LockSet.h"
#include "Segment.h"





class ThreadLocalData
{
public:
	ThreadLocalData(THREADID tid);


	Segment* currentSegment;
	VectorClock* currentVectorClock;
	UINT32 currentReaderLocksetId;
	UINT32 currentWriterLocksetId;
	ADDRINT nextWriteLockAddr;
	ADDRINT nextReadLockAddr;
	ADDRINT nextWaitCondVariableAddr;
	ADDRINT nextBarrierAddr;
	ADDRINT nextSemaphoreAddr;
	UINT32 totalSegmentCount;
	PIN_LOCK threadLock;
	BOOL createNewSegment;
	BOOL isAlive;
	ADDRINT nextMallocSize;
	ADDRINT nextReallocAddr;
	ADDRINT nextReallocSize;


};

ThreadLocalData* getTLS(THREADID tid);


#endif /* THREADLOCALDATA_H_ */
