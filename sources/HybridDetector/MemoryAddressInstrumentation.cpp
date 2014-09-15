
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
 * MemoryAddressInstrumentation.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 *
 *   This file handles read and write
 *   memory accesses instrumentation
 *
 */


//#define LOG4CPLUSENABLED
#include "MemoryAddressInstrumentation.h"
#include "MemoryAddr.h"
#include "AllVariables.h"
#include "ThreadLocalData.h"
#include "AllLockSets.h"
#include "AllRaceInstances.h"
#include "SynronizedQueue.h"
#include "mtrand.h"
#define MAX_SEGMENTS_SIZE 10000

ADDRINT STACK_PTR_ERROR = 100000000;
AllRaceInstances races;

extern map<THREADID, BOOL> deadThreadIds;
extern PIN_LOCK testLock;
extern PIN_LOCK segmentDeleteLock;
extern vector<Segment*> segmentsToBeDeleted;
extern AllMemomryAddresses allMemoryLocations;
extern PIN_LOCK* variablePinLocks;
INT32 col;
extern AllLockSets allLockSets;
extern map<ADDRINT, ADDRINT> mallocCounts; //map will look like map[memoryAddrStart] = ( malloced bytes size)
extern PIN_LOCK mallocCountsLock;
extern UINT64 totalNumberOfHashMaps;
extern BOOL enableSingleAccessCheck;
extern SynronizedQueue* syncQueue;


#define GET_BIT(x, n) ((x >> (n)) & 0x1 )


unsigned long init[4] = {0x1233, 0x2343, 0x3452, 0x4561}, length = 4;
MTRand_int32 irand(init, length); // 32-bit int generator

int sampledCount = 0;
int nonSampledCount = 0;
/*
 * The aim is to be able to report race once for each malloced area,
 * This is mainly for reporting races once for each array if there exists
 * */
