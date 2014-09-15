
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
 * ThreadLocalData.cpp
 *
 *  Created on: Jun 11, 2013
 *      Author: onder
 */
#include "ThreadLocalData.h"
extern  TLS_KEY tls_key;

/*
 * currentVectorClock => threads vector clock
 * currentReaderLocksetId, currentWriterLocksetId => lockset ids
 * next* => these values are utilized when for instance a lock,mallock,barrier
 *  function calls are executed, which are explained in pthreadcallinstrumentation.cpp
 *
 * */
ThreadLocalData::ThreadLocalData(THREADID tid)
{
	currentVectorClock = new VectorClock(tid);
	currentReaderLocksetId = 0;
	currentWriterLocksetId = 0;
	nextWaitCondVariableAddr = 0;
	nextBarrierAddr = 0;
	currentSegment = NULL;
	totalSegmentCount = 0;
	InitLock(&threadLock);
	createNewSegment = false;
	isAlive = true;
	nextMallocSize = 0;
	nextReallocAddr = 0;
	nextReallocSize = 0;
	nextSemaphoreAddr = 0;
}


ThreadLocalData* getTLS(THREADID tid)
{
	return static_cast<ThreadLocalData*>(PIN_GetThreadData(tls_key,tid));
}


