/*
 * MemoryAddr.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */


#include "MemoryAddr.h"



MemoryAddr::MemoryAddr(ADDRINT addr)
{
	this->address = addr;
	this->writerVectorClock = new VectorClock(NON_THREAD_VECTOR_CLOCK);
	this->readerVectorClock = new VectorClock(NON_THREAD_VECTOR_CLOCK);
}


MemoryAddr::MemoryAddr(ADDRINT addr, BOOL isMoved)
{
	this->address = addr;

	if (!isMoved)
	{
		this->writerVectorClock = new VectorClock(NON_THREAD_VECTOR_CLOCK);
		this->readerVectorClock = new VectorClock(NON_THREAD_VECTOR_CLOCK);
	}
	else
	{
		this->writerVectorClock = NULL;
		this->readerVectorClock = NULL;
	}
}


