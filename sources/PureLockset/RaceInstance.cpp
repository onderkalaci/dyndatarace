/*
 * RaceInstance.cpp
 *
 *  Created on: May 9, 2013
 *      Author: onder
 */

#include "RaceInstance.h"
#include <sstream>
#include <math.h>
//extern Logger logger;
RaceInstance::RaceInstance(string fileNameIn, UINT32 lineIn, ADDRINT memoryAddrIn, ADDRINT insPtrIn, UINT32 addressSizeIn, ADDRINT stackPtrI)
{



	exactFileName = fileNameIn;
	exactLineNumber = lineIn;
	memoryAddr = memoryAddrIn;
	insPtr = insPtrIn;
	print = true;
	stackPtr = stackPtrI;
	addressSize = addressSizeIn;
}

void RaceInstance::fillLineNumberInfo()
{
	string fileName;
	int lineNumber,col;
    PIN_LockClient();
    PIN_GetSourceLocation(insPtr, &col, &lineNumber, &fileName);
    PIN_UnlockClient();

   this->exactFileName = fileName;
   this->exactLineNumber = lineNumber;
}

string RaceInstance::toStringSimple()
{

    ostringstream os;
    os << endl << "-----------------------RACE INFO STARTS-----------------------------" << endl;
    os << "The Exact Place:"  << exactFileName << "@" << exactLineNumber   << endl;
    os << "-----------------------RACE INFO ENDS-----------------------------" << endl;

	return os.str();
}


string RaceInstance::toStringDetailed()
{
    ostringstream os;
    os << endl << "-----------------------RACE INFO STARTS-----------------------------:" << hex << memoryAddr  << dec << endl;
    os << "The Exact Place:"  << exactFileName << "@" << exactLineNumber << "   stackPtr:" << hex << stackPtr << dec   << endl;
    os << "Difference:" << abs((INT64)(stackPtr - memoryAddr)) << endl;

	os << "-----------------------RACE INFO ENDS-----------------------------" << endl;
	return os.str();
}