isMallocAreaStrct findMallocStartAreaIfMallocedArea(/*isMallocAreaStrct &retVal,*/ ADDRINT effectiveAddr)
{
	//for the time being, do not use mallocCountsLock, bening race for reading mallocCounts map
	isMallocAreaStrct retVal(false, effectiveAddr);


	map<ADDRINT, ADDRINT>::iterator itStart = mallocCounts.upper_bound(effectiveAddr);

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
BOOL isMemoryGlobal(const ADDRINT& effectiveAddr,const ADDRINT& stackPtr)
{

	if ( static_cast<UINT64> (abs(stackPtr - effectiveAddr)) > STACK_PTR_ERROR) //if stack pointer is greater, it is global or in heap. ie shared
		return true;

	return false;
}

//extern UINT64 segmentComparisonCount;
//extern UINT64 memoryAccessCount;
//extern PIN_LOCK segmentCompareLock;


/**
 *  This function gets a memory address
 *  and checks reader-writer segments for potential
 *  data race
 *
 *  Firstly, check if this memory address is element of an array
 *  Then check if this memory address (or array) has already raced?
 *  If not, lastly check for the segment sets
 */
UINT64 isRaceCount = 0;
RaceInstance* isRace(MemoryAddr* currentMemory,const ADDRINT& insPtr, const UINT32& addressSize)
{
	if (currentMemory->isMallocedTested == false) //daha once bakmadi demek
	{
		isMallocAreaStrct normalizedAddrWithRespectToArrays = findMallocStartAreaIfMallocedArea(/*normalizedAddrWithRespectToArrays,*/ currentMemory->address);
		currentMemory->mallocAddress = normalizedAddrWithRespectToArrays.addrToUse;
		currentMemory->isMallocedTested = true;
	}


	if (!races.isRaceOK(currentMemory, insPtr, currentMemory->mallocAddress))
		return NULL;

/*
//++isRaceCount;
	UINT32 writerSize = currentMemory->writerSegmentSet.segments.size();
	UINT32 readerSize = currentMemory->readerSegmentSet.segments.size();
	Segment* writerIndexMemorySegment = NULL;
	Segment* writerInnerIndexMemorySegment = NULL;
	Segment* readIndexMemorySegment = NULL;
	UINT32 writerIndexThreadId;

	for (UINT32 writerIndex = 0 ; writerIndex < writerSize; ++writerIndex)
	{

		writerIndexMemorySegment = currentMemory->writerSegmentSet.segments.at(writerIndex);
		writerIndexThreadId = writerInnerIndexMemorySegment->ownerThreadId;
		for (UINT32 writerInnerIndex = writerIndex + 1; writerInnerIndex < writerSize; ++writerInnerIndex)
		{
			//continue;
			writerInnerIndexMemorySegment = currentMemory->writerSegmentSet.segments.at(writerInnerIndex);
			if (writerIndexThreadId != writerInnerIndexMemorySegment->ownerThreadId
					&&

					allLockSets.isIntersectionEmpty(writerIndexMemorySegment->writerLockSetId,writerInnerIndexMemorySegment->writerLockSetId )

			)

			{

				++isRaceCount;
				//cout << "Race happend:"<< currentMemory->address << "  inst:" << insPtr << endl;
				RaceInstance *r = new RaceInstance("", 0,
						 currentMemory->writerSegmentSet.segments.at(writerIndex),
						currentMemory->writerSegmentSet.segments.at(writerInnerIndex),
						currentMemory->address,
						insPtr,
						addressSize
				);
				currentMemory->alreadyRaced = true;
			   races.addToRaces(r, currentMemory->mallocAddress);
               return r;

			}
		}




		for (UINT32 readerIndex = 0; readerIndex < readerSize ; readerIndex++)
		{
			readIndexMemorySegment = currentMemory->readerSegmentSet.segments.at(readerIndex);
			if (    writerIndexThreadId != readIndexMemorySegment->ownerThreadId
					&&
					allLockSets.isIntersectionEmpty( writerIndexMemorySegment->writerLockSetId ,readIndexMemorySegment->readerLockSetId )
					&&
					(! writerIndexMemorySegment->clock->happensBefore( readIndexMemorySegment->clock))

			)
			{
				++isRaceCount;
				RaceInstance *r = new  RaceInstance("", 0,
						 currentMemory->writerSegmentSet.segments.at(writerIndex),
						 currentMemory->readerSegmentSet.segments.at(readerIndex),
						currentMemory->address,
						insPtr,
						addressSize
				);
				currentMemory->alreadyRaced = true;
				races.addToRaces(r, currentMemory->mallocAddress);
				return r;
			}
		}

	}

	return NULL;*/

	vector<Segment*>::iterator writer_iterator =  currentMemory->writerSegmentSet.segments.begin();
	vector<Segment*>::iterator reader_iterator =  currentMemory->readerSegmentSet.segments.begin();

	vector<Segment*>::iterator writer_iterator_end =  currentMemory->writerSegmentSet.segments.end();
	vector<Segment*>::iterator reader_iterator_end =  currentMemory->readerSegmentSet.segments.end();


	UINT32 writerIndexThreadId, writerIndexWriterLocksetId;

    for (;writer_iterator != writer_iterator_end; ++writer_iterator)
	{
		writerIndexThreadId = (*writer_iterator)->ownerThreadId;
		writerIndexWriterLocksetId = (*writer_iterator)->writerLockSetId;

		vector<Segment*>::iterator inner_writer_iterator = writer_iterator;
		++inner_writer_iterator;


		for (;inner_writer_iterator != writer_iterator_end; ++inner_writer_iterator)
		{


			if (writerIndexThreadId != (*inner_writer_iterator)->ownerThreadId
					&&

					allLockSets.isIntersectionEmpty(writerIndexWriterLocksetId ,(*inner_writer_iterator)->writerLockSetId )

			)

			{

				++isRaceCount;
				//cout << "Race happend:"<< currentMemory->address << "  inst:" << insPtr << endl;
				RaceInstance *r = new RaceInstance("", 0,
						(*inner_writer_iterator),
						(*writer_iterator),
						currentMemory->address,
						insPtr,
						addressSize
				);
				currentMemory->alreadyRaced = true;
				races.addToRaces(r, currentMemory->mallocAddress);
				return r;

			}

		}



		for (;reader_iterator != reader_iterator_end; ++reader_iterator)
		{
			//readIndexMemorySegment = currentMemory->readerSegmentSet.segments.at(readerIndex);
			if (    writerIndexThreadId != (*reader_iterator)->ownerThreadId
					&&
					(!(*writer_iterator)->clock->happensBefore( (*reader_iterator)->clock)
					&&
					allLockSets.isIntersectionEmpty( writerIndexWriterLocksetId ,/* currentMemory->readerSegmentSet.segments.at(readerIndex)*/(*reader_iterator)->readerLockSetId )
					)


			)
			{
				++isRaceCount;
				RaceInstance *r = new  RaceInstance("", 0,
						(*writer_iterator),
						 (*reader_iterator),
						currentMemory->address,
						insPtr,
						addressSize
				);
				currentMemory->alreadyRaced = true;
				races.addToRaces(r, currentMemory->mallocAddress);
				return r;
			}
		}



	}




	return NULL;
}

int deletedSegmentCount = 0;

/*
 * This function is intended
 * to delete the segments that are suitable to be deleted
 *
 */
VOID checkSegmentCount(THREADID tid)
{
	GetLock(&segmentDeleteLock, tid);
	int size = segmentsToBeDeleted.size();
	if (size > 2 * MAX_SEGMENTS_SIZE )
	{
		deletedSegmentCount = deletedSegmentCount + MAX_SEGMENTS_SIZE ;
		for (int i=0; i < MAX_SEGMENTS_SIZE; ++i)
		{
			if (segmentsToBeDeleted.at(i) != NULL && segmentsToBeDeleted.at(i)->inAvaliableQueue == true)
				continue;
			delete segmentsToBeDeleted.at(i);
			segmentsToBeDeleted.at(i) = NULL;
		}
		segmentsToBeDeleted.erase(segmentsToBeDeleted.begin(), segmentsToBeDeleted.begin() + MAX_SEGMENTS_SIZE);
	}
    ReleaseLock(&segmentDeleteLock);
}

void synchronizeTLS(ThreadLocalData* tls, THREADID threadid, ADDRINT insPtr)
{
	if ( tls->createNewSegment || tls->currentSegment == NULL )// || tls->currentReaderLocksetId != tls->currentSegment->readerLockSetId || tls->currentWriterLocksetId != tls->currentSegment->writerLockSetId  /*||  !(*(tls->currentSegment->clock)  == *(tls->currentVectorClock))*/)
	{
		if (tls->currentSegment != NULL )
			tls->currentSegment->deactivateSegment();
		//cout << "-----------------sync tls--------------" << endl;
		tls->currentSegment = new Segment(tls->currentWriterLocksetId, tls->currentReaderLocksetId, tls->currentVectorClock, threadid, insPtr);
		tls->totalSegmentCount++;
		tls->createNewSegment = false;
		//cout << "Segment Created:" << threadid << "  segment:"  << tls->currentSegment << endl;
		syncQueue->push(tls->currentSegment);
	}
	if (segmentsToBeDeleted.size() > 2 * MAX_SEGMENTS_SIZE )
		     checkSegmentCount(threadid);

}

/**
 * This function gets actual line information of
 * the race.
 */
VOID fillRaceExactLineInfo(RaceInstance* r, ADDRINT insPtr)
{
	string fileName;
	int lineNumber;
    PIN_LockClient();
    PIN_GetSourceLocation(insPtr, &col, &lineNumber, &fileName);
    PIN_UnlockClient();


    r->exactFileName = fileName;
    r->exactLineNumber = lineNumber;


    if (r->segmentOneFileName == "")
    {
        PIN_LockClient();
        PIN_GetSourceLocation(r->segmentOneInstPtr, &col, &lineNumber, &fileName);
        PIN_UnlockClient();
        r->segmentOneFileName = fileName;
        r->segmentOneLine = lineNumber;
    }

    if (r->segmentTwoFileName == "")
    {
        PIN_LockClient();
        PIN_GetSourceLocation(r->segmentOneInstPtr, &col, &lineNumber, &fileName);
        PIN_UnlockClient();
        r->segmentTwoFileName = fileName;
        r->segmentTwoLine = lineNumber;
    }

}

/*
 * If the memory address is not in shared memory(not in the heap) or this memory is already raced
 * do not instrument accesses related to the effectiveAddr
 * */
BOOL shouldMemoryBeConsidered(ADDRINT effectiveAddr, ADDRINT stackPtr, ADDRINT insPtr)
{

	if (isMemoryGlobal(effectiveAddr, stackPtr)  && !races.isInstructionHasRace(insPtr))//  && !races.isMemoryAlreadyHasRace(effectiveAddr) /* && !races.isInstructionHasRace(insPtr)*/)
		return true;
	return false;
}

extern double_t sampleRate;
UINT64 firstCount = 0;
UINT64 secondCount = 0;


/*
 * Read accesses are handled in this function
 * Improvements(optimizations) are added in this code
 * See Read Memory Access Algorithm in thesis
 * */
inline VOID MemoryReadInstrumentation(THREADID threadid,
		ADDRINT effectiveAddr,//efecctive addr = memory addr
		ADDRINT stackPtr,	//stackPtr is required for determining whether the addrees is global or not (ie in heap or bss or initiliazed vars)
		//const char* imageName,
		ADDRINT insPtr,
		UINT32 readSize,
		BOOL sampledFlag
)
{

	if (!sampledFlag || !shouldMemoryBeConsidered( effectiveAddr,  stackPtr, insPtr))
	{
		return;
	}

	ThreadLocalData* tls = getTLS(threadid);
	GetLock(&tls->threadLock, threadid);
	synchronizeTLS(tls, threadid, insPtr);
	Segment* tlsCurrentSegment = tls->currentSegment;
	ReleaseLock(&tls->threadLock);


	PIN_LOCK* currentLock = &variablePinLocks[(effectiveAddr >> 3) & allMemoryLocations.bitMask];
	variablesHashMap* tmp = allMemoryLocations.getVariableHashMap(effectiveAddr);

	if (tmp == NULL)
		return;
	GetLock(currentLock, threadid);
    if (tmp->count(effectiveAddr) == 0)
	{
		if (!sampledFlag) //sampled degilse follow etme
		{
			//++firstCount;
			ReleaseLock(currentLock);
			return;
		}
		else
			(*tmp)[effectiveAddr] = new MemoryAddr(effectiveAddr);
	}

    MemoryAddr* currentMemoryObject = (*tmp)[effectiveAddr];
	if ( currentMemoryObject->alreadyRaced ||
			(enableSingleAccessCheck && currentMemoryObject->isSegmentInReaderSegmentSet(tlsCurrentSegment)))
	{
		ReleaseLock(currentLock);
		return;
	}

	currentMemoryObject->deleteSegmentsNotInAvaliableQueue();
	currentMemoryObject->readerSegmentSet.removeHappenedBeforeSegments(tlsCurrentSegment);
	currentMemoryObject->readerSegmentSet.addSegment(tlsCurrentSegment);
	RaceInstance* r = isRace(currentMemoryObject, insPtr, readSize);
	ReleaseLock(currentLock);

	if (r != NULL)
		fillRaceExactLineInfo(r, insPtr);

}




/*
 * Write accesses are handled in this function
 * Improvements(optimizations) are added in this code
 * See Write Memory Access Algorithm in thesis
 * */

inline VOID  MemoryWriteInstrumentation(THREADID threadid,
		ADDRINT effectiveAddr,//efecctive addr = memory addr
		ADDRINT stackPtr,	//stackPtr is required for determining whether the addrees is global or not (ie in heap or bss or initiliazed vars)
		const char* imageName,
		ADDRINT insPtr,
		UINT32 writeSize,
		BOOL sampledFlag
		)
{

	if (!sampledFlag || !shouldMemoryBeConsidered( effectiveAddr,  stackPtr, insPtr) )
			return;

	ThreadLocalData* tls = getTLS(threadid);
	GetLock(&tls->threadLock, threadid);
	synchronizeTLS(tls, threadid, insPtr);
	Segment* tlsCurrentSegment = tls->currentSegment;
	ReleaseLock(&tls->threadLock);

	PIN_LOCK* currentLock = &variablePinLocks[(effectiveAddr >> 3) & allMemoryLocations.bitMask];
	variablesHashMap* tmp = allMemoryLocations.getVariableHashMap(effectiveAddr);
	if (tmp == NULL)
		return;

	GetLock(currentLock, threadid);
	if (tmp->count(effectiveAddr) == 0)
	{
		if (!sampledFlag) //sampled degilse follow etme
		{
			firstCount++;
			ReleaseLock(currentLock);
			return;
		}
		else
			(*tmp)[effectiveAddr] = new MemoryAddr(effectiveAddr);
	}
	MemoryAddr* currentMemoryObject = (*tmp)[effectiveAddr];

	if (
			 currentMemoryObject->alreadyRaced ||(
			enableSingleAccessCheck &&  currentMemoryObject->isSegmentInWriterSegmentSet(tlsCurrentSegment)))
	{
		ReleaseLock(currentLock);
		return;
	}

    currentMemoryObject->deleteSegmentsNotInAvaliableQueue();
	currentMemoryObject->readerSegmentSet.removeHappenedBeforeSegments(tlsCurrentSegment);
	currentMemoryObject->writerSegmentSet.removeHappenedBeforeSegments(tlsCurrentSegment);
	currentMemoryObject->writerSegmentSet.addSegment(tlsCurrentSegment);

	RaceInstance* r = isRace(currentMemoryObject, insPtr, writeSize);
	ReleaseLock(currentLock);

	if (r != NULL)
		fillRaceExactLineInfo(r, insPtr);
}

/*
 * If an instruction is a write instruction, then process write memory operands
 *
 * biz instruction'u da instrument etmeyecegiz, instruction da bize memory adreslere ulasmamizi saglayacak
 *
 * Process all memory adresses that is written for this particular instruction
 * */
void processMemoryWriteInstruction(INS ins, const char* imageName, BOOL sampledFlag)
{
	UINT32 memoryOperandCount = INS_MemoryOperandCount(ins);
	for(UINT32 i = 0; i < memoryOperandCount; ++i)
	{
		if (INS_MemoryOperandIsWritten(ins, i) /* && INS_OperandIsMemory(ins,i)*/)
		{

			INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE, (AFUNPTR)MemoryWriteInstrumentation,
					IARG_THREAD_ID,
					IARG_MEMORYOP_EA, i,
					IARG_REG_VALUE, REG_STACK_PTR, //pass current stack ptr
					IARG_PTR, imageName,
					IARG_INST_PTR,
					IARG_MEMORYWRITE_SIZE,
					IARG_BOOL,
					sampledFlag,
					IARG_CALL_ORDER, CALL_ORDER_FIRST + 30,
					IARG_END);
			}
	}
}

