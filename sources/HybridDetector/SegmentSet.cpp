
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
 * SegmentSet.cpp
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */

#include "SynronizedQueue.h"
#include "SegmentSet.h"
extern PIN_LOCK segmentDeleteLock;
extern vector<Segment*> segmentsToBeDeleted;
extern SynronizedQueue* syncQueue;

extern UINT64 maxSegmentSizeCount;
extern BOOL enableMaxSegmentSize;
using namespace std;


//empty default constructor
SegmentSet::SegmentSet()
{
	//segments.reserve(50);
}

UINT64 addSegmetnCount = 0; //this is for statistical output


/*
 * Simply addes segment to segment set.
 * If the same segment exists, the function discards the add operation
 * */
void SegmentSet::addSegment(Segment* s)
{

	++addSegmetnCount;
	//UINT32 size = 0;
	vector<Segment*>::iterator it = segments.begin() ;
	for(; it != segments.end() ; ++it )
	{
		//++size;
		if (s == (*it) )
			return;
	}

	//GetLock(&(s->segmentLock), s->ownerThreadId);
    s->referenceCount++;
   // return;
    //ReleaseLock(&(s->segmentLock));
    //++size;

    /*if (enableMaxSegmentSize && size >= maxSegmentSizeCount)
    {
    	Segment* segmentToDelete = this->segments[0];
    	this->segments[0] = s;
    	GetLock(&(segmentToDelete->segmentLock), (segmentToDelete)->ownerThreadId);
		(segmentToDelete)->referenceCount--;
		if ((segmentToDelete)->referenceCount == 0 && (segmentToDelete)->segmentIsActive == false)
		{
			GetLock(&segmentDeleteLock, (segmentToDelete)->ownerThreadId);
			segmentsToBeDeleted.push_back((segmentToDelete));
			ReleaseLock(&segmentDeleteLock);
		}

		ReleaseLock(&((segmentToDelete)->segmentLock));
    }
    else*/

    this->segments.push_back(s);
    return;

}

/*
 * Returns true if the input segment in the set
 */
bool SegmentSet::isSegmentInSet(Segment* s)
{

	vector<Segment*>::iterator it = segments.begin() ;
	for(; it != segments.end() ; ++it )
		if (s == (*it) )
			return true;

	return false;
}




/*
 * Pushes all the segments to
 * segments that are going to be deleted
 * */
void SegmentSet::clearAllSegments()
{
	  //cout << "Segments.size:" << segments.size() << endl;
    if (segments.size() == 0)
            return;
    //deleteSegmentsNotInAvaliableQueue();
    vector<Segment*>::iterator it;
    for (it = segments.begin(); it != segments.end();++it)
    {
    		GetLock(&((*it)->segmentLock), (*it)->ownerThreadId);
            (*it)->referenceCount--;
            if ((*it)->referenceCount == 0 && (*it)->segmentIsActive == false)
            {
                    GetLock(&segmentDeleteLock, (*it)->ownerThreadId);
                    //cout << "to be deleted:" << endl;
                   	segmentsToBeDeleted.push_back((*it));
                    ReleaseLock(&segmentDeleteLock);
            }
            ReleaseLock(&((*it)->segmentLock));
    }
    segments.clear();
}


/*
 * This is called to delete segments that must be deleted
 * due to limiting the total number of segments in the execution.
 * If a segment is menat to be discarded from the whole execution,
 * its inAvaliableQueue becomes false, and it is deleted from the segment set
 *
 * */
UINT64 SegmentSet::totalDeletedCount = 0;
void SegmentSet::deleteSegmentsNotInAvaliableQueue()
{
	if (segments.size() == 0 )
		return;

	vector<Segment*>::iterator it = segments.begin() ;
	for(; it != segments.end() ; /*++it*/ )
	{
		if ((*it) == NULL ||  !(*it)->inAvaliableQueue)
		{
			it = segments.erase(it);
			//cout << "deleteSegmentsNotInAvaliableQueue" << endl;
			totalDeletedCount++;
		}
		else
			++it;
	}
}





UINT64 removeHapppendCount = 0;



