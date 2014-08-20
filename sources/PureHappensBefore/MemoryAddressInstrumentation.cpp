/*
 * MemoryAddressInstrumentation.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */


//#define LOG4CPLUSENABLED
#include "MemoryAddressInstrumentation.h"
#include "MemoryAddr.h"
#include "AllVariables.h"
#include "ThreadLocalData.h"
#include "AllRaceInstances.h"
#define MAX_SEGMENTS_SIZE 1000

ADDRINT STACK_PTR_ERROR = 1000;
AllRaceInstances races;

extern PIN_LOCK segmentDeleteLock;
extern AllMemomryAddresses allMemoryLocations;
extern PIN_LOCK variablePinLocks[1];
INT32 col;
extern map<ADDRINT, ADDRINT> mallocCounts; //map will look like map[memoryAddrStart] = ( malloced bytes size)


#define GET_BIT(x, n) ((x >> (n)) & 0x1 )


//extern UINT64 segmentComparisonCount;
//extern UINT64 memoryAccessCount;
//extern PIN_LOCK segmentCompareLock;

/*
 * The aim is to be able to report race once for each malloced area,
 * This is mainly for reporting races once for each array if there exists
 * */
isMallocAreaStrct findMallocStartAreaIfMallocedArea(ADDRINT effectiveAddr)
{
	//for the time being, do not use mallocCountsLock, bening race for reading mallocCounts map

	isMallocAreaStrct retVal(false, effectiveAddr);
	map<ADDRINT, ADDRINT>::iterator itStart =  mallocCounts.upper_bound(effectiveAddr);

	if (itStart == mallocCounts.begin())
		return retVal;

	--itStart;
	int difference = static_cast<int>(effectiveAddr - (*itStart).first );

	//cout << "findMallocStartAreaIfMallocedArea:" << effectiveAddr << "  first:" << (*itStart).first << "  diff:" << dec << (int)difference << " val:" << (int)(*itStart).second << endl;

	if ( /*difference <= 0 &&  */ abs(difference) <=  (int)(*itStart).second)
	{
		retVal.addrToUse = (*itStart).first;
		retVal.isMallocArea = true;
		retVal.freeSize = (int)(*itStart).second;

	}
	//	else if (itStart == mallocCounts.end() || effectiveAddr < (*itStart).first) //bigger than || smaller than any ids
     //    return effectiveAddr;

	return retVal;
}

/*
 * Decide if the effective address is a global address
 *  (ie in heap or bss or initiliazed vars)
 *  In order to understand this function, refer to the linux process stack
 *
 *  TODO: debug amacli testlerini yap!!
 * */
BOOL isMemoryGlobal(ADDRINT effectiveAddr, ADDRINT stackPtr)
{

	//return true;
	if ( static_cast<UINT64> (abs(stackPtr - effectiveAddr)) > STACK_PTR_ERROR) //if stack pointer is greater, it is global or in heap. ie shared
		return true;

	return false;
}

RaceInstance* isRace(MemoryAddr* currentMemory, ADDRINT insPtr, UINT32 addressSize, BOOL isWrite, VectorClock* threadsClock, ADDRINT stackPtr)
{



	if (currentMemory->writerVectorClock->areConcurrent(threadsClock, threadsClock->processId)
			|| ( isWrite && currentMemory->readerVectorClock->areConcurrent(threadsClock,  threadsClock->processId)  ))
	{


/*
		cout << "race addr:" << hex << currentMemory->address << dec  << "  isWrite:" << (isWrite ? "true" : "false")<< endl;
		cout << "currentMemory->writerVectorClock->areConcurrent(threadsClock):"
				<< currentMemory->writerVectorClock->areConcurrent(threadsClock,  threadsClock->processId) << endl;
		cout << " ( isWrite && currentMemory->readerVectorClock->areConcurrent(threadsClock)  ):" <<
				( isWrite && currentMemory->readerVectorClock->areConcurrent(threadsClock, threadsClock->processId)  ) << endl;

		cout <<"Read Clock:"<< *(currentMemory->readerVectorClock) << endl;
		cout << "Write Clock:" << *(currentMemory->writerVectorClock) << endl;
		cout <<"Thread's clock:"<< *(threadsClock) << endl;

*/

		return new RaceInstance("file", 0, currentMemory->address,insPtr,addressSize, stackPtr);
	}
	return NULL;
}

int deletedSegmentCount = 0;



VOID fillRaceExactLineInfo(RaceInstance* r, ADDRINT insPtr)
{
	string fileName;
	int lineNumber;
    PIN_LockClient();
    PIN_GetSourceLocation(insPtr, &col, &lineNumber, &fileName);
    PIN_UnlockClient();


    r->exactFileName = fileName;
    r->exactLineNumber = lineNumber;


}

