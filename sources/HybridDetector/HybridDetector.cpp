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
*
* This file includes the main method
* All user input is explained
*
 */


#include "definitions.h"
#include "PthreadCallInstrumentation.h"
#include "ThreadLocalData.h"
#include "AllLockSets.h"
#include "MemoryAddr.h"
#include "MemoryAddressInstrumentation.h"
#include "AllVariables.h"
#include "AllRaceInstances.h"
#include "Segment.h"
#include "SynronizedQueue.h"
//#include "VCHashMap.h"

PIN_LOCK thread_lock;  //only used when thread is created/joined
TLS_KEY tls_key; //this is required by PIN for thread local storage
AllLockSets allLockSets; //shadow memory for all locksets
AllMemomryAddresses allMemoryLocations;  //shadow memory for memory addresses
PIN_LOCK* variablePinLocks;
PIN_LOCK testLock;
//PIN_LOCK VCHistoryLock;

PIN_LOCK segmentDeleteLock;
vector<Segment*> segmentsToBeDeleted( 2 * 10000 + 1);
UINT64 vcHistorySize = 1000;
//VCHashMap vcHashMap(vcHistorySize);

extern AllRaceInstances races;
map<THREADID, THREADID> mapOfThreadIds; //this data structure is utilized for keeping all thread's parent information
map<THREADID, BOOL> deadThreadIds;
SynronizedQueue* syncQueue;  //this data structure is utilized for limiting the total number of segments throughout the execution
bool syncQueueEnabled;



int totalThreadCount = 0;



/*
 * Whenever a thread start event is captured by PIN, this callback is called
 *
   if not the first thread, get parent thread id
   it is required since vector clock information
   must be updated to its parent value
   and parent must execute an internal event, since we do not
   instrument thread_create we follow thread creation  via PIN callback
 * */
VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
	cout << "Thread Start:" << threadid << endl;
	cout.flush();


	if (threadid == 0) //if first thread is created, no parent info is gathered
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

	tld->currentVectorClock->receiveAction(parentTls->currentVectorClock);
	parentTls->currentVectorClock->event();

	ReleaseLock(&tld->threadLock);
	ReleaseLock(&parentTls->threadLock);
}

/*
 	when a thread finishes (not the main/first thread)
	find its parent thread and execute a join operation
	since we do not
	instrument thread_join we follow thread join via PIN callback
 */

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


/*
 * these parameters are opened if internal statistical analysis is conducted:

extern int totalLockCount ;
extern  int opCount;
extern  int nopCount;
extern  int totalCount;
extern  int smallerCount;
extern  int greaterCount;
*/





/*
 * All the below parameters are required when statistical analysis is contucted
 * Also, fini function is called when the whole execuiton is finished
 * And fini function is responsible for printing the output results
 * Since each of the print statement is self explained, the variables are not
 * explained one by one.
 * */

extern int deletedSegmentCount;

extern UINT32 cacheHit;
extern int nonCacheHit;
// This routine is executed once at the end.

extern  int sampledCount;
extern int nonSampledCount;
extern UINT64 firstCount;
extern UINT64 secondCount;
extern UINT64 isRaceCount;
extern UINT64 addSegmetnCount;
extern UINT64 clearAllSegmentsCount;
extern UINT64 removeHapppendCount;

extern map<UINT64, BOOL> lastSegmentCompareOperations;
extern UINT64 totalCounter ;
//UINT64 segmentComparisonCount = 0;
//UINT64 memoryAccessCount = 0;
//PIN_LOCK segmentCompareLock;
UINT64 vcHitCount = 0;
UINT64 vcNonHitCount = 0;
UINT64 pairCount = 0;
UINT64 addNodeCount = 0;
 UINT64 isPairExistsVectorCount = 0 ;
 UINT64 addVectorCount = 0 ;


