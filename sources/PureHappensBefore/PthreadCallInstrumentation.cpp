/*
 * PthreadCallInstrumentation.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */

#include "PthreadCallInstrumentation.h"
#include "ThreadLocalData.h"
#include "AllVariables.h"
#include "Lock.h"
using namespace std;


#define MALLOC "malloc"
#define FREE "free"
#define CALLOC "calloc"
#define REALLOC "realloc"


vector <functionNameInstrumentFunctionName> functionNamesEntry;
vector <functionNameInstrumentFunctionName> functionNamesExit;
vector <functionNameInstrumentFunctionName> functionNamesEnterTwoParameters;

//TODO handle lost signals

vector <waitQueueStruct> waiterQueue;
vector <semQueueStruct> semQueue;
vector <barrierQueueStruct> barrierQueue;


PIN_LOCK outputLock;
PIN_LOCK signalQueueLock;
PIN_LOCK barrierQueueLock;
PIN_LOCK semQueueLock;


map<ADDRINT, ADDRINT> mallocCounts; //map will look like map[memoryAddrStart] = ( malloced bytes size)
unordered_map<ADDRINT, Lock*> allLocks;
PIN_LOCK lockAcqRel;

PIN_LOCK mallocCountsLock;
extern AllMemomryAddresses allMemoryLocations;
extern PIN_LOCK variablePinLocks[1];


int mallocCountsMaxSize = 0;