/*
 * This function is called when segment based hybrid approach
 * maintains concurrent segment sets. (Mostly, WRST or RDST of a variable)
 *
 * If a segment is not referenced by a segment set, it is discarded from the whole execution
 * */
void SegmentSet::removeHappenedBeforeSegments(Segment* s)
{
	if (segments.size() == 0)
		return;

	//deleteSegmentsNotInAvaliableQueue();
	vector<Segment*>::iterator it = segments.begin() ;

	for(; it != segments.end() ; /*++it*/ )
	{
		// Segment* tmpSegment = (*it);
		if ( s == ((*it)))
		{
			return;//++it;
		}
		else if (  ((*it))->inAvaliableQueue == false || ((*it))->clock->happensBefore(s->clock) )
		{
			++removeHapppendCount;
			//cout << (*it)->clock << "  " << s->clock << endl;

			//GetLock(&((*it)->segmentLock), (*it)->ownerThreadId);
			((*it))->referenceCount--;
			//ReleaseLock(&((*it)->segmentLock));
			//bool shouldPush = (  ((*it)->referenceCount == 0) && ( (*it)->segmentIsActive == false) );


			if (  (((*it))->referenceCount == 0) && ( ((*it))->segmentIsActive == false))
			{

				GetLock(&segmentDeleteLock, ((*it))->ownerThreadId);
				segmentsToBeDeleted.push_back(((*it)));
				ReleaseLock(&segmentDeleteLock);
			}

			it = segments.erase(it);
			//it_end = segments.end() ;
		}
		else
			++it;
	}

}


#define VECTOR_CLOCK_COMPARE_HISTORY 250
#define GET_KEY(a, b) ((a << (32)) | b )
#include <unordered_map>
map<UINT64, BOOL> lastSegmentCompareOperations;
vector<UINT64> lastSegmentCompareOperationsV;
UINT64 totalCounter = 0;
/*
 * This function is an experiment.
 * Since the performance decreased, it is NOT used in the execution.
 * In future may be optimized
 * */
void SegmentSet::removeHappenedBeforeSegmentsNew(Segment* s)
{
	if (segments.size() == 0)
		return;

	UINT64 inputSegmentId = s->segmentId;
	//deleteSegmentsNotInAvaliableQueue();
	vector<Segment*>::iterator it = segments.begin() ;

	for(; it != segments.end() ; /*++it*/ )
	{
		++totalCounter;
		// Segment* tmpSegment = (*it);
		if ( s == ((*it)))
		{
			return;//++it;
		}


		UINT64 key = GET_KEY((UINT64)(*it)->segmentId, inputSegmentId);
		key = key + 1;

		//if(lastSegmentCompareOperationsV.size() < VECTOR_CLOCK_COMPARE_HISTORY)
		//{
		//	remove =  lastSegmentCompareOperations[key];
		//}
		//else
		//{
		//	int a = rand() % 250;

		//}

		if (  ((*it))->inAvaliableQueue == false  ||((*it))->clock->happensBefore(s->clock) )
		{
		   // if (lastSegmentCompareOperations.size() < VECTOR_CLOCK_COMPARE_HISTORY)
		   // 	lastSegmentCompareOperationsV.push_back(key);
		    //else
		    //{

		    //}

		    ++removeHapppendCount;
			//cout << (*it)->clock << "  " << s->clock << endl;

			//GetLock(&((*it)->segmentLock), (*it)->ownerThreadId);
			((*it))->referenceCount--;
			//ReleaseLock(&((*it)->segmentLock));
			//bool shouldPush = (  ((*it)->referenceCount == 0) && ( (*it)->segmentIsActive == false) );


			if (  (((*it))->referenceCount == 0) && ( ((*it))->segmentIsActive == false))
			{

				GetLock(&segmentDeleteLock, ((*it))->ownerThreadId);
				segmentsToBeDeleted.push_back(((*it)));
				ReleaseLock(&segmentDeleteLock);
			}

			it = segments.erase(it);
			//it_end = segments.end() ;
		}
		else
			++it;
	}

}
