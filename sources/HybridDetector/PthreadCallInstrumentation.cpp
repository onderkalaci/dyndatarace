/*

 * PthreadCallInstrumentation.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 *
 *  This file includes pthread and system(malloc,calloc, vs)
 *  function call insturmentation
 */

#include "PthreadCallInstrumentation.h"
#include "ThreadLocalData.h"
#include "AllLockSets.h"
#include "AllVariables.h"
#include "HeapInfo.h"
#include "AllRaceInstances.h"
using namespace std;


#define MALLOC "malloc"
#define FREE "free"
#define CALLOC "calloc"
#define REALLOC "realloc"

/**
 * PIN provides input parameters of a function
 * only at the enterence of the function that is insturmented
 * Therefore, for instance for such a function call: pthread_mutex_lock(&l);
 * we will only be able to get the address of lock "l" on the insturmentation at the enterence of function
 *
 *  Moreover, when the function call is exited (blocking or non-blocking) the required action
 *  is taken in its functionNamesExit counterpart. For instance if a thread call pthread_mutex_lock , its lockset must be
 *  updated when thread exists "pthread_mutex_lock" call. That is where functionNamesExit is used
 */
vector <functionNameInstrumentFunctionName> functionNamesEntry;
vector <functionNameInstrumentFunctionName> functionNamesExit;

//TODO handle lost signals
vector <signalQueueStruct> signalQueue;
vector <semQueueStruct> semQueue;
vector <barrierQueueStruct> barrierQueue;

extern AllLockSets allLockSets;
int totalLockCount = 0;
PIN_LOCK outputLock;
PIN_LOCK signalQueueLock;
PIN_LOCK barrierQueueLock;
PIN_LOCK semQueueLock;


map<ADDRINT, ADDRINT> mallocCounts; //map will look like map[memoryAddrStart] = ( malloced bytes size)
map<ADDRINT, HeapInfo*> heapInfos;  // [addr] => [baseMallocAddr, current heap size ]
PIN_LOCK mallocCountsLock;
extern AllMemomryAddresses allMemoryLocations;
extern PIN_LOCK* variablePinLocks;

int mallocCountsMaxSize = 0;

/* all instrumentation to pthread lock calls are similar, so the way we do instrumentation is the same.
 * thus we need some initilization for function_name and
 * instrumentation_function_name mapping.
 * Then, we will use this vector in a single loop
 *
 * This function intentionally adds each instrumentation manually. The reason is it can easily
 * be examined and changed
 *
 * Lets explain a single example
 *
 *	functionNameInstrumentFunctionName mutexGetExit;    => Not Important Give Whatever Name
	mutexGetExit.functionName = "pthread_mutex_lock";   => pthread_mutex_lock is the function to be instrumented
	mutexGetExit.instrumentFunction = (AFUNPTR)GetWriteLockAnalysis;  => GetWriteLockAnalysis  the analysis function
	mutexGetExit.iPoint = IPOINT_AFTER;  => when to call, after thread exists exectuing the function
	mutexGetExit.callOrder = 2;  => call order is used if pin executes more than one callbacks, this order will be utilzed
	functionNamesExit.push_back(mutexGetExit);  => since IPOINT_AFTER, push to functionNamesExit . if it were  IPOINT_BEFORE push to functionNamesEntry
 */
