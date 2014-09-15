
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
 * PthreadCallInstrumentation.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */

#include "PthreadCallInstrumentation.h"
#include "ThreadLocalData.h"
#include "AllLockSets.h"
#include "AllVariables.h"
using namespace std;


#define MALLOC "malloc"
#define FREE "free"
#define CALLOC "calloc"
#define REALLOC "realloc"

vector <functionNameInstrumentFunctionName> functionNamesEntry;
vector <functionNameInstrumentFunctionName> functionNamesExit;


extern AllLockSets allLockSets;
int totalLockCount = 0;
PIN_LOCK outputLock;

map<ADDRINT, ADDRINT> mallocCounts; //map will look like map[memoryAddrStart] = ( malloced bytes size)
PIN_LOCK mallocCountsLock;
extern AllMemomryAddresses allMemoryLocations;
extern PIN_LOCK variablePinLocks[4096];

int mallocCountsMaxSize = 0;

//all instrumentation to pthread lock calls are similar, so the way we do instrumentation is the same.
//thus we need some initilization for function_name and instrumentation_function_name mapping. Then, we will use this vector in a single loop
void initFunctionNameInstrumentPthread()
{


	InitLock(&outputLock);
	InitLock(&mallocCountsLock);



	//locks
	functionNameInstrumentFunctionName mutexGetExit;
	mutexGetExit.functionName = "pthread_mutex_lock";
	mutexGetExit.instrumentFunction = (AFUNPTR)GetWriteLockAnalysis;
	mutexGetExit.iPoint = IPOINT_AFTER;
	mutexGetExit.callOrder = 2;
	functionNamesExit.push_back(mutexGetExit);

	//locks
	functionNameInstrumentFunctionName mutexTryLockExit;
	mutexTryLockExit.functionName = "pthread_mutex_trylock";
	mutexTryLockExit.instrumentFunction = (AFUNPTR)mutexTryLockExitAnalysis;
	mutexTryLockExit.iPoint = IPOINT_AFTER;
	mutexTryLockExit.callOrder = 2;
	functionNamesExit.push_back(mutexTryLockExit);

	//locks
	functionNameInstrumentFunctionName mutexTryLockEnter;
	mutexTryLockEnter.functionName = "pthread_mutex_trylock";
	mutexTryLockEnter.instrumentFunction = (AFUNPTR)mutexTryLockEnterAnalysis;
	mutexTryLockEnter.iPoint = IPOINT_BEFORE;
	mutexTryLockEnter.callOrder = 1;
	functionNamesEntry.push_back(mutexTryLockEnter);

	//locks
	functionNameInstrumentFunctionName timedMutexTryLockExit;
	timedMutexTryLockExit.functionName = "pthread_mutex_timedlock";
	timedMutexTryLockExit.instrumentFunction = (AFUNPTR)mutexTryLockExitAnalysis;
	timedMutexTryLockExit.iPoint = IPOINT_AFTER;
	timedMutexTryLockExit.callOrder = 2;
	functionNamesExit.push_back(timedMutexTryLockExit);

	//locks
	functionNameInstrumentFunctionName timedMutexTryLockEnter;
	timedMutexTryLockEnter.functionName = "pthread_mutex_timedlock";
	timedMutexTryLockEnter.instrumentFunction = (AFUNPTR)mutexTryLockEnterAnalysis;
	timedMutexTryLockEnter.iPoint = IPOINT_BEFORE;
	timedMutexTryLockEnter.callOrder = 1;
	functionNamesEntry.push_back(timedMutexTryLockEnter);


	//locks
	functionNameInstrumentFunctionName mutexGet;
	mutexGet.functionName = "pthread_mutex_lock";
	mutexGet.instrumentFunction = (AFUNPTR)GetWriteLockAnalysisEnter;
	mutexGet.iPoint = IPOINT_BEFORE;
	mutexGet.callOrder = 1;
	functionNamesEntry.push_back(mutexGet);





	//locks
	functionNameInstrumentFunctionName rwWriteGet;
	rwWriteGet.functionName = "pthread_rwlock_wrlock";
	rwWriteGet.instrumentFunction = (AFUNPTR)GetWriteLockAnalysisEnter;
	rwWriteGet.iPoint = IPOINT_BEFORE;
	rwWriteGet.callOrder = 1;
	functionNamesEntry.push_back(rwWriteGet);


	functionNameInstrumentFunctionName mutexRelease;
	mutexRelease.functionName = "pthread_mutex_unlock";
	mutexRelease.instrumentFunction = (AFUNPTR)ReleaseLockAnalyis;
	mutexRelease.iPoint = IPOINT_BEFORE;
	mutexRelease.callOrder = 5;
	functionNamesEntry.push_back(mutexRelease);

	functionNameInstrumentFunctionName rwLockWriteLock;
	rwLockWriteLock.functionName = "pthread_rwlock_wrlock";
	rwLockWriteLock.instrumentFunction = (AFUNPTR)GetWriteLockAnalysis;
	rwLockWriteLock.iPoint = IPOINT_AFTER;
	rwLockWriteLock.callOrder =  2;
	functionNamesEntry.push_back(rwLockWriteLock);






	functionNameInstrumentFunctionName rwLockUnlock;
	rwLockUnlock.functionName = "pthread_rwlock_unlock";
	rwLockUnlock.instrumentFunction = (AFUNPTR)ReleaseLockAnalyis;
	rwLockUnlock.iPoint = IPOINT_BEFORE;
	rwLockUnlock.callOrder =  5;
	functionNamesEntry.push_back(rwLockUnlock);

	functionNameInstrumentFunctionName rwLockReadLock;
	rwLockReadLock.functionName = "pthread_rwlock_rdlock";
	rwLockReadLock.instrumentFunction = (AFUNPTR)GetReadLockAnalysis;
	rwLockReadLock.iPoint = IPOINT_AFTER;
	rwLockReadLock.callOrder =  4;
	functionNamesEntry.push_back(rwLockReadLock);

	functionNameInstrumentFunctionName rwLockReadLockEnter;
	rwLockReadLockEnter.functionName = "pthread_rwlock_rdlock";
	rwLockReadLockEnter.instrumentFunction = (AFUNPTR)GetReadLockAnalysisEnter;
	rwLockReadLockEnter.iPoint = IPOINT_BEFORE;
	rwLockReadLockEnter.callOrder =  3;
	functionNamesEntry.push_back(rwLockReadLockEnter);



}

