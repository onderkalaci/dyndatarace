/*
 * MemoryAddr.h
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */

#ifndef MEMORYADDR_H_
#define MEMORYADDR_H_

#include "definitions.h"
#include "VectorClock.h"

class MemoryAddr
{
public:
	MemoryAddr(ADDRINT addr);
	MemoryAddr(ADDRINT addr, BOOL isMoved);
	ADDRINT address;
	VectorClock* writerVectorClock;
	VectorClock* readerVectorClock;
};




#endif /* MEMORYADDR_H_ */