void initFunctionNameInstrumentPthread()
{

	InitLock(&outputLock);
	InitLock(&signalQueueLock);
	InitLock(&barrierQueueLock);
	InitLock(&mallocCountsLock);
	InitLock(&semQueueLock);



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
	functionNameInstrumentFunctionName pthreadJoin;
	pthreadJoin.functionName = "pthread_join";
	pthreadJoin.instrumentFunction = (AFUNPTR)PthreadJoinAnalysis;
	pthreadJoin.iPoint = IPOINT_BEFORE;
	pthreadJoin.callOrder = 1;
	functionNamesEntry.push_back(pthreadJoin);



	//locks
	functionNameInstrumentFunctionName pthreadCreate;
	pthreadCreate.functionName = "pthread_create";
	pthreadCreate.instrumentFunction = (AFUNPTR)PthreadCreateAnalysis;
	pthreadCreate.iPoint = IPOINT_BEFORE;
	pthreadCreate.callOrder = 1;
	functionNamesEntry.push_back(pthreadCreate);

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

	//signal -  wait
	functionNameInstrumentFunctionName condWait;
	condWait.functionName = "pthread_cond_wait";
	condWait.instrumentFunction = (AFUNPTR)CondWaitAnalysisEnter;
	condWait.iPoint = IPOINT_BEFORE;
	condWait.callOrder =  7;
	functionNamesEntry.push_back(condWait);


	functionNameInstrumentFunctionName condWaitIAfter; //instrument after,  wait diyen birden cok thread varsa, hangisinin kalkacagini bilmek gerekir
	condWaitIAfter.functionName = "pthread_cond_wait";
	condWaitIAfter.instrumentFunction = (AFUNPTR)CondWaitAnalysisExit;
	condWaitIAfter.iPoint = IPOINT_AFTER; //?/?/?/?/?/
	condWaitIAfter.callOrder =  8;
	functionNamesExit.push_back(condWaitIAfter);

	//signal -  wait
	functionNameInstrumentFunctionName barrierWait;
	barrierWait.functionName = "pthread_barrier_wait";
	barrierWait.instrumentFunction = (AFUNPTR)BarrirerWaitAnalysisEnter;
	barrierWait.iPoint = IPOINT_BEFORE;
	barrierWait.callOrder =  7;
	functionNamesEntry.push_back(barrierWait);

	functionNameInstrumentFunctionName barrierWaitIAfter; //instrument after,  wait diyen birden cok thread varsa, hangisinin kalkacagini bilmek gerekir
	barrierWaitIAfter.functionName = "pthread_barrier_wait";
	barrierWaitIAfter.instrumentFunction = (AFUNPTR)BarrirerWaitAnalysisExit;
	barrierWaitIAfter.iPoint = IPOINT_AFTER; //?/?/?/?/?/
	barrierWaitIAfter.callOrder =  8;
	functionNamesExit.push_back(barrierWaitIAfter);


	//signal -  wait
	functionNameInstrumentFunctionName barrierWait_tmp;
	barrierWait_tmp.functionName = "_Z19parsec_barrier_waitP16parsec_barrier_t";
	barrierWait_tmp.instrumentFunction = (AFUNPTR)BarrirerWaitAnalysisEnter;
	barrierWait_tmp.iPoint = IPOINT_BEFORE;
	barrierWait_tmp.callOrder =  7;
	functionNamesEntry.push_back(barrierWait_tmp);

	functionNameInstrumentFunctionName barrierWaitIAfter_tmp; //instrument after,  wait diyen birden cok thread varsa, hangisinin kalkacagini bilmek gerekir
	barrierWaitIAfter_tmp.functionName = "_Z19parsec_barrier_waitP16parsec_barrier_t";
	barrierWaitIAfter_tmp.instrumentFunction = (AFUNPTR)BarrirerWaitAnalysisExit;
	barrierWaitIAfter_tmp.iPoint = IPOINT_AFTER; //?/?/?/?/?/
	barrierWaitIAfter_tmp.callOrder =  8;
	functionNamesExit.push_back(barrierWaitIAfter_tmp);


	functionNameInstrumentFunctionName condSignal;
	condSignal.functionName = "pthread_cond_signal"; //wakes up single thread
	condSignal.instrumentFunction = (AFUNPTR)CondSignalAnalysis;
	condSignal.iPoint = IPOINT_BEFORE;
	condSignal.callOrder =  9;
	functionNamesEntry.push_back(condSignal);




	functionNameInstrumentFunctionName condBCast;
	condBCast.functionName = "pthread_cond_broadcast"; //wakes up all threads
	condBCast.instrumentFunction = (AFUNPTR)CondBCastAnalysis;
	condBCast.iPoint = IPOINT_BEFORE;
	condBCast.callOrder =  10;
	functionNamesEntry.push_back(condBCast);




	functionNameInstrumentFunctionName semWaitEnter;
	semWaitEnter.functionName = "sem_wait"; //wakes up single thread
	semWaitEnter.instrumentFunction = (AFUNPTR)semWaitAnalysisEnter;
	semWaitEnter.iPoint = IPOINT_BEFORE;
	semWaitEnter.callOrder =  9;
	functionNamesEntry.push_back(semWaitEnter);

	functionNameInstrumentFunctionName semWaitExit;
	semWaitExit.functionName = "sem_wait"; //wakes up single thread
	semWaitExit.instrumentFunction = (AFUNPTR)semWaitAnalysisExit;
	semWaitExit.iPoint = IPOINT_AFTER;
	semWaitExit.callOrder =  9;
	functionNamesExit.push_back(semWaitExit);


	functionNameInstrumentFunctionName semPost;
	semPost.functionName = "sem_post"; //wakes up single thread
	semPost.instrumentFunction = (AFUNPTR)semPostAnalyis;
	semPost.iPoint = IPOINT_BEFORE;
	semPost.callOrder =  9;
	functionNamesEntry.push_back(semPost);



}




