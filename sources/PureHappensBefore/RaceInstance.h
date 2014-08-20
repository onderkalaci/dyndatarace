/*
 * RaceInstance.h
 *
 *  Created on: May 9, 2013
 *      Author: onder
 */

#ifndef RACEINSTANCE_H_
#define RACEINSTANCE_H_

#include "definitions.h"


class RaceInstance
{



public:
	RaceInstance(string fileName, UINT32 lineNumber, ADDRINT memoryAddr, ADDRINT insPtr, UINT32 addressSizeIn,ADDRINT stackPtrIn);
	string toStringSimple();
	string toStringDetailed();
	ADDRINT memoryAddr;
	ADDRINT insPtr;
	void fillLineNumberInfo();

	string exactFileName;
	int exactLineNumber;
	ADDRINT stackPtr;


	UINT32 addressSize;
	BOOL print;

};


#endif /* RACEINSTANCE_H_ */