//all instrumentation to pthread lock calls are similar, so the way we do instrumentation is the same.
//thus we need some initilization for function_name and instrumentation_function_name mapping. Then, we will use this vector in a single loop
void initFunctionNameInstrumentPthread()
{



	InitLock(&outputLock);
	InitLock(&signalQueueLock);
	InitLock(&barrierQueueLock);
	InitLock(&mallocCountsLock);
	InitLock(&lockAcqRel);
	InitLock(&semQueueLock);




	//locks
	functionNameInstrumentFunctionName mutexInit;
	mutexInit.functionName = "pthread_mutex_init";
	mutexInit.instrumentFunction = (AFUNPTR)LockInit;
	mutexInit.iPoint = IPOINT_BEFORE;
	mutexInit.callOrder = 2;
	functionNamesEntry.push_back(mutexInit);

	//locks
	functionNameInstrumentFunctionName mutexGetExit;
	mutexGetExit.functionName = "pthread_mutex_lock";
	mutexGetExit.instrumentFunction = (AFUNPTR)GetLockAnalysis;
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
	mutexGet.instrumentFunction = (AFUNPTR)GetLockAnalysisEnter;
	mutexGet.iPoint = IPOINT_BEFORE;
	mutexGet.callOrder = 1;
	functionNamesEntry.push_back(mutexGet);





	//locks
	functionNameInstrumentFunctionName rwWriteGet;
	rwWriteGet.functionName = "pthread_rwlock_wrlock";
	rwWriteGet.instrumentFunction = (AFUNPTR)GetLockAnalysisEnter;
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
	rwLockWriteLock.instrumentFunction = (AFUNPTR)GetLockAnalysis;
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
	rwLockReadLock.instrumentFunction = (AFUNPTR)GetLockAnalysis;
	rwLockReadLock.iPoint = IPOINT_AFTER;
	rwLockReadLock.callOrder =  4;
	functionNamesEntry.push_back(rwLockReadLock);

	functionNameInstrumentFunctionName rwLockReadLockEnter;
	rwLockReadLockEnter.functionName = "pthread_rwlock_rdlock";
	rwLockReadLockEnter.instrumentFunction = (AFUNPTR)GetLockAnalysisEnter;
	rwLockReadLockEnter.iPoint = IPOINT_BEFORE;
	rwLockReadLockEnter.callOrder =  3;
	functionNamesEntry.push_back(rwLockReadLockEnter);

	//signal -  wait
	functionNameInstrumentFunctionName condWait;
	condWait.functionName = "pthread_cond_wait";
	condWait.instrumentFunction = (AFUNPTR)CondWaitAnalysisEnter;
	condWait.iPoint = IPOINT_BEFORE;
	condWait.callOrder =  7;
	functionNamesEnterTwoParameters.push_back(condWait);


	functionNameInstrumentFunctionName condWaitIAfter; //instrument after,  wait diyen birden cok thread varsa, hangisinin kalkacagini bilmek gerekir
	condWaitIAfter.functionName = "pthread_cond_wait";
	condWaitIAfter.instrumentFunction = (AFUNPTR)CondWaitAnalysisExit;
	condWaitIAfter.iPoint = IPOINT_AFTER; //?/?/?/?/?/
	condWaitIAfter.callOrder =  1;
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

VOID ReallocEnter(CHAR * name, ADDRINT previousAddress, ADDRINT newSize, THREADID tid)
{
	ThreadLocalData* tls = getTLS(tid);
	GetLock(&tls->threadLock, tid);
	//cout << "Realloc Enter:" << previousAddress << "   " << newSize << endl;
	tls->nextReallocAddr = previousAddress ;
	tls->nextReallocSize = newSize;
	ReleaseLock(&tls->threadLock);
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
		newTmpMemAddrObj = new MemoryAddr(newMemAddrTmp, true); //yeni mem adresi olustur
		(*tmpHashMap)[newMemAddrTmp] = newTmpMemAddrObj;
		newTmpMemAddrObj->readerVectorClock = tmpMemAddr->readerVectorClock; //vector clocklari al
		newTmpMemAddrObj->writerVectorClock = tmpMemAddr->writerVectorClock;
		tmpMemAddr->readerVectorClock = NULL; //eskinin vectorClocklari null'a
		tmpMemAddr->writerVectorClock = NULL;
		ReleaseLock(&variablePinLocks[(newMemAddrTmp >> 3) & allMemoryLocations.bitMask]);

		GetLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask], tid);
		tmpHashMap = allMemoryLocations.getVariableHashMap(currentAddress);
		tmpHashMap->erase(currentAddress);
		delete tmpMemAddr;
		ReleaseLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask]);

	}
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

		delete (*tmpHashMap)[currentAddress]->writerVectorClock;
		delete  (*tmpHashMap)[currentAddress]->readerVectorClock;

		(*tmpHashMap).erase(currentAddress);
		ReleaseLock(&variablePinLocks[(currentAddress >> 3) & allMemoryLocations.bitMask]);
	}
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


	//cout << "countFreedMem:" << countFreedMem << endl;
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

	//if (string(imageName).find(libpthread) == string::npos)
		//return; //here we only instrument pthread function calls, other images should not be instrumented


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
	for (UINT32 i = 0; i < functionNamesEnterTwoParameters.size(); ++i )
	{
		functionNameInstrumentFunctionName currentType = functionNamesEnterTwoParameters.at(i);
		RTN pthreadMutexRTN = RTN_FindByName(img, currentType.functionName.c_str());
		if (RTN_Valid(pthreadMutexRTN))
		{

			RTN_Open(pthreadMutexRTN);
			// Instrument pthread_mutex_lock() to print the input argument value and the return value.
			RTN_InsertCall(pthreadMutexRTN, currentType.iPoint,
					(AFUNPTR)currentType.instrumentFunction,
					IARG_FUNCARG_ENTRYPOINT_VALUE,
					0,
					IARG_FUNCARG_ENTRYPOINT_VALUE,
					1,
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


VOID createNewLock(ADDRINT lockAddr, THREADID tid)
{
	GetLock(&lockAcqRel, tid);
	//cout << "Create new lock:" << hex << lockAddr << dec << endl;
	if (allLocks.count(lockAddr) == 0)
	{
		Lock* tmp = new Lock(lockAddr);
		allLocks[lockAddr] = tmp;
		//cout << "Created Lock:" << hex << lockAddr << dec << endl;
	}
	ReleaseLock(&lockAcqRel);
}

VOID releaseLock(ADDRINT lockAddr, THREADID tid)
{
	VectorClock* threadsVC = NULL;
	ThreadLocalData* tls = getTLS(tid);
	GetLock(&tls->threadLock, tid);
	threadsVC = tls->currentVectorClock;
	tls->currentVectorClock->event();
	ReleaseLock(&tls->threadLock);

	GetLock(&lockAcqRel, tid);
	//cout << "Release lock:" << hex << lockAddr << dec << endl;
	if (allLocks.count(lockAddr) != 0)
	{
		Lock* tmp = allLocks[lockAddr];
		tmp->lockVectorClock->receiveAction(threadsVC); //OOPS there is little bit possibility of race on threadsVc, otherwise possibility
		//cout << "ReleasedLock:" << endl << *(tmp->lockVectorClock) << endl;
	}
	else
		cout << "OOPS, released lock is NOT alive" << endl;
	ReleaseLock(&lockAcqRel);

}

//burada thread'in hangi lock'u aldigini tutacagiz, lock address' function entry'de yapmamiz gerekir
//ancak esas yapmamiz gereken instrumentation, exit'te yapilmali, o yuzden boyle bir yola girdim
VOID GetLockAnalysisEnter(ADDRINT lockAddr, THREADID id ,char* imageName , ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	//cout << "Thread " << id << "  gets lock:" << hex << lockAddr << dec << endl;
	tls->nextLockAddr = lockAddr;
	ReleaseLock(&tls->threadLock);
	createNewLock(lockAddr, id);
}





VOID mutexTryLockEnterAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName , ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->nextLockAddr = lockAddr;
	ReleaseLock(&tls->threadLock);
	createNewLock(lockAddr, id);
}



VOID LockInit(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	//cout << "Lock inited:" << lockAddr << endl;
	createNewLock(lockAddr, id);
	releaseLock(lockAddr, id);
}


//aslinda burada lock'ta global mi diye bakabiliriz, ama cok sacma olur local bir lock kullandildiysa!!!!!
VOID GetLockAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	lockAddr = 	tls->nextLockAddr;
	//cout << "thread "  << id << "  gets lock" << endl;
	ReleaseLock(&tls->threadLock);

	Lock* tmp = NULL;
	GetLock(&lockAcqRel, id);
	if (allLocks.count(lockAddr) != 0)
		tmp = allLocks[lockAddr];
	else
	{
		cout << "OOPS, gotten lock is NOT alive:" << hex << lockAddr << dec << endl;
		ReleaseLock(&lockAcqRel);
		return;
	}
	ReleaseLock(&lockAcqRel);


	GetLock(&tls->threadLock, id);
	//cout <<"Received Clocked:"<< *(tmp->lockVectorClock) << endl;
	tls->currentVectorClock->receiveAction(tmp->lockVectorClock);
	//cout << "tls->currentVectorClock:" << *(tls->currentVectorClock) << endl;

	ReleaseLock(&tls->threadLock);
}


//aslinda burada lock'ta global mi diye bakabiliriz, ama cok sacma olur local bir lock kullandildiysa!!!!!
VOID mutexTryLockExitAnalysis(ADDRINT exitVal, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	if (exitVal == 0) //yani lock'u alabildiysek
	{
		ThreadLocalData* tls = getTLS(id);
		GetLock(&tls->threadLock, id);
		ADDRINT lockAddr = tls->nextLockAddr;
		ReleaseLock(&tls->threadLock);
		releaseLock(lockAddr, id);
	}
}


//aslinda burada lock'ta global mi diye bakabiliriz, ama cok sacma olur local bir lock kullandildiysa!!!!!
VOID ReleaseLockAnalyis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	releaseLock(lockAddr, id);
}