/*
 *  All the functions that its name includes the word  "Enter
 *   such as: MallocEnter, GetWriteLockAnalysisEnter
 *   is called IPOINT_BEFORE of the function.
 *   The main purpose of these functions are to be able to
 *   save inputs of to the function to thread local storage until
 *   the exit function (IPOINT_AFTER counterpart) is called
 *
 *
 *   Mostly, exit functions(IPOINT_AFTER) are the ones that
 *   does the actual job. For instance, GetWriteLockAnalysis callback function
 *   updates the writer lockset of the thread.
 *   Or MallocAfter function keeps track of malloc functions that the
 *   caller thread executes
 *
 * */


/*
 * Save malloc size to
 * thread local storage
 * until MallocAfter is called
 * */
VOID MallocEnter(CHAR * name, ADDRINT size, THREADID tid)
{
	ThreadLocalData* tls = getTLS(tid);
	GetLock(&tls->threadLock, tid);
	tls->nextMallocSize = size;
	//cout << "Malloc size:" << size << endl;
	ReleaseLock(&tls->threadLock);
}

isMallocAreaStrct isMallocedAreaInAlreadyMallocedHeap(ADDRINT effectiveAddr)
{

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
	return retVal;
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


	ReleaseLock(&mallocCountsLock);

}



/*
 * Save free size to
 * thread local storage
 * until FreeAfter is called
 * */

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
	freeMemoryAddress(memoryAddrFreeStarts, maxMemoryAddrToBeFreed, threadid);

}

UINT64 freeHit = 0;
UINT64 freeNonHit = 0;


/*
 * When any memory address is freed, it must be cleared from
 * shadow memory in order to discard false positives as described
 * in thesis
 * */
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
			//++freeNonHit;
			continue;
		}
        //++freeHit;
		(*tmpHashMap)[currentAddress]->readerSegmentSet.clearAllSegments();
		(*tmpHashMap)[currentAddress]->writerSegmentSet.clearAllSegments();
		(*tmpHashMap)[currentAddress]->mallocAddress = 0;
		ReleaseLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask]);
	}
}


/*
 * Save calloc size to
 * thread local storage
 * until MallocAfter is called
 * */

VOID CallocEnter(CHAR * name, ADDRINT nElements, ADDRINT sizeOfEachElement, THREADID tid)
{
	ThreadLocalData* tls = getTLS(tid);
	GetLock(&tls->threadLock, tid);
	tls->nextMallocSize = nElements * sizeOfEachElement;
	//cout << "Calloc size:" << nElements * sizeOfEachElement << endl;
	ReleaseLock(&tls->threadLock);

}



/*
 * (Firstly read how realloc works!!)
 *
 * After realloc, check if the newly allocated area is bigger or smaller
 * or size equal but addresses changed.
 * Handle each case by moving/freeing the addresses
 */
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


/*
 * if realloc lead memory addresses to be moved,
 * this function moves memory addressed to their new places
 * */
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
		newTmpMemAddrObj = new MemoryAddr(newMemAddrTmp); //yeni mem adresi olustur
		newTmpMemAddrObj->readerSegmentSet.segments = tmpMemAddr->readerSegmentSet.segments;
		newTmpMemAddrObj->writerSegmentSet.segments = tmpMemAddr->writerSegmentSet.segments;
		(*tmpHashMap)[newMemAddrTmp] = newTmpMemAddrObj;

		tmpMemAddr->readerSegmentSet.segments.clear(); //eskinin vectorClocklari null'a
		tmpMemAddr->writerSegmentSet.segments.clear();

		ReleaseLock(&variablePinLocks[(newMemAddrTmp >> 3) & allMemoryLocations.bitMask]);

		GetLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask], tid);
		tmpHashMap = allMemoryLocations.getVariableHashMap(currentAddress);
		tmpHashMap->erase(currentAddress);
		delete tmpMemAddr;
		ReleaseLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask]);
	}

}


/*
 * Save realloc size to
 * thread local storage
 * until MallocAfter is called
 * */
