
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
 * RaceDetectorMain.cpp
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */


/*
 * ThreadSimple.cpp
 *
 *  Created on: Mar 14, 2013
 *      Author: onder
 */



#include "definitions.h"
#include "PthreadCallInstrumentation.h"
#include "ThreadLocalData.h"
#include "MemoryAddr.h"
#include "MemoryAddressInstrumentation.h"
#include "AllVariables.h"
#include "AllRaceInstances.h"


PIN_LOCK thread_lock;
TLS_KEY tls_key;
AllMemomryAddresses allMemoryLocations;
PIN_LOCK variablePinLocks[4096];

PIN_LOCK testLock;

PIN_LOCK segmentDeleteLock;
extern AllRaceInstances races;
map<THREADID, THREADID> mapOfThreadIds;
map<THREADID, BOOL> deadThreadIds;

//UINT64 segmentComparisonCount = 0;
//UINT64 memoryAccessCount = 0;
//PIN_LOCK segmentCompareLock;

int totalThreadCount = 0;

VOID threadCretator();

VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
	cout << "Thread Start:" << threadid << endl;

	if (threadid == 0)
	{
		GetLock(&thread_lock, threadid);
		deadThreadIds[threadid] = false;
		mapOfThreadIds[threadid] = PIN_GetTid();
		ReleaseLock(&thread_lock);
		ThreadLocalData* tld = new ThreadLocalData(threadid);
		PIN_SetThreadData(tls_key, tld, threadid);
		return;
	}
	THREADID parentThreadId = 0;
	GetLock(&thread_lock, threadid);
	mapOfThreadIds[threadid] = PIN_GetTid();
	THREADID parentTid = PIN_GetParentTid();
	if (mapOfThreadIds.count(parentTid) > 0)
		parentThreadId = mapOfThreadIds[parentTid];


	ReleaseLock(&thread_lock);
	ThreadLocalData* parentTls = getTLS(parentThreadId);
	ThreadLocalData* tld = new ThreadLocalData(threadid);
	PIN_SetThreadData(tls_key, tld, threadid);

	GetLock(&parentTls->threadLock, threadid);
	GetLock(&tld->threadLock, threadid);
	++totalThreadCount;
	parentTls->currentVectorClock->event();

	tld->currentVectorClock->receiveAction(parentTls->currentVectorClock);
	//cout << "threads' initial clock:" << *(tld->currentVectorClock) << endl;
	//cout << "parrents' initial clock:" << *(parentTls->currentVectorClock) << endl;

	ReleaseLock(&tld->threadLock);
	ReleaseLock(&parentTls->threadLock);
}

VOID ThreadFini(THREADID threadid, const CONTEXT *ctxt, INT32 code, VOID *v)
{

	if (threadid == 0)
		return;
	cout << "Thread Finish:" << threadid << endl;
	THREADID parentThreadId = 0;
	GetLock(&thread_lock, threadid);
	mapOfThreadIds[threadid] = PIN_GetTid();
	THREADID parentTid = PIN_GetParentTid();
	if (mapOfThreadIds.count(parentTid) > 0)
		parentThreadId = mapOfThreadIds[parentTid];
	//deadThreadIds[threadid] = true;
	ReleaseLock(&thread_lock);


	ThreadLocalData* parentTls = getTLS(parentThreadId);

	ThreadLocalData* tls = getTLS(threadid);
	tls->isAlive = false;
	GetLock(&parentTls->threadLock, parentTid);
	GetLock(&tls->threadLock, parentTid);

	parentTls->currentVectorClock->receiveAction(tls->currentVectorClock);
	parentTls->currentVectorClock->event();
//	cout << "Thread Finished:" << *(tls->currentVectorClock) << endl << "parent:" << *(parentTls->currentVectorClock) << endl;
	ReleaseLock(&tls->threadLock);
	ReleaseLock(&parentTls->threadLock);

}

VOID initLogger()
{

}




VOID finializeLogger()
{


}

#include <pthread.h>
int tester = 0;
int LOOP_COUNT = 10000;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m_top = PTHREAD_MUTEX_INITIALIZER;


void* thread_f(void*)
{
	pthread_mutex_lock(&m_top);
	for (int i=0; i < LOOP_COUNT; ++i)
	{
		pthread_mutex_lock(&m);
		++tester;
		pthread_mutex_unlock(&m);
	}
	pthread_mutex_unlock(&m_top);
	return NULL;
}