VOID ReallocEnter(CHAR * name, ADDRINT previousAddress, ADDRINT newSize, THREADID tid)
{
	ThreadLocalData* tls = getTLS(tid);
	GetLock(&tls->threadLock, tid);
	//cout << "Realloc Enter:" << previousAddress << "   " << newSize << endl;
	tls->nextReallocAddr = previousAddress ;
	tls->nextReallocSize = newSize;
	ReleaseLock(&tls->threadLock);
}


VOID CallocEnter(CHAR * name, ADDRINT nElements, ADDRINT sizeOfEachElement, THREADID tid)
{
	ThreadLocalData* tls = getTLS(tid);
	GetLock(&tls->threadLock, tid);
	tls->nextMallocSize = nElements * sizeOfEachElement;
	//cout << "Calloc size:" << nElements * sizeOfEachElement << endl;
	ReleaseLock(&tls->threadLock);

}

VOID MallocEnter(CHAR * name, ADDRINT size, THREADID tid)
{
	ThreadLocalData* tls = getTLS(tid);
	GetLock(&tls->threadLock, tid);
	tls->nextMallocSize = size;
	//cout << "Malloc size:" << size << endl;
	ReleaseLock(&tls->threadLock);
}
VOID MallocAfter(ADDRINT memoryAddrMallocStarts, THREADID tid)
{
	//cout << "MallocAfter: Thread id:" << tid << endl;
	ThreadLocalData* tls = getTLS(tid);	//first get the next malloc size
	GetLock(&tls->threadLock, tid);
	ADDRINT mallocSize = tls->nextMallocSize;
	tls->nextMallocSize = 0;
	ReleaseLock(&tls->threadLock);


	GetLock(&mallocCountsLock, tid);
	mallocCounts[memoryAddrMallocStarts] = mallocSize;
	//cout << "memoryAddrMallocStarts:"  << hex << memoryAddrMallocStarts << dec << "  mallocSize:" << mallocSize << endl;
	int tmpSize = mallocCounts.size();
	if ( tmpSize > mallocCountsMaxSize)
		mallocCountsMaxSize = tmpSize;

    ReleaseLock(&mallocCountsLock);

}

