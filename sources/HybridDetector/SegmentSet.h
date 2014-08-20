/*
 * SegmentSet.h
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */

#ifndef SEGMENTSET_H_
#define SEGMENTSET_H_

#include "Segment.h"
#include <deque>
class SegmentSet
{
public:
	SegmentSet(); //OK
	vector<Segment*>segments;

public:
	void addSegment(Segment* s);//OK
	void removeHappenedBeforeSegments(Segment* s); //OK
	void removeHappenedBeforeSegmentsNew(Segment* s); //experimantal
	void clearAllSegments(); //reference count'u da azaltarak
	bool isSegmentInSet(Segment* s);
	void deleteSegmentsNotInAvaliableQueue();
	static UINT64 totalDeletedCount;
};


#endif /* SEGMENTSET_H_ */
