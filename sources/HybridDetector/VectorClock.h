/*
 * VectorClock.h
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */

#ifndef VECTORCLOCK_H_
#define VECTORCLOCK_H_
#include "definitions.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <stdlib.h>
#include <strings.h> //added for  bzero function
#include <algorithm>
using namespace::std;

class VectorClock
{
	//private:
public:
		unsigned int* v;
		//VCHistoryNode* history;
		vector<UINT64>vcHistoryVector;
		//UINT64 vcHistory;

	public:
		int processId;
		bool alreadyFilled;
	 	void event();
		VectorClock(int processId);
		~VectorClock();
		VectorClock(VectorClock* inClockPtr, int processId);
		void toString();
		void receiveAction(VectorClock* vectorClockReceived);
		unsigned int* getValues() const;
		void sendEvent();
		bool happensBefore(VectorClock* input);//OK
		bool happensBeforeNew(VectorClock* input);//OK
	    bool isPairExistsVector(UINT64 id,UINT64);
	    void addNodeVectorToHistory(UINT64 id);



	public:
		static int totalProcessCount;
		static int totalDeletedLockCount;
		VectorClock& operator++(); //prefix increment ++vclock
		VectorClock operator++(int x); //postfix increment
		bool operator==(const VectorClock &vRight);
		bool operator!=(const VectorClock &vRight);
		bool operator<(const VectorClock& vRight);
		bool operator<=(const VectorClock& vRight);
		friend ostream& operator<<(ostream& os, const VectorClock &v);
};


#endif /* VECTORCLOCK_H_ */
