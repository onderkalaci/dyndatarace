
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
using namespace std;

#define NON_THREAD_VECTOR_CLOCK -1

class VectorClock
{
	private:
		UINT32* v;
	public:
		int processId;
	 	void event();
		VectorClock(int processId);
		~VectorClock();
		VectorClock(VectorClock* inClockPtr, int processId);
		void toString();
		void receiveAction(VectorClock* vectorClockReceived);
		void receiveActionFromSpecialPoint(VectorClock* vectorClockReceived, UINT32 specialPoint);
		UINT32* getValues() const;
		void sendEvent();
		bool happensBefore(VectorClock* input);//OK
		bool happensBeforeSpecial(VectorClock* input, UINT32 processId);
		bool isUniqueValue(int processIdIn);
		bool isEmpty();

	public:
		static int totalProcessCount;
		static int totalDeletedLockCount;
		VectorClock& operator++(); //prefix increment ++vclock
		VectorClock operator++(int x); //postfix increment
		bool operator==(const VectorClock &vRight);
		bool operator!=(const VectorClock &vRight);
		bool operator<(const VectorClock& vRight);
		bool operator<=(const VectorClock& vRight);
		bool areConcurrent(VectorClock* vectorClockReceived, ADDRINT processId);
		friend ostream& operator<<(ostream& os, const VectorClock &v);
};


#endif /* VECTORCLOCK_H_ */
