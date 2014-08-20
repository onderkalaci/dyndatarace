/*
 * MemoryAddr.h
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */

#ifndef MEMORYADDR_H_
#define MEMORYADDR_H_

#include "definitions.h"
#include "SegmentSet.h"

class MemoryAddr
{
public:
	MemoryAddr(ADDRINT addr);

	ADDRINT address;
	ADDRINT mallocAddress;
	BOOL isMallocedTested;
	BOOL alreadyRaced;

	SegmentSet writerSegmentSet;
	SegmentSet readerSegmentSet;
	BOOL isSegmentInReaderSegmentSet(Segment *s);
	BOOL isSegmentInWriterSegmentSet(Segment *s);
	VOID deleteSegmentsNotInAvaliableQueue();

};




#endif /* MEMORYADDR_H_ */
