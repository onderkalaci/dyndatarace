/*
 * Segment.cpp
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */

#include "Segment.h"

/*
 *
 *
 * ADDRINT insPtr;  instruction pointer of enterence of segment, it is utilzed for extracting line number information of segment.
 *                  if this segment is included in any race
 *
	UINT32 segmentId; //it is mostly used for debug purposes
	VectorClock* clock;//vector clock of a segment
	THREADID ownerThreadId;
	LockSet writerLockSet, readerLockSet; Lockset IDs of a segment
 *
 * */



/*
 * This method is only used in isRace for performance reasons
 * Shallow copy is enough since only read from segment is applied, no updates
 * */
Segment& Segment::operator=(Segment *segmentIn)
{
	//cout << "SEGMENT operator= CALLED****************************************" << endl;
	insPtr = segmentIn->insPtr;
	//segmentId = segmentIn->segmentId;
	clock = segmentIn->clock;
	ownerThreadId = segmentIn->ownerThreadId;
	writerLockSetId = segmentIn->writerLockSetId;
	readerLockSetId = segmentIn->readerLockSetId;
	segmentId = segmentIn->segmentId;
	return *this;
}


int Segment::deletedSegmentCount = 0;
Segment::~Segment()
{
	//cout << "Destructor called:" << this << endl;
	//delete clock yapabiliriz!!!
	++deletedSegmentCount;
	//cout << "readSize:" << readMemoryAddresses.size() << "  writeSize:" << writeMemoryAddresses.size() << endl;
	//writeMemoryAddresses.clear();
	//readMemoryAddresses.clear();
	delete clock;
}

Segment::Segment()
{
	//owner_thread = NULL;
	clock = NULL;
	//cout << "SEGMENT CREATED WITH NULL CLOCK_1:" << endl;
	InitLock(&segmentLock);
	segmentIsActive = true;
	referenceCount = 0;
	inAvaliableQueue = true;
	++totalSegmentCount;
	segmentId = totalSegmentCount;
}


/* When  a race happens, this function is called to get line information of the instruction
 * pointer of the segment's first instruction
 * */
void Segment::fillLineAndFileInfo(ADDRINT insPtrIn, const char* imageNameIn)
{

	//return;
	insPtr = insPtrIn;
	//return;
	//cout << "::::::::fillLineAndFileInfo:::::" << insPtr << endl;
	if (segmentInfoFilled)
		return; //already filled

	imageName = string(imageNameIn);
	segmentInfoFilled = true;
    INT32 col = 0,line =0;
    string file = "";

    PIN_LockClient();
    PIN_GetSourceLocation(insPtr, &col, &line, &file);
    PIN_UnlockClient();

   // cout << "::::::::fillLineAndFileInfo_end:::::" << file <<"  line:" << line << endl;
}


void Segment::setWriterLockSetId(UINT32 threadsWriterLockSetId)
{
	writerLockSetId = threadsWriterLockSetId;
}
void Segment::setReaderLockSetId(UINT32 threadsReaderLockSetId)
{
	readerLockSetId = threadsReaderLockSetId;
}


void Segment::deactivateSegment()
{
	segmentIsActive = false;
	//readMemoryAddresses.clear();
	//writeMemoryAddresses.clear();
}

Segment::Segment(UINT32 threadsWriterLockSetId, UINT32 threadsReaderLockSetId, VectorClock* currentThreadsClock, THREADID tid, ADDRINT insPtrIn)
{
	clock = new VectorClock(currentThreadsClock, tid);
	ownerThreadId = tid;
	insPtr = insPtrIn;
	//cout << "Segment Created thread:" << tid <<  " , with lock count write:" << threadsWriterLockSetId << "   read:" << threadsReaderLockSetId << endl;
	setWriterLockSetId(threadsWriterLockSetId);
	setReaderLockSetId(threadsReaderLockSetId);
	//readMemoryAddresses.reserve(1000);
	//writeMemoryAddresses.reserve(1000);
	InitLock(&segmentLock);
	segmentIsActive = true;
	segmentInfoFilled = false;
	++totalSegmentCount;
	segmentId = totalSegmentCount;
	 referenceCount = 0;
	 inAvaliableQueue = true;

}
int Segment::totalSegmentCount = 0;
Segment::Segment(UINT32 threadsWriterLockSetId, UINT32 threadsReaderLockSetId, VectorClock* currentThreadsClock,
		THREADID threadId, string fileNameIn, UINT32 lineIn, ADDRINT insPtrIn)
{
	clock = new VectorClock(currentThreadsClock, threadId);
	ownerThreadId = threadId;
	//insPtr = insPtrIn;
	//output_stream << "Segment Created thread:" << threadId <<  " , with lock count write:" << threadsWriterLockSet.locks.size() << "   read:" << threadsReaderLockSet.locks.size() << endl;
	setWriterLockSetId(threadsWriterLockSetId);
	setReaderLockSetId(threadsReaderLockSetId);
	file  = fileNameIn;
	line = lineIn;
	insPtr = insPtrIn;

	InitLock(&segmentLock);
	referenceCount = 0;
	segmentIsActive = true;
	segmentInfoFilled = false;
	inAvaliableQueue = true;
	++totalSegmentCount;
	segmentId = totalSegmentCount;

}
