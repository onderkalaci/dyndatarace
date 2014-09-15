
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