BOOL shouldMemoryBeConsidered(ADDRINT effectiveAddr, ADDRINT stackPtr, ADDRINT insPtr)
{

		//cout << "mem global:" << isMemoryGlobal(effectiveAddr, stackPtr)  << "  MemAlreadyRace:" << races.isMemoryAlreadyHasRace(effectiveAddr)
//	<<"  inst race:" << races.isInstructionHasRace(insPtr) << endl;

	if (isMemoryGlobal(effectiveAddr, stackPtr)   && !races.isMemoryAlreadyHasRace(effectiveAddr) /* && !races.isInstructionHasRace(insPtr)*/)
		return true;


	return false;

}

inline VOID MemoryReadInstrumentation(THREADID threadid,
		ADDRINT effectiveAddr,//efecctive addr = memory addr
		ADDRINT stackPtr,	//stackPtr is required for determining whether the addrees is global or not (ie in heap or bss or initiliazed vars)
		const char* imageName,
		ADDRINT insPtr,
		UINT32 readSize
)
{
	//return;
	if (!shouldMemoryBeConsidered( effectiveAddr,  stackPtr, insPtr))
		return;
	ThreadLocalData* tls = getTLS(threadid);
	GetLock(&tls->threadLock, threadid);
	VectorClock* threadsClock = tls->currentVectorClock;
	ReleaseLock(&tls->threadLock);


	//GetLock(&segmentCompareLock, 0);
	//memoryAccessCount = memoryAccessCount + 1;
	//ReleaseLock(&segmentCompareLock);
	GetLock(&variablePinLocks[(effectiveAddr >> 3) & allMemoryLocations.bitMask], threadid);


//	cout << "Read enter:" << hex << effectiveAddr << dec << "  tid:" << threadid << endl;
	variablesHashMap* tmp = allMemoryLocations.getVariableHashMap(effectiveAddr);

	if (tmp == NULL)
	{
		ReleaseLock(&variablePinLocks[(effectiveAddr >> 3) & allMemoryLocations.bitMask]);
		return;
	}

	if (tmp->count(effectiveAddr) == 0)
	{
		(*tmp)[effectiveAddr] = new MemoryAddr(effectiveAddr);
	//	(*tmp)[effectiveAddr]->readerVectorClock->receiveAction(threadsClock);
	}
	//else
	{
		//if ((*tmp)[effectiveAddr]->readerVectorClock->isEmpty())
			//(*tmp)[effectiveAddr]->readerVectorClock->receiveAction(threadsClock);
		//else
			(*tmp)[effectiveAddr]->readerVectorClock->receiveActionFromSpecialPoint(threadsClock, threadid);
	}

	if (!(*tmp)[effectiveAddr]->writerVectorClock->isUniqueValue(threadid) )
	{
		RaceInstance* r = isRace((*tmp)[effectiveAddr], insPtr, readSize, false, threadsClock, stackPtr);
		ReleaseLock(&variablePinLocks[(effectiveAddr >> 3) & allMemoryLocations.bitMask]);

		if (r != NULL)
		{
			isMallocAreaStrct normalizedAddrWithRespectToArrays =  findMallocStartAreaIfMallocedArea(effectiveAddr);
			fillRaceExactLineInfo(r, insPtr);
			races.addToRaces(r, normalizedAddrWithRespectToArrays);
		}
	}
	else
		ReleaseLock(&variablePinLocks[(effectiveAddr >> 3) & allMemoryLocations.bitMask]);

}




inline VOID  MemoryWriteInstrumentation(THREADID threadid,
		ADDRINT effectiveAddr,//efecctive addr = memory addr
		ADDRINT stackPtr,	//stackPtr is required for determining whether the addrees is global or not (ie in heap or bss or initiliazed vars)
		const char* imageName,
		ADDRINT insPtr,
		UINT32 writeSize
		)
{
	//return;

	if (!shouldMemoryBeConsidered( effectiveAddr,  stackPtr, insPtr))
		return;

	//cout << "write:" << hex << effectiveAddr << dec << "  tid:" << threadid << endl;

	ThreadLocalData* tls = getTLS(threadid);

	//cout << *(tls->currentVectorClock) << endl;
	//cout << "------------------------" << endl;


	GetLock(&tls->threadLock, threadid);
	VectorClock* threadsClock = tls->currentVectorClock;
	ReleaseLock(&tls->threadLock);

	//GetLock(&segmentCompareLock, 0);
	//memoryAccessCount = memoryAccessCount + 1;
	//ReleaseLock(&segmentCompareLock);


	GetLock(&variablePinLocks[(effectiveAddr >> 3) & allMemoryLocations.bitMask], threadid);
	variablesHashMap* tmp = allMemoryLocations.getVariableHashMap(effectiveAddr);

	if (tmp == NULL)
	{
		ReleaseLock(&variablePinLocks[(effectiveAddr >> 3) & allMemoryLocations.bitMask]);
		return;
	}

	if (tmp->count(effectiveAddr) == 0)
	{
		(*tmp)[effectiveAddr] = new MemoryAddr(effectiveAddr);
		//(*tmp)[effectiveAddr]->writerVectorClock->receiveAction(threadsClock);
	}
	//else
	{
		//if ((*tmp)[effectiveAddr]->writerVectorClock->isEmpty())
			//(*tmp)[effectiveAddr]->writerVectorClock->receiveAction(threadsClock);
	//	else
			(*tmp)[effectiveAddr]->writerVectorClock->receiveActionFromSpecialPoint(threadsClock, threadid);

	}

	bool shouldCheckRace = true;
	if (((*tmp)[effectiveAddr]->writerVectorClock->isUniqueValue(threadid)) && ((*tmp)[effectiveAddr]->readerVectorClock->isUniqueValue(threadid)) )
		shouldCheckRace = false;
	//if (shouldCheckRace && (*tmp)[effectiveAddr]->readerVectorClock->isEmpty())
		//shouldCheckRace = false;


	if ( shouldCheckRace )
	{
		RaceInstance* r = isRace((*tmp)[effectiveAddr], insPtr, writeSize, true, threadsClock, stackPtr);
		ReleaseLock(&variablePinLocks[(effectiveAddr >> 3) & allMemoryLocations.bitMask]);

		if (r != NULL)
		{
			isMallocAreaStrct normalizedAddrWithRespectToArrays =  findMallocStartAreaIfMallocedArea(effectiveAddr);
			fillRaceExactLineInfo(r, insPtr);
			races.addToRaces(r, normalizedAddrWithRespectToArrays);
		}
		return;
	}


	ReleaseLock(&variablePinLocks[(effectiveAddr >> 3) & allMemoryLocations.bitMask]);
}