VOID ReallocEnter(CHAR * name, ADDRINT previousAddress, ADDRINT newSize, THREADID tid)
{
	ThreadLocalData* tls = getTLS(tid);
	GetLock(&tls->threadLock, tid);
	//cout << "Realloc Enter:" << previousAddress << "   " << newSize << endl;
	tls->nextReallocAddr = previousAddress ;
	tls->nextReallocSize = newSize;
	ReleaseLock(&tls->threadLock);
}

/**
 * Add callbacks for malloc/free
 */
void instrumentMallocFree(IMG img)
{

	/*
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

	*/
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

/*
 * Instead of insturmenting pthread_create and pthread_join,
 * we use ThreadStart and ThreadFini functions provided by PIN
 * Therefore, below functons are useless currently
 * */
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



/*
 * Save writer lock address  to
 * thread local storage
 * until GetWriteLockAnalysis is called
 * */
VOID GetWriteLockAnalysisEnter(ADDRINT lockAddr, THREADID id ,char* imageName , ADDRINT stackPtr)
{

	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->nextWriteLockAddr = lockAddr;
	ReleaseLock(&tls->threadLock);

}




/*
 * Save writer lock address  to
 * thread local storage
 * until mutexTryLockExitAnalysis is called
 * */
VOID mutexTryLockEnterAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName , ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->nextWriteLockAddr = lockAddr;
	ReleaseLock(&tls->threadLock);
}



/*
 * Save reader lock address  to
 * thread local storage
 * until GetReadLockAnalysis is called
 * */
VOID GetReadLockAnalysisEnter(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->nextReadLockAddr = lockAddr;
	ReleaseLock(&tls->threadLock);
}



/*
 * Update Writer Lockset Of the calling thread upon acuqire
 * */
VOID GetWriteLockAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{


	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->currentVectorClock->event();

	tls->currentWriterLocksetId = allLockSets.getLockSet(tls->currentWriterLocksetId, tls->nextWriteLockAddr, true);
	tls->currentReaderLocksetId = allLockSets.getLockSet(tls->currentReaderLocksetId, tls->nextWriteLockAddr, true);//write lock otomatik read lock

	tls->createNewSegment = true;
	ReleaseLock(&tls->threadLock);
}



/*
 * Update Writer Lockset Of the calling thread iff
 * thread successfully acquires the lock
 * */

VOID mutexTryLockExitAnalysis(ADDRINT exitVal, THREADID id ,char* imageName, ADDRINT stackPtr)
{

	if (exitVal == 0) //successfull acquire
	{
		++totalLockCount;
		ThreadLocalData* tls = getTLS(id);
		GetLock(&tls->threadLock, id);
		tls->currentVectorClock->event();
		tls->currentWriterLocksetId = allLockSets.getLockSet(tls->currentWriterLocksetId, tls->nextWriteLockAddr, true);
		tls->currentReaderLocksetId = allLockSets.getLockSet(tls->currentReaderLocksetId, tls->nextWriteLockAddr, true);
		tls->createNewSegment = true;
		ReleaseLock(&tls->threadLock);
	}
	else
	{
		ThreadLocalData* tls = getTLS(id);
		GetLock(&tls->threadLock, id);
		tls->nextWriteLockAddr = 0;
		ReleaseLock(&tls->threadLock);

	}
}

/*
 * Update Writer Lockset Of the calling thread upon release
 * */
VOID ReleaseLockAnalyis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->currentVectorClock->event();
	tls->currentWriterLocksetId = allLockSets.getLockSet(tls->currentWriterLocksetId, lockAddr, false);
	tls->currentReaderLocksetId = allLockSets.getLockSet(tls->currentReaderLocksetId, lockAddr, false);
	tls->createNewSegment = true;
	//cout <<"tid:" << id << "  Release Lock:" << tls->currentWriterLocksetId  << "  addr:" << lockAddr  << endl;
	ReleaseLock(&tls->threadLock);

}


/*
 * Update Reader Lockset Of the calling thread upon acquire
 * */
VOID GetReadLockAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->currentVectorClock->event();
	tls->currentReaderLocksetId = allLockSets.getLockSet(tls->currentWriterLocksetId, tls->nextReadLockAddr, true);
	tls->createNewSegment = true;
	//cout << "Thread " << id << "  get lock:" << lockAddr << endl;
	ReleaseLock(&tls->threadLock);


}


/*
 * Save condition variable address to
 * thread local storage
 * until CondWaitAnalysisExit is called
 * */
VOID CondWaitAnalysisEnter(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
//	cout << "Thread " << id << " entered to condWait:" << condVarAddr << endl;
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->nextWaitCondVariableAddr = condVarAddr;
	ReleaseLock(&tls->threadLock);
}


