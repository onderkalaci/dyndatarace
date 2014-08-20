/*
 * MemoryAddressInstrumentation.h
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */

#ifndef MEMORYADDRESSINSTRUMENTATION_H_
#define MEMORYADDRESSINSTRUMENTATION_H_

#include "definitions.h"
#include "RaceInstance.h"


BOOL isMemoryGlobal(ADDRINT effectiveAddr, ADDRINT stackPtr);
VOID MemoryReadInstrumentation(THREADID threadid, ADDRINT effectiveAddr, ADDRINT stackPtr, const char* imageName, ADDRINT insPtr, UINT32 readSize);
VOID MemoryWriteInstrumentation(THREADID threadid, ADDRINT effectiveAddr, ADDRINT stackPtr, const char* imageName, ADDRINT insPtr, UINT32 writeSize);
void processMemoryReadInstruction(INS ins);
void processMemoryWriteInstruction(INS ins);
VOID InstrumentSegment(TRACE trace, VOID *v);

#endif /* MEMORYADDRESSINSTRUMENTATION_H_ */