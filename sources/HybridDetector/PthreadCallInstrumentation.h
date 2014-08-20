/*
 * PthreadCallInstrumentation.h
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */

#ifndef PTHREADCALLINSTRUMENTATION_H_
#define PTHREADCALLINSTRUMENTATION_H_


#include "definitions.h"
#include "VectorClock.h"

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
VOID CondWaitAnalysisEnter(ADDRINT arg1, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID CondWaitAnalysisExit(ADDRINT arg1, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID CondBCastAnalysis(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID CondSignalAnalysis(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID assignVectorClockToWaitQueue(ADDRINT condVarAddr, THREADID id, VectorClock* senderVClock);
VOID CondWaitAfterAnalysis(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID BarrirerWaitAnalysisExit(ADDRINT barrierAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID BarrirerWaitAnalysisEnter(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr);

VOID FreeEnter(CHAR * name, ADDRINT memoryAddrFreeStarts, THREADID threadid);
VOID freeMemoryAddress(ADDRINT memoryAddrFreeStarts, ADDRINT maxMemoryAddrToBeFreed, THREADID threadid);
VOID moveMemoryAddresses(ADDRINT startOfOldPlace, ADDRINT startOfNewPlace, UINT32 size, THREADID tid);

VOID semWaitAnalysisEnter(ADDRINT semAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID semWaitAnalysisExit(ADDRINT semAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID semPostAnalyis(ADDRINT semAddr, THREADID id ,char* imageName, ADDRINT stackPtr);


void initFunctionNameInstrumentPthread();
void printThreadLockSets(THREADID id);


typedef struct
{
	string functionName;
	AFUNPTR instrumentFunction;
	IPOINT iPoint;
	UINT16 callOrder;

} functionNameInstrumentFunctionName;


//TODO handle lost signal issue
//waitQueueStruct will be used for keeping track of the threads that are waiting for signal
struct signalQueueStruct
{
	ADDRINT condVarAddr;
	VectorClock *clock;
	signalQueueStruct (ADDRINT a, VectorClock* signalersClock) :
		condVarAddr(a)
	{
		clock = signalersClock;// new VectorClock(signalersClock, signalersClock->processId); //clock tmp birsey oldugu icin processId onemli degil, debug amacli
	}
	signalQueueStruct()
	{
		condVarAddr = 0;
		clock = NULL;
	}
};


struct semQueueStruct
{
	ADDRINT semAddr;
	VectorClock *clock;
	semQueueStruct (ADDRINT a, VectorClock* signalersClock) :
		semAddr(a)
	{
		clock = signalersClock;// new VectorClock(signalersClock, signalersClock->processId); //clock tmp birsey oldugu icin processId onemli degil, debug amacli
	}
	semQueueStruct()
	{
		semAddr = 0;
		clock = NULL;
	}
};



//waitQueueStruct will be used for keeping track of the threads that are waiting for signal
struct barrierQueueStruct
{
	ADDRINT barrierAddr;
	THREADID tid;
	barrierQueueStruct (ADDRINT barrierAddrIn, THREADID tidIn) :
		barrierAddr(barrierAddrIn), tid(tidIn)
	{

	}
	barrierQueueStruct()
	{
		barrierAddr = 0;
		tid = 0;
	}
};


#endif /* PTHREADCALLINSTRUMENTATION_H_ */