/*
 * Process all memory adresses that is read for this particular instruction
 * */
void processMemoryReadInstruction(INS ins, const char* imageName, BOOL sampledFlag)
{
	UINT32 memoryOperandCount = INS_MemoryOperandCount(ins);
	for(UINT32 i = 0; i < memoryOperandCount; ++i)
		if (INS_MemoryOperandIsRead(ins, i) /*&& INS_OperandIsMemory(ins,i)*/)
		{
			//if (PIN_ThreadId() >0 )
			//cout << "ThreadID:" << PIN_ThreadId() << "   read:  '"<< INS_Disassemble(ins) << "'  operand:" << i << endl;
			INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE, (AFUNPTR)MemoryReadInstrumentation,
					IARG_THREAD_ID,
					IARG_MEMORYOP_EA, i,
					IARG_REG_VALUE, REG_STACK_PTR,//pass current stack ptr
					//IARG_PTR, imageName,
					IARG_INST_PTR,
					IARG_MEMORYWRITE_SIZE,
					IARG_BOOL,
					sampledFlag,
					IARG_CALL_ORDER, CALL_ORDER_FIRST + 30,
					IARG_END);
		}
}



UINT64 insCount = 0;
/* PIN's Trace equals to HybridDetectors's Segment, namely, single entrance multiple exists
* Insturemnt Trace,
* Move from Trace to Memory Access
* Trace=>BBL=>Instruction=>Memory Access
*
 */
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


	if (tls->currentSegment != NULL )
		tls->currentSegment->deactivateSegment();

	tls->currentSegment = new Segment(tls->currentWriterLocksetId, tls->currentReaderLocksetId, tls->currentVectorClock, tid, 0);
	tls->totalSegmentCount++;

	ReleaseLock(&tls->threadLock);

	 BOOL sampledFlag = true;
	// Visit every basic block  in the trace
	//BBLs are not crucial for us, they are only important for use to reach instructions
	for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
	{

		//biz BBL'i instrument etmeyecegiz. BBL sadece instruction'a ulasmamizi saglayacak
		for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
		{
			if (( irand.rand_int32() & 1023)  > sampleRate)
			{
				//++nonSampledCount;
				sampledFlag = false;
			}
			else
			{	sampledFlag = true;
			    //++sampledCount;
			}


			processMemoryWriteInstruction(ins, imageName, sampledFlag);
			processMemoryReadInstruction(ins, imageName, sampledFlag);
		}
	}

	syncQueue->push(tls->currentSegment); //update new segment
	checkSegmentCount(tid);

}


