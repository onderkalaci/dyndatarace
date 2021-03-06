
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
 * MemoryAddr.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */


/*
 * This class is the shadow memory for memory addresses
 *
 */

#include "MemoryAddr.h"
#include "SynronizedQueue.h"

extern SynronizedQueue* syncQueue;
extern   bool syncQueueEnabled;


/**
 * Each Memory address class includes memory address +
 * whether the address is in malloced area or not +
 * if it is already raced or not
 */
MemoryAddr::MemoryAddr(ADDRINT addr)
{
	this->address = addr;
	this->mallocAddress = addr;
	this->isMallocedTested = false;
	this->alreadyRaced = false;
}

BOOL MemoryAddr::isSegmentInReaderSegmentSet(Segment* s)
{
	//cout << "entered" << endl;
	return readerSegmentSet.isSegmentInSet(s);

}

BOOL MemoryAddr::isSegmentInWriterSegmentSet(Segment* s)
{
	return writerSegmentSet.isSegmentInSet(s);
}

VOID MemoryAddr::deleteSegmentsNotInAvaliableQueue()
{
	if (syncQueue->isEnabled()/*syncQueue->enabled*/)
	{
		writerSegmentSet.deleteSegmentsNotInAvaliableQueue();
		readerSegmentSet.deleteSegmentsNotInAvaliableQueue();
	}
}


