
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
 * ThreadLocalData.h
 *
 *  Created on: Jun 11, 2013
 *      Author: onder
 */

#ifndef THREADLOCALDATA_H_
#define THREADLOCALDATA_H_

#include "definitions.h"
#include "VectorClock.h"





class ThreadLocalData
{
public:
	ThreadLocalData(THREADID tid);


	VectorClock* currentVectorClock;
	ADDRINT nextLockAddr;
	ADDRINT nextWaitCondVariableAddr;
	ADDRINT nextWaitCondVariableLockAddr;
	ADDRINT nextBarrierAddr;
	ADDRINT nextSemaphoreAddr;

	PIN_LOCK threadLock;
	BOOL isAlive;
	ADDRINT nextMallocSize;
	ADDRINT nextReallocAddr;
	ADDRINT nextReallocSize;

};

ThreadLocalData* getTLS(THREADID tid);


#endif /* THREADLOCALDATA_H_ */
