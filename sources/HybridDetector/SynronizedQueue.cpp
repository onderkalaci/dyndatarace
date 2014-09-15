
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
 * SynronizedQueue.cpp
 *
 *  Created on: Aug 13, 2013
 *      Author: onder
 */

#ifndef SYNRONIZEDQUEUE_CPP_
#define SYNRONIZEDQUEUE_CPP_


/*
 * SynronizedQueue data structure is utilzed for
 * limiting the maxium number of segments throughout the execution.
 * The class includes some of queue operations in a synronized manner
 * */

#include "SynronizedQueue.h"
extern vector<Segment*> segmentsToBeDeleted;
extern PIN_LOCK segmentDeleteLock;


SynronizedQueue::SynronizedQueue(UINT32 maxSizeIn) : enabled(maxSizeIn > 0)
{
	maxSize = maxSizeIn ;
	InitLock(&lock);

};

BOOL SynronizedQueue::isEnabled() const
{
	return enabled;
}

void SynronizedQueue::push(Segment* pushedSegment)
{
	if (!enabled)
		return;

	GetLock(&lock,0 );
	m_queue.push(pushedSegment);
	if (m_queue.size() >= maxSize)
	{
		Segment* tmp = NULL;
		tmp = nonSyncFront();
		//cout << "poped data:" << tmp->data << endl;
		//burda deactiva edecegiz
		tmp->inAvaliableQueue = false;
		nonSyncPop();
		//GetLock(&segmentDeleteLock,0);
		//if (tmp->segmentIsActive == false)
		//	segmentsToBeDeleted.push_back(tmp);
		//ReleaseLock(&segmentDeleteLock);
	}
	ReleaseLock(&lock);
};

int SynronizedQueue::getSize()
{
	GetLock(&lock,0 );
	int size = m_queue.size();
	ReleaseLock(&lock);
	return size;
};

Segment* SynronizedQueue::nonSyncFront() const
{
	return m_queue.front();
};

void SynronizedQueue::nonSyncPop()
{
	m_queue.pop();
};



#endif /* SYNRONIZEDQUEUE_CPP_ */
