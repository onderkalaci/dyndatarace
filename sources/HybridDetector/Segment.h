
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
 * Segment.h
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */

#ifndef SEGMENT_H_
#define SEGMENT_H_

#include "LockSet.h"
#include "VectorClock.h"
#include <unordered_map>
#include <map>

class Segment
{

public:
	ADDRINT insPtr;
	VectorClock* clock;//segment'in clock'u sabit olacak, pointer ama new olusturacagiz
	THREADID ownerThreadId;
	//LockSet writerLockSet, readerLockSet;//thread Segment'e geldiginde, thread'in O ANKI lockset'leri buraya koyulacak.

	UINT32 writerLockSetId, readerLockSetId;
	Segment();
	~Segment();
	Segment(UINT32 threadsWriterLockSetId, UINT32 threadsReaderLockSetId, VectorClock* currentThreadsClock, THREADID tid, ADDRINT insPtrIn);
	void setWriterLockSetId(UINT32 threadsWriterLockSetId);
	void setReaderLockSetId(UINT32 threadsReaderLockSetId);
	Segment& operator=(Segment* s);
	Segment(const Segment& s);
    int segmentId;
	string file;
	int line;
	//map<ADDRINT, ADDRINT> writeMemoryAddresses;
	//map<ADDRINT, ADDRINT> readMemoryAddresses;
	PIN_LOCK segmentLock;
	BOOL segmentIsActive;
	BOOL segmentInfoFilled;
	string imageName;
	void deactivateSegment();
	static int totalSegmentCount;
	static int deletedSegmentCount;
	UINT32 referenceCount;
	BOOL inAvaliableQueue;


	void fillLineAndFileInfo(ADDRINT insPtrIn, const char* imageName);
	Segment(UINT32 threadsWriterLockSetId, UINT32 threadsReaderLockSetId, VectorClock* currentThreadsClock,
			THREADID threadId, string fileNameIn, UINT32 lineIn, ADDRINT insPtr);


};

#endif /* SEGMENT_H_ */
