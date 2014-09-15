
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
