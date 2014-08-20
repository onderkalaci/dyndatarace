/*
 * VectorClock.cpp
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */
#include "VectorClock.h"
using namespace::std;

/**
 * Vector Clock Implementation
 *
 * **/

/*
 * This parameter is utilezed
 * for Vector Clock History Cache optimization
 * If this value equals zero, the optimization is not executed
 * */
extern UINT64 maxVCHistoryCount;



/*
 * total process count defines
*  the size of vector clock
*  for our experiments these parameters
*  are statically defined befor run time
*/
int VectorClock::totalProcessCount = 48;


/*
 * This function is intended for debug purposes
 * It prints vector clock nicely
 */
ostream& operator<<(ostream& os, const VectorClock& v)
{
	os << "Vector Clock Of " << v.processId << ":" << endl;
	unsigned int *values = v.getValues();
	for (int i = 0; i < v.totalProcessCount; ++i)
	{
		os << setw(32) << values[i];
		if ( (i +1) % 8 == 0)
			os << endl;
	}

	return os;
}

/*
 * Vector clock increment operations, both post and pre increments
 * ++v  or v++
 * */
VectorClock& VectorClock::operator++() //++v;
{
	v[processId]++;
	return *this;
}
VectorClock VectorClock::operator++(int) //v++;
{
	VectorClock tmp = *this;
	v[processId]++;
	return tmp;
}


bool VectorClock::operator<=(const VectorClock& vRight)
{
	if (operator<(vRight)  || operator==(vRight) )  //kucuk ya da esitse, kucuk esittir
		return true;
	return false;
}

bool VectorClock::operator<(const VectorClock& vRight)
{
	bool strictlySmaller = false;
	unsigned int* vRightValues = vRight.getValues();
	for (int i = 0; i < totalProcessCount; ++i)
	{
		if ( v[i] < vRightValues[i] )		//at least ONE value is stricly smaller
			strictlySmaller = true;
		else if (v[i] > vRightValues[i])  	//if any value of v[i] is greater, than no way v<vRight
			 return false;
	}

	return strictlySmaller; //if there happened strictlySmaller, then smaller operation returns true;
}


extern UINT64 isPairExistsVectorCount ;
extern UINT64 addVectorCount ;


/*
 * isPairExistsVector function is used
 * for Vector Clock History cache improvement
 * if the same comparison is done before, return true
 */
inline  bool VectorClock::isPairExistsVector(UINT64 key, UINT64 InprocessId)
{
	++isPairExistsVectorCount;
	return  maxVCHistoryCount && (/*(InprocessId ==  (UINT64) this->processId)  ||*/   find(vcHistoryVector.begin(), vcHistoryVector.end(), (UINT64)key) != vcHistoryVector.end() );
}

/*
 * addNodeVectorToHistory adds new vector clock id to
 * vector clock history cache
 */

 inline void VectorClock::addNodeVectorToHistory(UINT64 id) //this method adds the node at the end of the chain
{
	 if (maxVCHistoryCount)
		 return;

	 if (alreadyFilled /*&&size >= maxVCHistoryCount*/)
    	vcHistoryVector[id % maxVCHistoryCount] = id;
    else
    {
    	vcHistoryVector.push_back(id);
        if (vcHistoryVector.size() == maxVCHistoryCount  )
        	alreadyFilled = true;

    }
}


extern UINT64 vcHitCount;
extern UINT64 vcNonHitCount;

/*
 *  v1->happensBefore(v2)  returns true
 *  iff v1 happens before v2 :)
 *  Moreover, if vector clock comp. cache is enabled,
 *  the comparion is added to cache of v1
 * */
bool VectorClock::happensBefore(VectorClock* input)
{
	if ( isPairExistsVector((UINT64)input,input->processId))
	{
		++vcHitCount; //just for statistical information
		//return val;
		return true;
	}
	else
		++vcNonHitCount;

	int thisProcessId = this->processId;
	int inputProcessId = input->processId;
	if (
	 ((this->v[thisProcessId]  <= input->v[thisProcessId] )
	 && (this->v[inputProcessId]  < input->v[inputProcessId] )))
	{
		addNodeVectorToHistory((UINT64)input);
		return true;

	}
	return false;
}



/*
 * Basic VC operations != and == is defined
 */
bool VectorClock::operator!=(const VectorClock& vRight)
{
	return !(operator==(vRight));
}
bool VectorClock::operator==(const VectorClock& vRight)
{
	unsigned int *vRightValues = vRight.getValues();
	for (int i = 0; i < totalProcessCount; ++i)
		if (v[i] != vRightValues[i])
			return false;

		return true;
}


/*
 * Create vector clock for each thread
 * */
VectorClock::VectorClock(int processIdIn)
{
	processId = processIdIn;
	alreadyFilled = false;
	vcHistoryVector.reserve(maxVCHistoryCount);
	size_t size_of_bytes = sizeof(int) * totalProcessCount;
	v = (unsigned int*)malloc(size_of_bytes);
	bzero(v, size_of_bytes); //bzero yapmak onemli
	v[processIdIn] = 1;
	//this->history = new VCHistoryNode(UINT64(this));
}


/*
 * Initilaze vector clock from an existing clock
 *  For instance, this kind of constructor is used
 *  when a new segment is created
 * */
VectorClock::VectorClock(VectorClock* inClockPtr, int processIdIn)
{

	this->vcHistoryVector = inClockPtr->vcHistoryVector;
	alreadyFilled = inClockPtr->alreadyFilled;
	processId = processIdIn; //processId su an isimize yaramaz bu sekilde olustrulan vector clock icin, ancak ilerde yarayabilir
	size_t size_of_bytes = sizeof(int) * totalProcessCount;
	v = (unsigned int*)malloc(size_of_bytes);
	bzero(v, size_of_bytes); //bzero yapmak onemli
	for (int i = 0; i < totalProcessCount; ++i )
		v[i] = inClockPtr->v[i];
}

int VectorClock::totalDeletedLockCount = 0;  //just for statistical analysis

/*
 * Vector clock destructor free the integers
 */
VectorClock::~VectorClock()
{
	//cout << "Vector Clock Destructor Called" << endl;
	totalDeletedLockCount++;
	//free(v);
	free(v);
}

void VectorClock::sendEvent()
{
	event();
}

/*
 * Internal VC event
 */
void VectorClock::event()
{
	v[processId]++;
}

/*
 * Get Values for VC comparison
 */
unsigned int* VectorClock::getValues() const
{
	return v;
}

/*
 * v1->receiveAction(v2) ie v1 waits on cond. var. and
 * v2 notifies
 */
void VectorClock::receiveAction(VectorClock* vectorClockReceived)
{
	unsigned int *vOfReceivedClock = vectorClockReceived->getValues();
	for (int i = 0; i < totalProcessCount; ++i)
		v[i] = ( v[i] > vOfReceivedClock[i] ) ? v[i] : vOfReceivedClock[i];

	//event();
}


void VectorClock::toString()
{
	//cout << "segmentId:" << processId << "\t";
	//for (int i=0; i < totalProcessCount; ++i)
		//cout << v[i] << " " ;
	//cout << endl;

}
