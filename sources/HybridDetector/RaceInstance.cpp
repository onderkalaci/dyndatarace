/*
 * RaceInstance.cpp
 *
 *  Created on: May 9, 2013
 *      Author: onder
 */

#include "RaceInstance.h"
#include <sstream>
//extern Logger logger;


/*
 * Race Instance Class is utilized for
 * keeping race information in a more compact way
 * Moreover, if two races happen in the same line, one is discarded since it is
 * not logical to output same races
 *
 * Basically a race instance consists of information of the two segments that
 * are involved in the race and the exact file and line information of the
 * potential data race
 *
 * Two types of toString methods exist: First one is simple, the other one is detailed
 * */


UINT64 raceCounter = 0;
RaceInstance::RaceInstance(string fileNameIn, UINT32 lineIn, Segment* s1, Segment* s2, ADDRINT memoryAddrIn, ADDRINT insPtrIn, UINT32 addressSizeIn)
{
	++raceCounter;
	segmentOneFileName = s1->file;
	segmentOneLine = s1->line;
	segmentOneThread = s1->ownerThreadId;
	segmentOneWriterLockset = s1->writerLockSetId;
	segmentOneReaderLockset = s1->readerLockSetId;
	segmentOneInstPtr = s1->insPtr;
	segmentOneImage = s1->imageName;
	segmentTwoFileName = s2->file;
	segmentTwoLine = s2->line;
	segmentTwoThreadId = s2->ownerThreadId;
	segmentTwoWriterLockset = s2->writerLockSetId;
	segmentTwoReaderLockset = s2->readerLockSetId;
	segmentTwoInstPtr = s2->insPtr;
	segmentTwoImage = s2->imageName;

	exactFileName = fileNameIn;
	exactLineNumber = lineIn;
	memoryAddr = memoryAddrIn;
	insPtr = insPtrIn;
	print = true;

	addressSize = addressSizeIn;
}


string RaceInstance::toStringSimple()
{

    ostringstream os;
    os << endl << "-----------------------RACE INFO STARTS-----------------------------" << endl;
    os << "The Race between Threads:"  << segmentOneThread
			<<  "  and " << segmentTwoThreadId << endl;
	os << "The Exact Place:"  << exactFileName << "@" << exactLineNumber   << endl;
	os << "-----------------------RACE INFO ENDS-----------------------------" << endl;

	return os.str();
}


string RaceInstance::toStringDetailed()
{

    ostringstream os;

    os << endl << "-----------------------RACE INFO STARTS-----------------------------:" << hex << memoryAddr  << dec << endl;
       os << "The Race between Threads:"  << segmentOneThread
   			<<  "  and " << segmentTwoThreadId << endl;
   	os << "The Exact Place:"  << exactFileName << "@" << exactLineNumber   << endl;
	os << "The race Happened after the two segments has started:" << endl;

	//first_segment->insPtr
	os << "First Segment In Race:" << segmentOneFileName << "@" << segmentOneLine << endl;
	os << "Second Segment in race Image Name:" << segmentOneImage << endl;
	os << "First Segment Writer Lockset:" << segmentOneWriterLockset << endl;
	os << "First Segment Reader Lockset:" << segmentOneReaderLockset << endl;



	//os << "First Segment Vector Clock:" << *(first_segment->clock) << endl;


	os << "Second Segment in race:" << segmentTwoFileName << "@" << segmentTwoLine << endl;
	os << "Second Segment in race Image Name:" << segmentTwoImage << endl;

	os << "Second Segment Writer Lockset:" << segmentTwoWriterLockset << endl;
	os << "Second Segment Reader Lockset:" << segmentTwoReaderLockset << endl;


	// << "second Segment Vector Clock:" << *(second_segment->clock) << endl;


	os << "-----------------------RACE INFO ENDS-----------------------------" << endl;

	return os.str();
}