VOID threadCretator()
{
	int THREADCOUNT = 20;
	pthread_t threads[THREADCOUNT];
	for(int i=0; i < THREADCOUNT; ++i)
		pthread_create(&threads[i], NULL, thread_f, NULL);

	for(int i=0; i < THREADCOUNT; ++i)
		pthread_join(threads[i], NULL);
	cout << "Tester Count:" << tester << endl;
}

extern unordered_map<ADDRINT, ADDRINT> mallocCounts;

extern int mallocCountsMaxSize;


// This routine is executed once at the end.
VOID Fini(INT32 code, VOID *v)
{
	//finializeLogger();
	//threadCretator();
	//THREADID threadid = 0;
	/*for (int i=0; i < totalThreadCount; ++i)
	{
		ThreadLocalData* tls = getTLS(i);
		++threadid;
		cout <<"thread:" << threadid << ", EventCount:" << tls->threadDataTimes.size() << endl;
	}*/

	UINT64 totalMemSize = 0;
	int size =  allMemoryLocations.variableHashMaps.size();
	for (int i=0; i <size;++i)
	{
	//	cout << "i:" << i << "  size:" << allMemoryLocations.variableHashMaps.at(i)->size() << endl;
		totalMemSize += allMemoryLocations.variableHashMaps.at(i)->size();

	}


	cout << "Total Mem size : " << totalMemSize << endl;

	races.removeSameLineRaces();
	races.removeSameInstPtr();
	int totalRaceCount = 0;
	cout <<  "Total Race Count by races::" << races.getRaceCount() << endl;
	std::map<ADDRINT, RaceInstance*>::iterator it;
	for (it = races.allRaces.begin(); it != races.allRaces.end(); ++it)
		if (it->second != NULL && it->second->print)
			try
	{
				//LOG4CPLUS_INFO(logger, it->second->toStringDetailed());
				cout <<  it->second->toStringDetailed() << endl;
				++totalRaceCount;
	}
	catch(...)
	{
		//LOG4CPLUS_INFO(logger, "Exception During RaceInstance.toStringDetailed");

	}
	cout << dec ;
	//cout << "HBB_isRaceCountNew:" << memoryAccessCount << endl;
	//cout << "HBB_segmentComparisonCount:" << segmentComparisonCount << endl;
	//cout << "HBB_Results:" << (segmentComparisonCount * 1.0) / memoryAccessCount << endl;
	cout << "Exact totalRaceCount:" << totalRaceCount << endl;
	//allMemoryLocations.calculateAllCapacity();

	return;

}


/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This Pintool prints a trace of malloc calls in the guest application\n"
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

//KNOB<UINT64> KNOB_MAX_SIM_COUNT (KNOB_MODE_WRITEONCE, "pintool",   "max_sim_count", "0", "maximum simulation cycle count");
//KNOB<BOOL> BoolTest (KNOB_MODE_WRITEONCE, "pintool",   "booltest", "0", "maximum simulation cycle count");

int main(INT32 argc, CHAR **argv)
{


	initFunctionNameInstrumentPthread();
	for (int i=0; i < 4096; ++i)
		InitLock(&variablePinLocks[i]);

	InitLock(&segmentDeleteLock);

	if (PIN_Init(argc, argv)) return Usage();
	PIN_InitSymbols();
	InitLock(&thread_lock);
	InitLock(&testLock);
	tls_key = PIN_CreateThreadDataKey(0);

	//InitLock(&segmentCompareLock);
    PIN_AddThreadStartFunction(ThreadStart, 0); //for thread start instrumentation
    PIN_AddThreadFiniFunction(ThreadFini, 0); //for thread finish instrumentation
    IMG_AddInstrumentFunction(InstrumentImage, 0);//for function call instrumentation, PthreadCallInstrumentation
    PIN_AddFiniFunction(Fini, 0);
    TRACE_AddInstrumentFunction(InstrumentSegment, 0); //for instruction read/write instrumentation, MemoryAdressInstrumentation
   // PIN_AddFiniUnlockedFunction(UnlockedFini, 0);

 //   cout << "KNOB_MAX_SIM_COUNT.Value():" << KNOB_MAX_SIM_COUNT.Value() << endl;
  //  cout << "BoolTest.Value():" << BoolTest.Value() << endl;


    PIN_StartProgram();

    return 0;
}