/*
 * Update vector clock Of the calling thread upon condition variable wait
 * exit
 *  We simply keep a signallers queue, if the condVarAddr is
 *  already signalled, create the happens before relation
 * */
VOID CondWaitAnalysisExit(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{

	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	condVarAddr = tls->nextWaitCondVariableAddr;

	signalQueueStruct tmpSignal;

	GetLock(&signalQueueLock, id);
	int signalQueueSize = signalQueue.size();
	for (int i=0; i < signalQueueSize; ++i)
	{
		if (signalQueue.at(i).condVarAddr == condVarAddr)
		{
			tmpSignal = signalQueue.at(i);
			break;
		}
	}
	ReleaseLock(&signalQueueLock);

	if (tmpSignal.clock == NULL)
		cout << "OOPS signal Error" << endl;

	tls->currentVectorClock->receiveAction(tmpSignal.clock);

	tls->currentVectorClock->event();
	tls->createNewSegment = true;

	ReleaseLock(&tls->threadLock);


}



/*
 * Save barrier  address to
 * thread local storage
 * until BarrirerWaitAnalysisExit is called
 * */
VOID BarrirerWaitAnalysisEnter(ADDRINT barrierAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	barrierQueueStruct b(barrierAddr, id);
	GetLock(&barrierQueueLock, id);
	barrierQueue.push_back(b);
	ReleaseLock(&barrierQueueLock);
	ThreadLocalData* currentThreadTLS = getTLS(id);
	GetLock(&currentThreadTLS->threadLock, id);
	currentThreadTLS->nextBarrierAddr = barrierAddr;
	ReleaseLock(&currentThreadTLS->threadLock);



}


/*
 * Update vector clock Of the calling thread upon condition variable wait
 * exit
 *  Barriers lead to (n)*(n-1) HB relations. Whenever one of the
 *  barrier waiter thread exitst, create the HB relations using thread ids of waiter threads
 * */

VOID BarrirerWaitAnalysisExit(ADDRINT barrierAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	GetLock(&barrierQueueLock, id);
	ThreadLocalData* currentThreadTLS = getTLS(id);
	barrierAddr = currentThreadTLS->nextBarrierAddr;

	if (barrierAddr == 0)
	{
		ReleaseLock(&barrierQueueLock);
		return;
	}

	vector<THREADID> barrierWaiterThreadIds;
	int samebarrierThreadCount = 0;

	int size =  barrierQueue.size();
	for (int i=0; i < size; ++i)
		if (barrierQueue.at(i).barrierAddr == barrierAddr && barrierQueue.at(i).tid != id)
			barrierWaiterThreadIds.push_back(barrierQueue.at(i).tid );


	samebarrierThreadCount = barrierWaiterThreadIds.size();
	for (int i=0; i < samebarrierThreadCount; ++i)
	{
		ThreadLocalData* tls = getTLS(barrierWaiterThreadIds.at(i));
		currentThreadTLS->currentVectorClock->receiveAction(tls->currentVectorClock);
		tls->nextBarrierAddr = 0;
	}

	for (int i=0; i < samebarrierThreadCount; ++i)
	{
		ThreadLocalData* tls = getTLS(barrierWaiterThreadIds.at(i));
		tls->currentVectorClock->receiveAction(currentThreadTLS->currentVectorClock);
	}

	for (int i=0; i < samebarrierThreadCount; ++i)
	{
		ThreadLocalData* tls = getTLS(barrierWaiterThreadIds.at(i));
		tls->currentVectorClock->receiveAction(currentThreadTLS->currentVectorClock);
		tls->currentVectorClock->event();
		tls->createNewSegment = true;
		tls->nextBarrierAddr = 0;
	}
	currentThreadTLS->currentVectorClock->event();
	currentThreadTLS->nextBarrierAddr = 0;
	currentThreadTLS->createNewSegment = true;

	vector<barrierQueueStruct>::iterator it = barrierQueue.begin();
	for(; it != barrierQueue.end() ; )
	{
		if (  (*it).barrierAddr == barrierAddr)
		{
			barrierQueue.erase(it);
			it = barrierQueue.begin();
		}
		else
			++it;
	}


 	ReleaseLock(&barrierQueueLock);
}





/*
 *  when a cond variable is bcasted, add an element to
 *  signallers queue, and let all the waiter threads to utilze this
 *  signal in order to create HB relation
 *
 * */
VOID CondBCastAnalysis(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	//cout << "Thread " << id << "  bcasts on " << condVarAddr << endl;
	bool condVarAlreadyExists = false;
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	VectorClock *threadsCurrentClock = new VectorClock(tls->currentVectorClock, id);

	GetLock(&signalQueueLock, id);
		int signalQueueSize = signalQueue.size();
		for (int i=0; i < signalQueueSize; ++i)
		{
			if (signalQueue.at(i).condVarAddr == condVarAddr)
			{
				condVarAlreadyExists = true;
				delete signalQueue.at(i).clock;
				signalQueue.at(i).clock =  new VectorClock(threadsCurrentClock, id);
				break;
			}
		}

		if (!condVarAlreadyExists)
		{
			signalQueueStruct s(condVarAddr, threadsCurrentClock );
			signalQueue.push_back(s);
		}
	ReleaseLock(&signalQueueLock);
	tls->currentVectorClock->event();
	ReleaseLock(&tls->threadLock);

}

/*
 *  when a cond variable is bcasted, add an element to
 *  signallers queue, and let one of the waiter threads to utilze this
 *  signal in order to create HB relation
 *
 * */
VOID CondSignalAnalysis(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{

	bool condVarAlreadyExists = false;
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	VectorClock *threadsCurrentClock = new VectorClock(tls->currentVectorClock, id);

	GetLock(&signalQueueLock, id);
		int signalQueueSize = signalQueue.size();
		for (int i=0; i < signalQueueSize; ++i)
		{
			if (signalQueue.at(i).condVarAddr == condVarAddr)
			{
				condVarAlreadyExists = true;
				delete signalQueue.at(i).clock;
				signalQueue.at(i).clock =  new VectorClock(threadsCurrentClock, id);
				break;
			}
		}

		if (!condVarAlreadyExists)
		{
			//cout << "threadsCurrentClock:" << *threadsCurrentClock << endl;
			signalQueueStruct s(condVarAddr, threadsCurrentClock );
			signalQueue.push_back(s);
		}
	ReleaseLock(&signalQueueLock);
	tls->currentVectorClock->event();
	ReleaseLock(&tls->threadLock);
}


/*
 * Save semaphore  address to
 * thread local storage
 * until semWaitAnalysisExit is called
 * */

VOID semWaitAnalysisEnter(ADDRINT semAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	//	cout << "Thread " << id << " entered to condWait:" << condVarAddr << endl;
		ThreadLocalData* tls = getTLS(id);
		GetLock(&tls->threadLock, id);
		tls->nextSemaphoreAddr = semAddr;
		//cout << "Thread " << id << "  enters  semaphore wait: " << semAddr << endl;
		ReleaseLock(&tls->threadLock);

}

/*
 *  when a sem is waited, add an element to
 *  sem post queue, and let one of the waiter threads to utilze this
 *  post in order to create HB relation
 *
 * */
VOID semWaitAnalysisExit(ADDRINT semAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	semAddr = tls->nextSemaphoreAddr;

	//sem queue can be used identically for semaphores
	semQueueStruct tmpSem;

	GetLock(&semQueueLock, id);
	int semQueueSize = semQueue.size();
	for (int i=0; i < semQueueSize; ++i)
	{
		if (semQueue.at(i).semAddr == semAddr)//queue dan ilk buldugunu alacan
		{
			tmpSem = semQueue.at(i);
			break;
		}
	}
	ReleaseLock(&semQueueLock);

	if (tmpSem.clock == NULL)
		cout << "OOPS semaphore Error" << endl;

	tls->currentVectorClock->receiveAction(tmpSem.clock);
	//cout << "TLS received:" << *(tls->currentVectorClock) << endl;
	tls->currentVectorClock->event();
	tls->createNewSegment = true;
	ReleaseLock(&tls->threadLock);

}

/*
 *  when a sem is posted, add an element to
 *  sem post queue, and let one of the waiter threads to utilze this
 *  post in order to create HB relation
 *
 * */
VOID semPostAnalyis(ADDRINT semAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{

	//bool semAddressAlreadyExists = false;
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	VectorClock *threadsCurrentClock = new VectorClock(tls->currentVectorClock, id);

	GetLock(&semQueueLock, id);
	//her post u  queue ya koyacan
	semQueueStruct s(semAddr, threadsCurrentClock );
	semQueue.push_back(s);
	ReleaseLock(&semQueueLock);
	tls->currentVectorClock->event();
	ReleaseLock(&tls->threadLock);
}
