/*
 * RaceInstance.h
 *
 *  Created on: May 9, 2013
 *      Author: onder
 */

#ifndef RACEINSTANCE_H_
#define RACEINSTANCE_H_

#include "definitions.h"
#include "Segment.h"


class RaceInstance
{



public:
	RaceInstance(string fileName, UINT32 lineNumber, Segment* s1, Segment* s2, ADDRINT memoryAddr, ADDRINT insPtr, UINT32 addressSizeIn);
	string toStringSimple();
	string toStringDetailed();
	ADDRINT memoryAddr;
	ADDRINT insPtr;

	string exactFileName;
	int exactLineNumber;

	string segmentOneFileName;
	int segmentOneLine;
	THREADID segmentOneThread;
	ADDRINT segmentOneWriterLockset;
	ADDRINT segmentOneInstPtr;
	ADDRINT segmentOneReaderLockset;
	string segmentOneImage;

	string segmentTwoFileName;
	int segmentTwoLine;
	THREADID segmentTwoThreadId;
	ADDRINT segmentTwoWriterLockset;
	ADDRINT segmentTwoInstPtr;
	ADDRINT segmentTwoReaderLockset;
	string segmentTwoImage;

	UINT32 addressSize;
	BOOL print;

};


#endif /* RACEINSTANCE_H_ */
