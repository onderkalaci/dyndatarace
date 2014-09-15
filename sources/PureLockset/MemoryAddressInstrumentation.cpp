
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
 */


//#define LOG4CPLUSENABLED
#include "MemoryAddressInstrumentation.h"
#include "MemoryAddr.h"
#include "AllVariables.h"
#include "ThreadLocalData.h"
#include "AllLockSets.h"
#include "AllRaceInstances.h"
#define MAX_SEGMENTS_SIZE 1000

ADDRINT STACK_PTR_ERROR = 1000000;
AllRaceInstances races;

extern PIN_LOCK testLock;

extern AllMemomryAddresses allMemoryLocations;
extern PIN_LOCK variablePinLocks[4096];
INT32 col;
extern AllLockSets allLockSets;
extern map<ADDRINT, ADDRINT> mallocCounts; //map will look like map[memoryAddrStart] = ( malloced bytes size)





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

	if ( static_cast<UINT64> (abs(stackPtr - effectiveAddr)) > STACK_PTR_ERROR	) //if stack pointer is greater, it is global or in heap. ie shared
		return true;

	return false;
}

int raceCounter = 0;
RaceInstance* createRaceInstance(MemoryAddr* currentMemory, ADDRINT insPtr, UINT32 addressSize, ADDRINT stackPtr)
{
	++raceCounter;
	return new RaceInstance("file", 0, currentMemory->address, insPtr, addressSize, stackPtr);
}



BOOL shouldMemoryBeConsidered(ADDRINT effectiveAddr, ADDRINT stackPtr, ADDRINT insPtr)
{

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


	if (!shouldMemoryBeConsidered( effectiveAddr,  stackPtr, insPtr))
		return;


	ThreadLocalData* tls = getTLS(threadid);
	//GetLock(&tls->threadLock, threadid);
	UINT32 threadWriterLocksetId = tls->currentWriterLocksetId;
	UINT32 threadReaderLocksetId = tls->currentReaderLocksetId;
	//ReleaseLock(&tls->threadLock);

	PIN_LOCK* currentVariableLock = &variablePinLocks[0];

	variablesHashMap* tmp = allMemoryLocations.getVariableHashMap(effectiveAddr);
	//cout << "read enter" << endl;


	if (tmp == NULL)
		return;



	MemoryAddr* memAddr = NULL;
	RaceInstance* r = NULL;
	bool raceHappened = false;

	GetLock(currentVariableLock, threadid);


	if (tmp->count(effectiveAddr) == 0)
	{
		(*tmp)[effectiveAddr] = new MemoryAddr(effectiveAddr,threadWriterLocksetId, threadReaderLocksetId, threadid, MEM_STATE_VIRGIN );
		memAddr = (*tmp)[effectiveAddr];
		/*memAddr->readerLocksetId = threadReaderLocksetId;
		memAddr->writerLocksetId = threadWriterLocksetId;
		memAddr->ownerThreadId = threadid;
		memAddr->currentState = MEM_STATE_VIRGIN;*/
		//cout << "Mem Addr Created (read):" << hex << effectiveAddr << dec << "   by thread:" << threadid << "  write size:" << readSize << endl;


	}
	else if ((*tmp)[effectiveAddr] != NULL)
	{

		memAddr = (*tmp)[effectiveAddr];
     	memAddr->updateState(threadid, false, threadWriterLocksetId, threadReaderLocksetId);

		//cout << "state read:" <<  (*tmp)[effectiveAddr]->currentState << hex << "   addr:" << effectiveAddr << dec  << endl;
		if (memAddr->currentState == MEM_STATE_READ_SHARED || memAddr->currentState == MEM_STATE_SHARED_MODIFIED)
		{	//readshared ya da shared modified ise, lockset'leri update et

			memAddr->readerLocksetId =  allLockSets.findIntersection(memAddr->readerLocksetId, threadReaderLocksetId);
			memAddr->writerLocksetId =  allLockSets.findIntersection(memAddr->writerLocksetId, threadWriterLocksetId);
			/*bool intersectWithReaderLocksets = allLockSets.isIntersectionEmpty(memAddr->readerLocksetId, threadReaderLocksetId);
			bool intersectWithReaderWriterLocksets = allLockSets.isIntersectionEmpty(memAddr->readerLocksetId, threadWriterLocksetId);
			bool intersectWithWriterLocksets = allLockSets.isIntersectionEmpty(memAddr->writerLocksetId, threadWriterLocksetId);
			bool intersectWithWriterReaderLocksets = allLockSets.isIntersectionEmpty(memAddr->writerLocksetId, threadReaderLocksetId);

			//cout << "intersectWithReaderLocksets:" << intersectWithReaderLocksets << "  intersectWithReaderWriterLocksets:" <<intersectWithReaderWriterLocksets
				//	<<"   intersectWithWriterLocksets:" << intersectWithWriterLocksets << "  intersectWithWriterReaderLocksets:" << intersectWithWriterReaderLocksets  << endl;



			//shared_modified ise, race olabilir diyelim
			if ( memAddr->currentState == MEM_STATE_SHARED_MODIFIED &&
					intersectWithReaderLocksets == true && intersectWithWriterLocksets == true
						&& intersectWithReaderWriterLocksets == true && intersectWithWriterReaderLocksets == true
					) //yani toptan bos ise
						raceHappened = true;

              */


			if ( memAddr->currentState == MEM_STATE_SHARED_MODIFIED &&
					allLockSets.isIntersectionEmpty(memAddr->readerLocksetId, threadReaderLocksetId)
					&&  allLockSets.isIntersectionEmpty(memAddr->readerLocksetId, threadWriterLocksetId)
				  && allLockSets.isIntersectionEmpty(memAddr->writerLocksetId, threadWriterLocksetId)
				  &&  allLockSets.isIntersectionEmpty(memAddr->writerLocksetId, threadReaderLocksetId)
								) //yani toptan bos ise
									raceHappened = true;
			//raceHappened = false;
		}
	}

	//isMallocAreaStrct normalizedAddrWithRespectToArrays(false, currentMemory->address);
	if (raceHappened && memAddr->isMallocedTested == false) //daha once bakmadi demek
	{
		isMallocAreaStrct normalizedAddrWithRespectToArrays = findMallocStartAreaIfMallocedArea( memAddr->address);
		memAddr->mallocAddress = normalizedAddrWithRespectToArrays.addrToUse;
		memAddr->isMallocedTested = true;
	}


	ReleaseLock(currentVariableLock);
	if (raceHappened )
	{
	    if (races.isRaceOKOutside(effectiveAddr, insPtr, memAddr->mallocAddress))
		{
			r = createRaceInstance((*tmp)[effectiveAddr], insPtr, readSize, stackPtr);
			r->fillLineNumberInfo();
			races.addToRaces(r, memAddr->mallocAddress);
		}
	}
}




