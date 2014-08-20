/*
 * MemoryAddr.h
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */

#ifndef MEMORYADDR_H_
#define MEMORYADDR_H_

#include "definitions.h"

#define MEM_STATE_VIRGIN 0
#define MEM_STATE_EXCLUSIVE 1
#define MEM_STATE_READ_SHARED 2
#define MEM_STATE_SHARED_MODIFIED 3




class MemoryAddr
{
public:
	MemoryAddr(ADDRINT addr, ADDRINT writerLocksetIn, ADDRINT readerLocksetIn, ADDRINT firstOwnerThread, ADDRINT initialState);

	ADDRINT address;
	UINT32 writerLocksetId;
	UINT32 readerLocksetId;
	THREADID ownerThreadId;
	UINT32 currentState;
	ADDRINT mallocAddress;
	BOOL isMallocedTested;
	void updateState(THREADID tid, BOOL isWrite, UINT32 writerLocksetIdIn, UINT32 readerLocksetIdIn);
};




#endif /* MEMORYADDR_H_ */
