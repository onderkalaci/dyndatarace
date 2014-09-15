
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
 * SynronizedQueue.h
 *
 *  Created on: Aug 13, 2013
 *      Author: onder
 */

#ifndef SYNRONIZEDQUEUE_H_
#define SYNRONIZEDQUEUE_H_
#include "definitions.h"
#include <queue>
#include "Segment.h"
class SynronizedQueue
{

	public:
		SynronizedQueue(UINT32 maxSizeIn = 1000000);
		void push(Segment* pushedSegment);
		int getSize();
	//private:
		Segment* nonSyncFront() const;
		void nonSyncPop();
		BOOL isEnabled() const;


	private:
		queue<Segment*> m_queue;
		PIN_LOCK lock;
		UINT32 maxSize;
		const BOOL enabled;




};


#endif /* SYNRONIZEDQUEUE_H_ */
