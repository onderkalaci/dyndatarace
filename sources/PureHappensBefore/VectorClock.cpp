/*
 * VectorClock.cpp
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */
#include "VectorClock.h"
#include "definitions.h"




/**
 * Vector Clock Implementation
 *
 * **/

//bunu degistirdikten sonra olusturacagin VectorClock'lar, o kadar
int VectorClock::totalProcessCount = 48;
ostream& operator<<(ostream& os, const VectorClock& v)
{
	os << "Vector Clock Of " << v.processId << ":" << endl;
	UINT32 *values = v.getValues();
	for (int i = 0; i < v.totalProcessCount; ++i)
	{
		os << setw(33) << values[i]; //output guzel gozuksun, her satirda 8 deger olsun
		if ( (i +1) % 4 == 0)
			os << endl;
	}

	return os;
}

VectorClock& VectorClock::operator++() //++v;
{
	//cout << "incermented:" << processId << endl;
	v[processId]++;
	return *this;
}
VectorClock VectorClock::operator++(int) //postincrement  v++;
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
	UINT32* vRightValues = vRight.getValues();
	for (int i = 0; i < totalProcessCount; ++i)
	{
		if ( v[i] < vRightValues[i] )		//at least ONE value is stricly smaller
			strictlySmaller = true;
		else if (v[i] > vRightValues[i])  	//if any value of v[i] is greater, than no way v<vRight
			strictlySmaller = false;//return false;
	}

	return strictlySmaller; //if there happened strictlySmaller, then smaller operation returns true;
}

#include "pin.H"
//extern UINT64 segmentComparisonCount;
//extern UINT64 memoryAccessCount;
//extern PIN_LOCK segmentCompareLock;
// clock_1.happensBefore(clock_2)  returns true, iff clock_1 < clock_2
bool VectorClock::happensBefore(VectorClock* input)
{
		//GetLock(&segmentCompareLock, 0);
		//segmentComparisonCount = segmentComparisonCount + 1;
		//ReleaseLock(&segmentCompareLock);
	    bool happensBeforeValue = (*this < *input);
		return  happensBeforeValue;
}

bool VectorClock::isUniqueValue(int processIdIn)
{
	bool isUnique = true;
	for (int i = 0; i < totalProcessCount ; ++i)
	{
		if (v[i] > 0 && i != processIdIn)
		{
			isUnique = false;;
			break;
		}
	}

	return isUnique;
}
bool VectorClock::isEmpty()
{
	for (int i = 0; i < totalProcessCount ; ++i)
	{
		if (v[i] != 0)
			return false;
	}

	return true;
}


bool VectorClock::happensBeforeSpecial(VectorClock* input, UINT32 processId)
{
	//GetLock(&segmentCompareLock, 0);
	//segmentComparisonCount = segmentComparisonCount + 1;
	//ReleaseLock(&segmentCompareLock);
	UINT32* vRightValues = input->getValues();
	for (int i = 0; i < totalProcessCount; ++i)
	{
		if ((uint32_t)i == processId)
			continue;

		if ( v[i]  > 0 &&  v[i] >= vRightValues[i] )		//at least ONE value is stricly smaller
			return true;
	}

	return false; //if there happened strictlySmaller, then smaller operation returns true;
}

bool VectorClock::areConcurrent(VectorClock* input, ADDRINT processId)
{
	//if  (this->happensBeforeSpecial(input, processId))
	if  (!this->happensBefore(input) &&  !input->happensBefore(this))
		return true;

	return false;
}

bool VectorClock::operator!=(const VectorClock& vRight)
{
	return !(operator==(vRight));
}
bool VectorClock::operator==(const VectorClock& vRight)
{
	UINT32 *vRightValues = vRight.getValues();
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

	size_t size_of_bytes = sizeof(int) * totalProcessCount;
	v = (UINT32*)malloc(size_of_bytes);
	bzero(v, size_of_bytes); //bzero yapmak onemli

	if (processId != NON_THREAD_VECTOR_CLOCK)
		v[processIdIn] = 1;
}


/*
 * Initilaze vector clock from an existing clock
 * Here, we do not need the processId, because this kind of vector clock is goin
 * */
VectorClock::VectorClock(VectorClock* inClockPtr, int processIdIn)
{
	processId = processIdIn; //processId su an isimize yaramaz bu sekilde olustrulan vector clock icin, ancak ilerde yarayabilir
	size_t size_of_bytes = sizeof(int) * totalProcessCount;
	v = (UINT32*)malloc(size_of_bytes);
	bzero(v, size_of_bytes); //bzero yapmak onemli
	for (int i = 0; i < totalProcessCount; ++i )
		v[i] = inClockPtr->v[i];
}

int VectorClock::totalDeletedLockCount = 0;
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

void VectorClock::event()
{
	v[processId]++;
}

UINT32* VectorClock::getValues() const
{
	return v;
}


void VectorClock::receiveAction(VectorClock* vectorClockReceived)
{
	UINT32 *vOfReceivedClock = vectorClockReceived->getValues();
	for (int i = 0; i < totalProcessCount; ++i)
		v[i] = ( v[i] > vOfReceivedClock[i] ) ? v[i] : vOfReceivedClock[i];

	//cout << "receive actiton from " << vectorClockReceived->processId << "  to:" << processId << endl;
	//event();
}

void VectorClock::receiveActionFromSpecialPoint(VectorClock* vectorClockReceived, UINT32 specialPoint)
{
	UINT32 *vOfReceivedClock = vectorClockReceived->getValues();
	v[specialPoint] = vOfReceivedClock[specialPoint];

	//event();
}


void VectorClock::toString()
{
	//cout << "segmentId:" << processId << "\t";
	//for (int i=0; i < totalProcessCount; ++i)
		//cout << v[i] << " " ;
	//cout << endl;

}