VOID FreeEnter(CHAR * name, ADDRINT memoryAddrFreeStarts, THREADID threadid)
{
	GetLock(&mallocCountsLock, threadid);
	//cout << "Free Called:" << memoryAddrFreeStarts << "  tid:" << threadid << endl;
	ADDRINT freeSize = mallocCounts[memoryAddrFreeStarts];
	mallocCounts[memoryAddrFreeStarts] = 0;
	mallocCounts.erase(memoryAddrFreeStarts);
	ReleaseLock(&mallocCountsLock);

	if (freeSize == 0)
		return;

	ADDRINT maxMemoryAddrToBeFreed = memoryAddrFreeStarts + freeSize;
	//cout << "Free address Start:" << hex << memoryAddrFreeStarts << "  size :" << dec << freeSize << endl;

	freeMemoryAddress(memoryAddrFreeStarts, maxMemoryAddrToBeFreed, threadid);
}

VOID freeMemoryAddress(ADDRINT memoryAddrFreeStarts, ADDRINT maxMemoryAddrToBeFreed, THREADID threadid)
{
	variablesHashMap* tmpHashMap;
	for (ADDRINT currentAddress = memoryAddrFreeStarts; currentAddress <= maxMemoryAddrToBeFreed; ++currentAddress)
	{
		GetLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask], threadid);
		tmpHashMap = allMemoryLocations.getVariableHashMap(currentAddress);
		if ( tmpHashMap == NULL || (*tmpHashMap).count(currentAddress)  == 0)
		{
			ReleaseLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask]);
			continue;
		}
		MemoryAddr* memAddr = (*tmpHashMap)[currentAddress];
		//cout << "countFreedMem:" << countFreedMem << endl;
		memAddr->currentState = MEM_STATE_VIRGIN;
		memAddr->ownerThreadId = 0;
		memAddr->readerLocksetId = 0;
		memAddr->writerLocksetId = 0;
		memAddr->mallocAddress = currentAddress;
		memAddr->isMallocedTested = false;

		ReleaseLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask]);
	}

}

VOID ReallocAfter(ADDRINT memoryAddrMallocStarts, THREADID tid)
{

	ThreadLocalData* tls = getTLS(tid);
	GetLock(&tls->threadLock, tid);
	ADDRINT  previousAddress = tls->nextReallocAddr;
	ADDRINT newSize = tls->nextReallocSize;
	//cout << "Realloc After:" << memoryAddrMallocStarts << endl;
	ReleaseLock(&tls->threadLock);


	if (previousAddress == 0) //if previous address is NULL, ir use is as malloc
	{
		GetLock(&tls->threadLock, tid); //clear everyting and call malloc rouitune
		tls->nextMallocSize = tls->nextReallocSize;
		tls->nextReallocAddr = 0;
		tls->nextReallocSize = 0;
		//cout << "NULL realloc is called" << endl;
		ReleaseLock(&tls->threadLock);
		MallocAfter(memoryAddrMallocStarts, tid);
		return;
	}

	GetLock(&mallocCountsLock, tid);
	ADDRINT mallocPreviousSize = mallocCounts[previousAddress];
	ReleaseLock(&mallocCountsLock);

	if (previousAddress == memoryAddrMallocStarts) //realloc gives the same address as previos realloc(or malloc,calloc)
	{

		GetLock(&mallocCountsLock, tid); //first update new malloc size
		mallocCounts[memoryAddrMallocStarts] = newSize;
		//cout << "Old Address realloc is called" << endl;
		ReleaseLock(&mallocCountsLock);

		if (newSize < mallocPreviousSize) //eger yeni yer daha kucuk ise, eskiden bizim olan aradaki yerleri free edelim
			freeMemoryAddress(memoryAddrMallocStarts + newSize , memoryAddrMallocStarts + mallocPreviousSize, tid);
	}
	else
	{
		//cout << "New Address realloc is called" << endl;

		if (newSize < mallocPreviousSize)
		{

			//cout << "new size is smaller" << endl;
			moveMemoryAddresses(previousAddress, memoryAddrMallocStarts, newSize, tid);
			freeMemoryAddress(previousAddress + newSize , previousAddress + mallocPreviousSize, tid); //eskiden kalan tasiyamadigimiz yerleri sil
		}
		else
		{
			//cout << "new size is grater" << endl;
			moveMemoryAddresses(previousAddress, memoryAddrMallocStarts, mallocPreviousSize, tid); //butun memory'i yeni yere tasi
			freeMemoryAddress(previousAddress , previousAddress + mallocPreviousSize, tid); //eski her yeri sil, tasiyamadigimiz yer kalmadi
		}

		GetLock(&mallocCountsLock, tid); //first update new malloc size
		mallocCounts[memoryAddrMallocStarts] = newSize;
		mallocCounts[previousAddress] = 0;
		//cout << "New Address realloc is called" << endl;
		ReleaseLock(&mallocCountsLock);

	}
}