inline VOID  MemoryWriteInstrumentation(THREADID threadid,
		ADDRINT effectiveAddr,//efecctive addr = memory addr
		ADDRINT stackPtr,	//stackPtr is required for determining whether the addrees is global or not (ie in heap or bss or initiliazed vars)
		const char* imageName,
		ADDRINT insPtr,
		UINT32 writeSize
		)
{

	if (!shouldMemoryBeConsidered( effectiveAddr,  stackPtr, insPtr))
		return;

	ThreadLocalData* tls = getTLS(threadid);
	//GetLock(&tls->threadLock, threadid);
	UINT32 threadWriterLocksetId = tls->currentWriterLocksetId;
	UINT32 threadReaderLocksetId = tls->currentReaderLocksetId;
	//ReleaseLock(&tls->threadLock);

	PIN_LOCK* currentVariableLock = &variablePinLocks[0];

	variablesHashMap* tmp = allMemoryLocations.getVariableHashMap(effectiveAddr);
	if (tmp == NULL)
		return;


	RaceInstance* r = NULL;
	bool raceHappened = false;
	MemoryAddr* memAddr = NULL;
	GetLock(currentVariableLock, threadid);

	if (tmp->count(effectiveAddr) == 0)
	{
		(*tmp)[effectiveAddr] = new MemoryAddr(effectiveAddr, threadWriterLocksetId, threadReaderLocksetId, threadid, MEM_STATE_EXCLUSIVE);
		memAddr = (*tmp)[effectiveAddr];
		/*memAddr->readerLocksetId = threadReaderLocksetId;
		memAddr->writerLocksetId = threadWriterLocksetId;
		memAddr->ownerThreadId = threadid;
		memAddr->currentState = MEM_STATE_EXCLUSIVE;*/
		//cout << "Mem Addr Created:" << hex << effectiveAddr << dec << "   by thread:" << threadid << "  write size:" << writeSize << endl;
	}
	else if ((*tmp)[effectiveAddr] != NULL)
	{
		memAddr = (*tmp)[effectiveAddr];
		//cout << "state write previous:" <<  (*tmp)[effectiveAddr]->currentState  << "  owner thread:" <<  (*tmp)[effectiveAddr]->ownerThreadId << "   current writerThread:" << threadid << hex << "  effective ad:" << effectiveAddr << dec  << "  write size:" << writeSize << endl;

		memAddr->updateState(threadid, true, threadWriterLocksetId, threadReaderLocksetId);
		//cout << "state write:" <<  (*tmp)[effectiveAddr]->currentState  << "  owner thread:" <<  (*tmp)[effectiveAddr]->ownerThreadId << "   current writerThread:" << threadid << hex << "  effective ad:" << effectiveAddr << dec  << "  write size:" << writeSize << endl;
		if (memAddr->currentState == MEM_STATE_READ_SHARED ||memAddr->currentState == MEM_STATE_SHARED_MODIFIED)
		{
			memAddr->writerLocksetId =  allLockSets.findIntersection(memAddr->writerLocksetId, threadWriterLocksetId);
			bool intersectWithWriterLocksets = allLockSets.isIntersectionEmpty(memAddr->writerLocksetId, threadWriterLocksetId);
			bool intersectWithWriterReaderLocksets = allLockSets.isIntersectionEmpty(memAddr->writerLocksetId, threadReaderLocksetId);
			//intersectWithWriterReaderLocksets = false;
			if ( memAddr->currentState == MEM_STATE_SHARED_MODIFIED &&
					intersectWithWriterLocksets == true && intersectWithWriterReaderLocksets == true ) //yani toptan bos ise
				raceHappened = true;
		}
	}


	ReleaseLock(currentVariableLock);
	if (memAddr->isMallocedTested == false) //daha once bakmadi demek
	{
		isMallocAreaStrct normalizedAddrWithRespectToArrays = findMallocStartAreaIfMallocedArea( memAddr->address);
		memAddr->mallocAddress = normalizedAddrWithRespectToArrays.addrToUse;
		memAddr->isMallocedTested = true;
	}
	if (raceHappened && races.isRaceOKOutside(effectiveAddr, insPtr, memAddr->mallocAddress))
	{
		r = createRaceInstance((*tmp)[effectiveAddr], insPtr, writeSize, stackPtr);
		r->fillLineNumberInfo();
		races.addToRaces(r, memAddr->mallocAddress);
	}
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
		if (INS_MemoryOperandIsWritten(ins, i)/* && INS_OperandIsMemory(ins,i) */)
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
		if (INS_MemoryOperandIsRead(ins, i) /* && INS_OperandIsMemory(ins,i)*/)
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


