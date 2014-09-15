
#
# +--------------------------------------------------------------------------+
# |Copyright (C) 2014 Bogazici University                                    |
# |               onderkalaci@gmail.com                                      |
# |                                                                          |
# | This is free software: you can redistribute it and/or modify             |
# | it under the terms of the GNU Lesser General Public License as published |
# | by the Free Software Foundation, either version 2.1 of the License, or   |
# | (at your option) any later version.                                      |
# |                                                                          |
# | This is distributed in the hope that it will be useful,                  |
# | but WITHOUT ANY WARRANTY; without even the implied warranty of           |
# | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            |
# | GNU Lesser General Public License for more details.                      |
# |                                                                          |
# | You should have received a copy of the GNU Lesser General Public License |
# | along with this program.  If not, see <http://www.gnu.org/licenses/>.    | 
# +--------------------------------------------------------------------------+
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
VOID LockInit(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID GetLockAnalysis(ADDRINT arg1, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID releaseLock(ADDRINT lockAddr, THREADID tid);
VOID createNewLock(ADDRINT lockAddr, THREADID tid);
VOID mutexTryLockExitAnalysis(ADDRINT exitVal, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID mutexTryLockEnterAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName , ADDRINT stackPtr);
VOID GetLockAnalysisEnter(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID PthreadJoinAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID PthreadCreateAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID ReleaseLockAnalyis(ADDRINT arg1, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID CondWaitAnalysisEnter(ADDRINT arg1, ADDRINT arg2, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID CondWaitAnalysisExit(ADDRINT arg1, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID CondBCastAnalysis(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID CondSignalAnalysis(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID assignVectorClockToWaitQueue(ADDRINT condVarAddr, THREADID id, VectorClock* senderVClock);
VOID CondWaitAfterAnalysis(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID BarrirerWaitAnalysisExit(ADDRINT barrierAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID BarrirerWaitAnalysisEnter(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr);

VOID MallocAfter(ADDRINT memoryAddrMallocStarts, THREADID tid);
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
struct waitQueueStruct
{
	ADDRINT condVarAddr;
	VectorClock *clock;
	waitQueueStruct (ADDRINT a, VectorClock* signalersClock) :
		condVarAddr(a)
	{
		clock = signalersClock;// new VectorClock(signalersClock, signalersClock->processId); //clock tmp birsey oldugu icin processId onemli degil, debug amacli
	}
	waitQueueStruct()
	{
		condVarAddr = 0;
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


#endif /* PTHREADCALLINSTRUMENTATION_H_ */