VOID moveMemoryAddresses(ADDRINT startOfOldPlace, ADDRINT startOfNewPlace, UINT32 size, THREADID tid)
{
	ADDRINT maxSize = (startOfOldPlace + size);
		ADDRINT newMemAddrTmp;
		variablesHashMap* tmpHashMap;
		MemoryAddr* tmpMemAddr = NULL;
		MemoryAddr* newTmpMemAddrObj = NULL;

		ADDRINT difference = 0;
		for (ADDRINT currentAddress = startOfOldPlace; currentAddress <= maxSize; ++currentAddress)
		{
			GetLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask], tid);
			tmpHashMap = allMemoryLocations.getVariableHashMap(currentAddress);
			if ( tmpHashMap == NULL || (*tmpHashMap).count(currentAddress)  == 0)
			{
				ReleaseLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask]);
				continue;
			}
			tmpMemAddr =  (*tmpHashMap)[currentAddress];
			difference = currentAddress - startOfOldPlace;
			ReleaseLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask]);

			newMemAddrTmp = startOfNewPlace + difference;

			GetLock(&variablePinLocks[(newMemAddrTmp >> 3) & allMemoryLocations.bitMask], tid);
			tmpHashMap = allMemoryLocations.getVariableHashMap(newMemAddrTmp);
			if ( tmpHashMap == NULL )
			{
				ReleaseLock(&variablePinLocks[(newMemAddrTmp >> 3) & allMemoryLocations.bitMask]);
				continue;
			}
			//cout << "moved:" << currentAddress << "  to:" << newMemAddrTmp << endl;
			newTmpMemAddrObj = new MemoryAddr(newMemAddrTmp, tmpMemAddr-> writerLocksetId, tmpMemAddr-> readerLocksetId, tmpMemAddr-> ownerThreadId, tmpMemAddr-> currentState); //yeni mem adresi olustur
			/*newTmpMemAddrObj->currentState = tmpMemAddr-> currentState;
			newTmpMemAddrObj->writerLocksetId = tmpMemAddr-> writerLocksetId;
			newTmpMemAddrObj->readerLocksetId = tmpMemAddr-> readerLocksetId;
			newTmpMemAddrObj->ownerThreadId = tmpMemAddr-> ownerThreadId;
*/
			(*tmpHashMap)[newMemAddrTmp] = newTmpMemAddrObj;

			ReleaseLock(&variablePinLocks[(newMemAddrTmp >> 3) & allMemoryLocations.bitMask]);

			GetLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask], tid);
			tmpHashMap = allMemoryLocations.getVariableHashMap(currentAddress);
			tmpHashMap->erase(currentAddress);
			delete tmpMemAddr;
			ReleaseLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask]);

		}

}