VOID updateWaiterQueue(VectorClock* vc, ADDRINT condVarAddr, THREADID id)
{
	bool thereExistsCondVar = false;
	GetLock(&signalQueueLock, id);

	int size = waiterQueue.size();
	for (int i = 0; i < size; ++i)
	{
		if (waiterQueue.at(i).condVarAddr == condVarAddr)
		{
			thereExistsCondVar = true;
			waiterQueue.at(i).clock->receiveAction(vc);
			//cout << "Waiter Queue Lock:" << " id:" << id  << endl<< *(waiterQueue.at(i).clock) << endl;
			break;
		}

	}

	if (!thereExistsCondVar)
	{
		//cout << "Thread: " << id << "  created the cond var " << endl;
		waitQueueStruct tmp(condVarAddr, new VectorClock(vc, NON_THREAD_VECTOR_CLOCK));
		waiterQueue.push_back(tmp);
	}
	ReleaseLock(&signalQueueLock);

}

VOID updateThreadVectorClockByCondVariable(VectorClock* vc, ADDRINT condVarAddr, THREADID id)
{

	bool thereExistsCondVar = false;
	GetLock(&signalQueueLock, id);

	int size = waiterQueue.size();
	for (int i = 0; i < size; ++i)
	{
		if (waiterQueue.at(i).condVarAddr == condVarAddr)
		{
			thereExistsCondVar = true;
			vc->receiveAction(waiterQueue.at(i).clock);
			break;
		}
	}

	if (!thereExistsCondVar)
	{
		cout << "OOPS, there is no avaliable condition variable" << endl;
	}
	ReleaseLock(&signalQueueLock);
}


