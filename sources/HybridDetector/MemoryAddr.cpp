/*
 * MemoryAddr.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */


/*
 * This class is the shadow memory for memory addresses
 *
 */

#include "MemoryAddr.h"
#include "SynronizedQueue.h"

extern SynronizedQueue* syncQueue;
extern   bool syncQueueEnabled;


/**
 * Each Memory address class includes memory address +
 * whether the address is in malloced area or not +
 * if it is already raced or not
 */
MemoryAddr::MemoryAddr(ADDRINT addr)
{
	this->address = addr;
	this->mallocAddress = addr;
	this->isMallocedTested = false;
	this->alreadyRaced = false;
}

BOOL MemoryAddr::isSegmentInReaderSegmentSet(Segment* s)
{
	//cout << "entered" << endl;
	return readerSegmentSet.isSegmentInSet(s);

}

BOOL MemoryAddr::isSegmentInWriterSegmentSet(Segment* s)
{
	return writerSegmentSet.isSegmentInSet(s);
}

VOID MemoryAddr::deleteSegmentsNotInAvaliableQueue()
{
	if (syncQueue->isEnabled()/*syncQueue->enabled*/)
	{
		writerSegmentSet.deleteSegmentsNotInAvaliableQueue();
		readerSegmentSet.deleteSegmentsNotInAvaliableQueue();
	}
}


