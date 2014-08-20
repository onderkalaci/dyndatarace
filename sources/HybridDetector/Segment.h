/*
 * Segment.h
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */

#ifndef SEGMENT_H_
#define SEGMENT_H_

#include "LockSet.h"
#include "VectorClock.h"
#include <unordered_map>
#include <map>

class Segment
{

public:
	ADDRINT insPtr;
	VectorClock* clock;//segment'in clock'u sabit olacak, pointer ama new olusturacagiz
	THREADID ownerThreadId;
	//LockSet writerLockSet, readerLockSet;//thread Segment'e geldiginde, thread'in O ANKI lockset'leri buraya koyulacak.

	UINT32 writerLockSetId, readerLockSetId;
	Segment();
	~Segment();
	Segment(UINT32 threadsWriterLockSetId, UINT32 threadsReaderLockSetId, VectorClock* currentThreadsClock, THREADID tid, ADDRINT insPtrIn);
	void setWriterLockSetId(UINT32 threadsWriterLockSetId);
	void setReaderLockSetId(UINT32 threadsReaderLockSetId);
	Segment& operator=(Segment* s);
	Segment(const Segment& s);
    int segmentId;
	string file;
	int line;
	//map<ADDRINT, ADDRINT> writeMemoryAddresses;
	//map<ADDRINT, ADDRINT> readMemoryAddresses;
	PIN_LOCK segmentLock;
	BOOL segmentIsActive;
	BOOL segmentInfoFilled;
	string imageName;
	void deactivateSegment();
	static int totalSegmentCount;
	static int deletedSegmentCount;
	UINT32 referenceCount;
	BOOL inAvaliableQueue;


	void fillLineAndFileInfo(ADDRINT insPtrIn, const char* imageName);
	Segment(UINT32 threadsWriterLockSetId, UINT32 threadsReaderLockSetId, VectorClock* currentThreadsClock,
			THREADID threadId, string fileNameIn, UINT32 lineIn, ADDRINT insPtr);


};

#endif /* SEGMENT_H_ */