VOID CondWaitAnalysisEnter(ADDRINT condVarAddr,ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->nextWaitCondVariableAddr = condVarAddr;
	tls->nextWaitCondVariableLockAddr = lockAddr;
	//tls->nextLockAddr = lockAddr;
	updateWaiterQueue(tls->currentVectorClock, condVarAddr, id);
	ReleaseLock(&tls->threadLock);

	releaseLock(lockAddr, id);
}


VOID CondWaitAnalysisExit(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{

	ADDRINT releasedLockAddr;
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);

	condVarAddr = tls->nextWaitCondVariableAddr;
	releasedLockAddr = tls->nextWaitCondVariableLockAddr;
	//cout << "Thread " << id << " exit to condWait:" << condVarAddr << endl;

	updateThreadVectorClockByCondVariable(tls->currentVectorClock, condVarAddr, id);

	//cout << "TLS received:" << *(tls->currentVectorClock) << endl;
	//tls->currentVectorClock->event();
	tls->nextWaitCondVariableAddr = 0;
	tls->nextWaitCondVariableLockAddr = 0;
	//tls->nextLockAddr = 0;
	ReleaseLock(&tls->threadLock);

	GetLockAnalysis(releasedLockAddr, id, NULL, 0);



}

/*

//aslinda pthread_cond_wait(cv, lock), 2 tane argumani var, ancak, 2. arguman olan lock'un bizle bir alakasi yok
VOID CondWaitAnalysisEnter(ADDRINT condVarAddr,ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	cout << "Thread " << id << " entered to condWait:" << condVarAddr << endl;

	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	tls->nextWaitCondVariableAddr = condVarAddr;
	//tls->nextWaitCondVariableLockAddr = lockAddr;
	tls->nextLockAddr = lockAddr;
	//tls->currentVectorClock->event();

	ReleaseLock(&tls->threadLock);

	//GetLockAnalysis(lockAddr, id, NULL,0);
}

//pthread_cond_wait'ten cikista cagirilacak call back
VOID CondWaitAnalysisExit(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{

	ADDRINT releasedLockAddr;
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);

	condVarAddr = tls->nextWaitCondVariableAddr;
	tls->currentVectorClock->event();
	releasedLockAddr = tls->nextLockAddr;
	//cout << "Thread " << id << " exit to condWait:" << condVarAddr << endl;

	signalQueueStruct tmpSignal;

	GetLock(&signalQueueLock, id);
	int signalQueueSize = signalQueue.size();
	for (int i=0; i < signalQueueSize; ++i)
	{
		if (signalQueue.at(i).condVarAddr == condVarAddr)
		{
			tmpSignal = signalQueue.at(i);
			//cout << "signal received: clock:"  << *(tmpSignal.clock ) << endl;
			break;
		}
	}
	ReleaseLock(&signalQueueLock);

	if (tmpSignal.clock == NULL)
		cout << "OOPS signal Error" << endl;

	tls->currentVectorClock->receiveAction(tmpSignal.clock);

	//cout << "TLS received:" << *(tls->currentVectorClock) << endl;
	//tls->currentVectorClock->event();
	tls->nextWaitCondVariableAddr = 0;
	//tls->nextWaitCondVariableLockAddr = 0;
	tls->nextLockAddr = 0;
	ReleaseLock(&tls->threadLock);

	releaseLock(releasedLockAddr, id);


}
*/



VOID BarrirerWaitAnalysisEnter(ADDRINT barrierAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	barrierQueueStruct b(barrierAddr, id);
	GetLock(&barrierQueueLock, id);
	barrierQueue.push_back(b);
	//cout << "Thread:" << id << "  barrier:" << hex << barrierAddr << dec << endl;
	ReleaseLock(&barrierQueueLock);
	ThreadLocalData* currentThreadTLS = getTLS(id);
	GetLock(&currentThreadTLS->threadLock, id);
	currentThreadTLS->nextBarrierAddr = barrierAddr;
	ReleaseLock(&currentThreadTLS->threadLock);



}