void instrumentMallocFree(IMG img)
{


	RTN reallocRtn = RTN_FindByName(img, REALLOC);
	if (RTN_Valid(reallocRtn))
	{
		RTN_Open(reallocRtn);

		RTN_InsertCall(reallocRtn, IPOINT_BEFORE, (AFUNPTR)ReallocEnter,
				IARG_ADDRINT, REALLOC,
				IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
				IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
				IARG_THREAD_ID,
				IARG_END);


		RTN_InsertCall(reallocRtn, IPOINT_AFTER, (AFUNPTR)ReallocAfter,
				IARG_FUNCRET_EXITPOINT_VALUE,
				IARG_THREAD_ID,
				IARG_END);

		RTN_Close(reallocRtn);
	}


	RTN callocRtn = RTN_FindByName(img, CALLOC);
	if (RTN_Valid(callocRtn))
	{
		RTN_Open(callocRtn);

		// Instrument malloc()
		RTN_InsertCall(callocRtn, IPOINT_BEFORE, (AFUNPTR)CallocEnter,
				IARG_ADDRINT, MALLOC,
				IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
				IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
				IARG_THREAD_ID,
				IARG_END);


		RTN_InsertCall(callocRtn, IPOINT_AFTER, (AFUNPTR)MallocAfter,
				IARG_FUNCRET_EXITPOINT_VALUE,
				IARG_THREAD_ID,
				IARG_END);

		RTN_Close(callocRtn);
	}


	//return ;
	RTN mallocRtn = RTN_FindByName(img, MALLOC);
	if (RTN_Valid(mallocRtn))
	{
		RTN_Open(mallocRtn);

		// Instrument malloc()
		RTN_InsertCall(mallocRtn, IPOINT_BEFORE, (AFUNPTR)MallocEnter,
				IARG_ADDRINT, MALLOC,
				IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
				IARG_THREAD_ID,
				IARG_END);


		RTN_InsertCall(mallocRtn, IPOINT_AFTER, (AFUNPTR)MallocAfter,
				IARG_FUNCRET_EXITPOINT_VALUE,
				IARG_THREAD_ID,
				IARG_END);

		RTN_Close(mallocRtn);
	}

	// Find the free() function.
	RTN freeRtn = RTN_FindByName(img, FREE);
	if (RTN_Valid(freeRtn))
	{
		RTN_Open(freeRtn);
		// Instrument free() to print the input argument value.
		RTN_InsertCall(freeRtn, IPOINT_BEFORE, (AFUNPTR)FreeEnter,
				IARG_ADDRINT, FREE,
				IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
				IARG_THREAD_ID,
				IARG_END);
		RTN_Close(freeRtn);
	}
}

/*
 * Pthread_function_calls will be instrumented here
 * */
VOID InstrumentImage(IMG img, VOID *v)
{


	instrumentMallocFree(img);

	char* imageName = (char*)IMG_Name(img).c_str();

	if (string(imageName).find(libpthread) == string::npos)
		return; //here we only instrument pthread function calls, other images should not be instrumented


	//butun function name, function instrumentation name'ler uzerinde yaklasik ayni islemleri yaptigimiz icin
	for (UINT32 i = 0; i < functionNamesEntry.size(); ++i )
	{
		//cout << "INSTRUMENTED FUNCTION:::" << functionNamesEntry.at(i).functionName << "  i:" << i << endl;

		//output_stream << "INSTRUMENTED FUNCTION:::" << functionNamesEntry.at(i).functionName << "  i:" << i << endl;
		functionNameInstrumentFunctionName currentType = functionNamesEntry.at(i);
		RTN pthreadMutexRTN = RTN_FindByName(img, currentType.functionName.c_str());
		if (RTN_Valid(pthreadMutexRTN))
		{

			RTN_Open(pthreadMutexRTN);
			// Instrument pthread_mutex_lock() to print the input argument value and the return value.
			RTN_InsertCall(pthreadMutexRTN, currentType.iPoint,
					(AFUNPTR)currentType.instrumentFunction,
					IARG_FUNCARG_ENTRYPOINT_VALUE,
					0,
					IARG_THREAD_ID,
					IARG_PTR, imageName,
					IARG_REG_VALUE, REG_STACK_PTR,
					IARG_CALL_ORDER,
					CALL_ORDER_FIRST + currentType.callOrder ,
					IARG_END);


			RTN_Close(pthreadMutexRTN);
		}
	}



	//butun function name, function instrumentation name'ler uzerinde yaklasik ayni islemleri yaptigimiz icin
	for (UINT32 i = 0; i < functionNamesExit.size(); ++i )
	{
		//output_stream << "INSTRUMENTED FUNCTION:::" << functionNamesEntry.at(i).functionName << "  i:" << i << endl;
		functionNameInstrumentFunctionName currentType = functionNamesExit.at(i);
		RTN pthreadMutexRTN = RTN_FindByName(img, currentType.functionName.c_str());
		if (RTN_Valid(pthreadMutexRTN))
		{

			RTN_Open(pthreadMutexRTN);
			// Instrument pthread_mutex_lock() to print the input argument value and the return value.
			RTN_InsertCall(pthreadMutexRTN, currentType.iPoint, (AFUNPTR)currentType.instrumentFunction,
					IARG_FUNCRET_EXITPOINT_VALUE,
					IARG_THREAD_ID,
					IARG_PTR, imageName,
					IARG_REG_VALUE, REG_STACK_PTR,
					IARG_CALL_ORDER,
					CALL_ORDER_FIRST + currentType.callOrder ,
					IARG_END);


			RTN_Close(pthreadMutexRTN);
		}
	}

}

