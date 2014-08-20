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