VOID Fini(INT32 code, VOID *v)
{

	UINT64 totalMemSize = 0;
	int size =  allMemoryLocations.variableHashMaps.size();
	for (int i=0; i <size;++i)
	{
	//	cout << "i:" << i << "  size:" << allMemoryLocations.variableHashMaps.at(i)->size() << endl;
		totalMemSize += allMemoryLocations.variableHashMaps.at(i)->size();

	}

	cout << allLockSets.toString() << endl;

	cout << "Total Mem size : " << totalMemSize << endl;

	races.removeSameLineRaces();
	races.removeSameInsturtionPointer();
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
    cout << "firstCount:" << firstCount << endl;
	cout << "secondCount:" << secondCount << endl;
	cout << "addSegmetnCount:" << addSegmetnCount << endl;
	cout << " removeHapppendCount:" <<  removeHapppendCount << endl;
    cout << "sampled:" << sampledCount << "  nonSampled:" << nonSampledCount << "   Oran:" << 1.0*sampledCount / (sampledCount + nonSampledCount * 1.0) << endl;
	cout << "Cache hit:" << cacheHit << endl;
	cout << "non cache hit:" << nonCacheHit << endl;
	cout << "Total Segment Count:" << Segment::totalSegmentCount << endl;
	cout << "Total deleted Segment Count:" << deletedSegmentCount << endl;
	cout << "Queue Size:" << syncQueue->getSize() << endl;
	cout << "isRaceCount:" << isRaceCount << endl;
	cout << "Total deleted count:" << SegmentSet::totalDeletedCount << endl;
	//cout << "NEW_isRaceCountNew:" << memoryAccessCount << endl;
	//cout << "NEW_segmentComparisonCount:" << segmentComparisonCount << endl;
	//cout << "NEW_Results:" << (segmentComparisonCount * 1.0) / memoryAccessCount << endl;
    cout << "lastSegmentCompareOperations.size:" << lastSegmentCompareOperations.size() << endl;
    cout << "totalCounter:" << totalCounter << endl;
    cout << "vcHitCount:" << vcHitCount << endl;
    cout << "vcNonHitCount: " << vcNonHitCount << endl;
    cout << "VC/NONVC:" << ((vcHitCount*1.0)/vcNonHitCount) << endl;
    cout << "addNodeCount:" << addNodeCount << endl;
    cout << "Pair Count:" << pairCount << endl;
    cout << "isPairExistsVectorCount:" << isPairExistsVectorCount << endl;
    cout << "addVectorCount:" << addVectorCount << endl;
	cout << "Exact totalRaceCount:" << totalRaceCount << endl;
	//allMemoryLocations.calculateAllCapacity();

	syncQueueEnabled = syncQueue->isEnabled();


}