/*
 * If an instruction is a write instruction, then process write memory operands
 *
 * biz instruction'u da instrument etmeyecegiz, instruction da bize memory adreslere ulasmamizi saglayacak
 *
 * Process all memory adresses that is written for this particular instruction
 * */

void processMemoryWriteInstruction(INS ins, const char* imageName)
{

	UINT32 memoryOperandCount = INS_MemoryOperandCount(ins);
	for(UINT32 i = 0; i < memoryOperandCount; ++i)
	{
		if (INS_MemoryOperandIsWritten(ins, i) /*  && INS_OperandIsMemory(ins,i)*/)
		{
			INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE, (AFUNPTR)MemoryWriteInstrumentation,
					IARG_THREAD_ID,
					IARG_MEMORYOP_EA, i,
					IARG_REG_VALUE, REG_STACK_PTR, //pass current stack ptr
					IARG_PTR, imageName,
					IARG_INST_PTR,
					IARG_MEMORYWRITE_SIZE,
					IARG_CALL_ORDER, CALL_ORDER_FIRST + 30,
					IARG_END);
			}
	}
}

/*
 * Process all memory adresses that is read for this particular instruction
 * */
void processMemoryReadInstruction(INS ins, const char* imageName)
{

	UINT32 memoryOperandCount = INS_MemoryOperandCount(ins);
	for(UINT32 i = 0; i < memoryOperandCount; ++i)
		if (INS_MemoryOperandIsRead(ins, i)  /*&& INS_OperandIsMemory(ins,i)*/)
		{

			INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE, (AFUNPTR)MemoryReadInstrumentation,
					IARG_THREAD_ID,
					IARG_MEMORYOP_EA, i,
					IARG_REG_VALUE, REG_STACK_PTR,//pass current stack ptr
					IARG_PTR, imageName,
					IARG_INST_PTR,
					IARG_MEMORYWRITE_SIZE,
					IARG_CALL_ORDER, CALL_ORDER_FIRST + 30,
					IARG_END);
		}
	//LOG4CPLUS_TRACE(logger, "EXIT:" << __FUNCTION__ );
	}




// PIN's Trace equals to ThreadSanitizer's Segment, namely, single entrance multiple exists
//Insturemnt Trace,   Trace=>BBL=>Instruction=>Variable
VOID InstrumentSegment(TRACE trace, VOID *v)
{

	IMG img = IMG_FindByAddress(TRACE_Address(trace));

	if (!IMG_Valid(img) || !IMG_IsMainExecutable(img) )
		return;

	const char* imageName = IMG_Name(img).c_str();
	THREADID tid = PIN_ThreadId();

	ThreadLocalData* tls = getTLS(tid);
	cout.flush();
	GetLock(&tls->threadLock, tid);


	//cout << "thread " << tid << "  created new segment, totalCount:" << tls->totalSegmentCount << "  tls->currentSegment :" << tls->currentSegment  << endl;
	ReleaseLock(&tls->threadLock);

	// Visit every basic block  in the trace
	//BBLs are not crucial for us, they are only important for use to reach instructions
	for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
	{
		//biz BBL'i instrument etmeyecegiz. BBL sadece instruction'a ulasmamizi saglayacak
		for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
		{
			processMemoryWriteInstruction(ins, imageName);
			processMemoryReadInstruction(ins, imageName);
		}
	}

}


