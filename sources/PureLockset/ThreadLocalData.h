/*
 * ThreadLocalData.h
 *
 *  Created on: Jun 11, 2013
 *      Author: onder
 */

#ifndef THREADLOCALDATA_H_
#define THREADLOCALDATA_H_

#include "definitions.h"
#include "LockSet.h"





class ThreadLocalData
{
public:
	ThreadLocalData(THREADID tid);


	UINT32 currentReaderLocksetId;
	UINT32 currentWriterLocksetId;
	ADDRINT nextWriteLockAddr;
	ADDRINT nextReadLockAddr;
	ADDRINT nextWaitCondVariableAddr;
	ADDRINT nextBarrierAddr;
	PIN_LOCK threadLock;
	BOOL isAlive;
	ADDRINT nextMallocSize;
	ADDRINT nextReallocSize;
	ADDRINT nextReallocAddr;


};

ThreadLocalData* getTLS(THREADID tid);


#endif /* THREADLOCALDATA_H_ */