VOID UnlockedFini(INT32 code, VOID *v)
{
	cout << "LockAddrMapSize:"<< allLockSets.lockAddrMap.size() << endl;
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



/*
 * KNOB_MEMORY_CHUNK_COUNT and KNOB_TOTAL_VARIABLE_COUNT => Not Explained in the thesis, but works for system level parallesim, not in algorithmic level
 *
 *  KNOB_SEGMENT_HISTORY_SIZE => defines maximum segment count in the execution
 *  KNOB_MAX_SEGMENT_SIZE => defines maximum segment count in a single segment
 *  KNOB_SAMPLE_RATE => defines sample rate in range 0.0 - 1.0
 *  KNOB_ENABLE_SINGLE_ACCESS_IN_A_SEGMENT => if true, disables multiple accesses inside a segment
 *  KNOB_VCHISTORY => defines vector clock history cache for each vector clock
 *
 * */

KNOB<UINT64> KNOB_MEMORY_CHUNK_COUNT (KNOB_MODE_WRITEONCE, "pintool",   "mem_chunk_count", "1", "total memory chunk count");
KNOB<UINT64> KNOB_TOTAL_VARIABLE_COUNT (KNOB_MODE_WRITEONCE, "pintool",   "total_variable_address_size", "10000000", "total memory chunk count");
KNOB<UINT64> KNOB_SEGMENT_HISTORY_SIZE (KNOB_MODE_WRITEONCE, "pintool",   "segment_history_table_size", "0", "segment history table size");
KNOB<UINT64> KNOB_MAX_SEGMENT_SIZE (KNOB_MODE_WRITEONCE, "pintool",   "max_single_segment_set_size", "10000", "maximum single segment set size");
KNOB<double_t> KNOB_SAMPLE_RATE (KNOB_MODE_WRITEONCE, "pintool",   "sample_rate", "1.0", "maximum single segment set size");
KNOB<BOOL>  KNOB_ENABLE_SINGLE_ACCESS_IN_A_SEGMENT (KNOB_MODE_WRITEONCE, "pintool",   "enable_signle_access", "0", "enable signle access a segment");
KNOB<UINT64>  KNOB_VCHISTORY (KNOB_MODE_WRITEONCE, "pintool",   "maxVCHistoryCount", "0", "segment history size");


UINT64 totalNumberOfHashMaps;
UINT64 totalNumberOfMemoryAddresses;
UINT64 segmentHistoryTableSize;
UINT64 maxSegmentSizeCount;
double_t sampleRate;
UINT64 maxVCHistoryCount;
BOOL enableMaxSegmentSize = false;

BOOL enableSingleAccessCheck;


/*
 * Just a helper function, if x is power of two, return 2
 * */
BOOL isPowerOfTwo (UINT64 x)
{

	if (x < 0)
		return false;

	unsigned int numberOfOneBits = 0;
	while(x && numberOfOneBits <=1)
	{
		if ((x & 1) == 1) /* Is the least significant bit a 1? */
			numberOfOneBits++;
		x >>= 1;          /* Shift number one bit to the right */
	}

	return (numberOfOneBits == 1); /* 'True' if only one 1 bit */
}

/*
 *  not stated in the theises but related to
 *  KNOB_MEMORY_CHUNK_COUNT and KNOB_TOTAL_VARIABLE_COUNT
 *
 */
VOID initilizeAllMemoryLocations()
{
	cout << "Memory Chunk Count:" << totalNumberOfHashMaps << endl;
	variablePinLocks = (PIN_LOCK*) (malloc(sizeof(PIN_LOCK) * totalNumberOfHashMaps));
	for (int i=0; i < static_cast<INT64>(totalNumberOfHashMaps); ++i)
		InitLock(&variablePinLocks[i]);
}


/*
 * Main function gets user parameters
 * and registers required pin callbacks
 * */

int main(INT32 argc, CHAR **argv)
{

	std::srand ( unsigned ( std::time(0) ) );

	initFunctionNameInstrumentPthread();

	if (PIN_Init(argc, argv)) return Usage();
	PIN_InitSymbols();


	totalNumberOfHashMaps = KNOB_MEMORY_CHUNK_COUNT.Value();
	totalNumberOfMemoryAddresses = KNOB_TOTAL_VARIABLE_COUNT.Value();
	enableSingleAccessCheck = KNOB_ENABLE_SINGLE_ACCESS_IN_A_SEGMENT.Value();
	segmentHistoryTableSize = KNOB_SEGMENT_HISTORY_SIZE.Value();
	maxSegmentSizeCount = KNOB_MAX_SEGMENT_SIZE.Value();
	sampleRate = KNOB_SAMPLE_RATE.Value();
	maxVCHistoryCount = KNOB_VCHISTORY.Value();

	if (sampleRate > 1 )
	{
		cout << "Sample rate must be within 0 - 1" << endl;
		return 0;
	}
	else
		cout << "Sample rate:" << sampleRate << endl;
	sampleRate = sampleRate * 1024;

	if (maxSegmentSizeCount != 10000)
		enableMaxSegmentSize = true;
	cout << "segmentHistoryTableSize:" << segmentHistoryTableSize << endl;
	cout << "maxSegmentSizeCount:" << maxSegmentSizeCount << endl;
	cout << "maxVCHistoryCount:" << maxVCHistoryCount << endl;
	if (!isPowerOfTwo(totalNumberOfHashMaps))
	{
		cout << "mem_chunk_count must be power of 2" << endl;
		return 0;
	}


	initilizeAllMemoryLocations();
	allMemoryLocations.initilaze();
	syncQueue = new SynronizedQueue(segmentHistoryTableSize);


	//InitLock(&segmentCompareLock);
	InitLock(&thread_lock);
	InitLock(&testLock);
	InitLock(&segmentDeleteLock);
	//InitLock(&VCHistoryLock);


	tls_key = PIN_CreateThreadDataKey(0);


    PIN_AddThreadStartFunction(ThreadStart, 0); //for thread start instrumentation
    PIN_AddThreadFiniFunction(ThreadFini, 0); //for thread finish instrumentation
    IMG_AddInstrumentFunction(InstrumentImage, 0);//for function call instrumentation, PthreadCallInstrumentation
    PIN_AddFiniFunction(Fini, 0);
    TRACE_AddInstrumentFunction(InstrumentSegment, 0); //for instruction read/write instrumentation, MemoryAdressInstrumentation
   // PIN_AddFiniUnlockedFunction(UnlockedFini, 0);

    PIN_StartProgram();

    return 0;
}