VOID BarrirerWaitAnalysisExit(ADDRINT barrierAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	GetLock(&barrierQueueLock, id);
	ThreadLocalData* currentThreadTLS = getTLS(id);
	barrierAddr = currentThreadTLS->nextBarrierAddr;
	currentThreadTLS->nextBarrierAddr = 0;
	if (barrierAddr == 0)
	{
		ReleaseLock(&barrierQueueLock);
		return;
	}

	vector<THREADID> barrierWaiterThreadIds;

	int size =  barrierQueue.size();
	//cout << "barrierQueue.size:" << barrierQueue.size() << endl;
	for (int i=0; i < size; ++i)
	{
		if (barrierQueue.at(i).barrierAddr == barrierAddr)
		{
			barrierWaiterThreadIds.push_back(barrierQueue.at(i).tid );
			//cout << "same barrier id:" << barrierQueue.at(i).tid << endl;
		}
	}
	int samebarrierThreadCount = barrierWaiterThreadIds.size();


	for (int i=0; i <samebarrierThreadCount ;++i)
	{
		ThreadLocalData* tls = getTLS(barrierWaiterThreadIds.at(i));
		GetLock(&tls->threadLock, barrierWaiterThreadIds.at(i));
		tls->currentVectorClock->event();
		tls->nextBarrierAddr = 0;
		ReleaseLock(&tls->threadLock);
	}

	for (int i=0; i < samebarrierThreadCount;++i)
		for (int j=0; j < samebarrierThreadCount;++j)
		{
			ThreadLocalData* tls = getTLS(barrierWaiterThreadIds.at(i));
			GetLock(&tls->threadLock, barrierWaiterThreadIds.at(i));
			tls->currentVectorClock->receiveAction(getTLS(barrierWaiterThreadIds.at(j))->currentVectorClock);
			ReleaseLock(&tls->threadLock);
		}

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





//pthread_cond_signal(cv)
VOID CondBCastAnalysis(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	CondSignalAnalysis(condVarAddr, id, NULL, stackPtr);
	/*
	bool condVarAlreadyExists = false;
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	VectorClock *threadsCurrentClock = new VectorClock(tls->currentVectorClock, id);

	GetLock(&signalQueueLock, id);
	//cout << "Thread " << id << "  bcasts on "  << hex << condVarAddr << dec<< endl;

		int signalQueueSize = signalQueue.size();
		for (int i=0; i < signalQueueSize; ++i)
		{
			if (signalQueue.at(i).condVarAddr == condVarAddr)
			{
				//cout << " avaliable threads:" << signalQueue.at(i).condVarAddr << endl;
				condVarAlreadyExists = true;
				//delete signalQueue.at(i).clock;
				signalQueue.at(i).clock->receiveAction(tls->currentVectorClock);// =  new VectorClock(threadsCurrentClock, id);
				break;
			}
		}

		if (!condVarAlreadyExists)
		{
			signalQueueStruct s(condVarAddr, threadsCurrentClock );
			signalQueue.push_back(s);
		}
		else
			delete threadsCurrentClock;
	ReleaseLock(&signalQueueLock);
	tls->currentVectorClock->event();


	ReleaseLock(&tls->threadLock);
	*/

}


//pthread_cond_signal(cv)
VOID CondSignalAnalysis(ADDRINT condVarAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	ThreadLocalData* tls = getTLS(id);
	GetLock(&tls->threadLock, id);
	//cout << " Signal analysis:" << hex << condVarAddr << dec << endl;
	VectorClock *threadsCurrentClock = new VectorClock(tls->currentVectorClock, id);
	updateWaiterQueue(threadsCurrentClock, condVarAddr, id);
	ReleaseLock(&signalQueueLock);
	tls->currentVectorClock->event();
	ReleaseLock(&tls->threadLock);
}

/*

//pthread_cond_signal(cv)
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
				//delete signalQueue.at(i).clock;
				//signalQueue.at(i).clock =  new VectorClock(threadsCurrentClock, id);
				signalQueue.at(i).clock->receiveAction(tls->currentVectorClock);
				break;
			}
		}

		if (!condVarAlreadyExists)
		{
			//cout << "threadsCurrentClock:" << *threadsCurrentClock << endl;
			signalQueueStruct s(condVarAddr, threadsCurrentClock );
			signalQueue.push_back(s);
		}
		else
			delete threadsCurrentClock;
	ReleaseLock(&signalQueueLock);
	tls->currentVectorClock->event();
	ReleaseLock(&tls->threadLock);
}
*/


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
	ReleaseLock(&tls->threadLock);

}
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



VOID semWaitAnalysisEnter(ADDRINT semAddr, THREADID id ,char* imageName, ADDRINT stackPtr)
{
	//	cout << "Thread " << id << " entered to condWait:" << condVarAddr << endl;
		ThreadLocalData* tls = getTLS(id);
		GetLock(&tls->threadLock, id);
		tls->nextSemaphoreAddr = semAddr;
		//cout << "Thread " << id << "  enters  semaphore wait: " << semAddr << endl;
		ReleaseLock(&tls->threadLock);

}


