/*
 * PthreadCallInstrumentation.h
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */

#ifndef PTHREADCALLINSTRUMENTATION_H_
#define PTHREADCALLINSTRUMENTATION_H_


#include "definitions.h"

#define libpthread "libpthread"

VOID InstrumentImage(IMG img, VOID *v);
VOID GetWriteLockAnalysis(ADDRINT arg1, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID GetReadLockAnalysisEnter(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID mutexTryLockExitAnalysis(ADDRINT exitVal, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID mutexTryLockEnterAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName , ADDRINT stackPtr);
VOID GetWriteLockAnalysisEnter(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID PthreadJoinAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID PthreadCreateAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID GetReadLockAnalysis(ADDRINT arg1, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID ReleaseLockAnalyis(ADDRINT arg1, THREADID id ,char* imageName, ADDRINT stackPtr);


VOID FreeEnter(CHAR * name, ADDRINT memoryAddrFreeStarts, THREADID threadid);
VOID freeMemoryAddress(ADDRINT memoryAddrFreeStarts, ADDRINT maxMemoryAddrToBeFreed, THREADID threadid);
VOID moveMemoryAddresses(ADDRINT startOfOldPlace, ADDRINT startOfNewPlace, UINT32 size, THREADID tid);

void initFunctionNameInstrumentPthread();
void printThreadLockSets(THREADID id);


typedef struct
{
	string functionName;
	AFUNPTR instrumentFunction;
	IPOINT iPoint;
	UINT16 callOrder;

} functionNameInstrumentFunctionName;




#endif /* PTHREADCALLINSTRUMENTATION_H_ */