int totalWriteLockEnter = 0;
int totalReadLockEnter = 0;
int totalHappensBefore = 0;

VOID PthreadCreateAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{

	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	cout << "****************  THREAD CREATED  ***************  " << id << "   pin id:"  << PIN_ThreadId()  << "  pin id2:" << PIN_GetTid()<< endl;
	ReleaseLock(&tls->threadLock);

}

VOID PthreadJoinAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
//	tls->isAlive = false;
	cout << "****************  THREAD JOINDE  ***************  " << id << "   pin id:"  << PIN_ThreadId()  << "  pin id2:" << PIN_GetTid()<< endl;


	ReleaseLock(&tls->threadLock);
}


//burada thread'in hangi lock'u aldigini tutacagiz, lock address' function entry'de yapmamiz gerekir
//ancak esas yapmamiz gereken instrumentation, exit'te yapilmali, o yuzden boyle bir yola girdim
VOID GetWriteLockAnalysisEnter(ADDRINT lockAddr, THREADID id ,char* imageName , ADDRINT stackPtr)
{

	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->nextWriteLockAddr = lockAddr;
	ReleaseLock(&tls->threadLock);

}





VOID mutexTryLockEnterAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName , ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->nextWriteLockAddr = lockAddr;
	ReleaseLock(&tls->threadLock);
}




//burada thread'in hangi lock'u aldigini tutacagiz, lock address' function entry'de yapmamiz gerekir
//ancak esas yapmamiz gereken instrumentation, exit'te yapilmali, o yuzden boyle bir yola girdim
VOID GetReadLockAnalysisEnter(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->nextReadLockAddr = lockAddr;
	ReleaseLock(&tls->threadLock);
}



//aslinda burada lock'ta global mi diye bakabiliriz, ama cok sacma olur local bir lock kullandildiysa!!!!!
VOID GetWriteLockAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{


	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->currentWriterLocksetId = allLockSets.getLockSet(tls->currentWriterLocksetId, tls->nextWriteLockAddr, true);
	tls->currentReaderLocksetId = allLockSets.getLockSet(tls->currentReaderLocksetId, tls->nextWriteLockAddr, true);//write lock otomatik read lock
	ReleaseLock(&tls->threadLock);
}


//aslinda burada lock'ta global mi diye bakabiliriz, ama cok sacma olur local bir lock kullandildiysa!!!!!
VOID mutexTryLockExitAnalysis(ADDRINT exitVal, THREADID id ,char* imageName, ADDRINT stackPtr)
{

	if (exitVal == 0) //yani lock'u alabildiysek
	{
		++totalLockCount;
		ThreadLocalData* tls = getTLS(id);
		GetLock(&tls->threadLock, id);
		//cout << "Before:GetWriteLockAnalysis  loclksetid:" << tls->currentWriterLocksetId << endl;
		tls->currentWriterLocksetId = allLockSets.getLockSet(tls->currentWriterLocksetId, tls->nextWriteLockAddr, true);
		tls->currentReaderLocksetId = allLockSets.getLockSet(tls->currentReaderLocksetId, tls->nextWriteLockAddr, true);
		ReleaseLock(&tls->threadLock);
	}
}


//aslinda burada lock'ta global mi diye bakabiliriz, ama cok sacma olur local bir lock kullandildiysa!!!!!
VOID ReleaseLockAnalyis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->currentWriterLocksetId = allLockSets.getLockSet(tls->currentWriterLocksetId, lockAddr, false);
	tls->currentReaderLocksetId = allLockSets.getLockSet(tls->currentReaderLocksetId, lockAddr, false);
	//cout <<"tid:" << id << "  Release Lock:" << tls->currentWriterLocksetId  << "  addr:" << lockAddr  << endl;
	ReleaseLock(&tls->threadLock);

}



//aslinda burada lock'ta global mi diye bakabiliriz, ama cok sacma olur local bir lock kullandildiysa!!!!!
VOID GetReadLockAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->currentReaderLocksetId = allLockSets.getLockSet(tls->currentWriterLocksetId, tls->nextReadLockAddr, true);
	//cout << "Thread " << id << "  get lock:" << lockAddr << endl;
	ReleaseLock(&tls->threadLock);


}


