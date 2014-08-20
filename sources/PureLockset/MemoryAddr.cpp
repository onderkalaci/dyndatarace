/*
 * MemoryAddr.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */


#include "MemoryAddr.h"



MemoryAddr::MemoryAddr(ADDRINT addr, ADDRINT writerLocksetIn, ADDRINT readerLocksetIn, ADDRINT firstOwnerThread, ADDRINT initialState):
address(addr), writerLocksetId(writerLocksetIn), readerLocksetId(readerLocksetIn), ownerThreadId(firstOwnerThread), currentState(initialState),mallocAddress(addr),  isMallocedTested(false)
{
	/*
	this->address = addr;
	this->writerLocksetId = writerLocksetIn;
	this->readerLocksetId = readerLocksetIn;
	this->currentState = initialState;
	this->ownerThreadId = firstOwnerThread;
	this->mallocAddress = addr;
	this->isMallocedTested = false;
	*/
}


VOID MemoryAddr::updateState(THREADID tid, BOOL isWrite, UINT32 writerLocksetIdIn, UINT32 readerLocksetIdIn)
{

	//if (this->currentState == MEM_STATE_SHARED_MODIFIED)
		//return; //do nothing
	//else
	if (this->currentState == MEM_STATE_READ_SHARED)
	{
		if (isWrite)
			this->currentState = MEM_STATE_SHARED_MODIFIED;
	}
	else if(this->currentState == MEM_STATE_EXCLUSIVE )
	{
		if (this->ownerThreadId != tid)
		{
			if (isWrite)
				this->currentState = MEM_STATE_SHARED_MODIFIED;
			else
				this->currentState = MEM_STATE_READ_SHARED;
		}
	}
	else if(this->currentState == MEM_STATE_VIRGIN)
	{
		if (isWrite)
		{
			this->currentState = MEM_STATE_EXCLUSIVE;
			this->ownerThreadId = tid;
			this->writerLocksetId = writerLocksetIdIn;
			this->readerLocksetId = readerLocksetIdIn;
		}
	}
}

