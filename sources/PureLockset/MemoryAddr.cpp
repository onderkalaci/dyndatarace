
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


#include "MemoryAddr.h"



MemoryAddr::MemoryAddr(ADDRINT addr, ADDRINT writerLocksetIn, ADDRINT readerLocksetIn, ADDRINT firstOwnerThread, ADDRINT initialState):
address(addr), writerLocksetId(writerLocksetIn), readerLocksetId(readerLocksetIn), ownerThreadId(firstOwnerThread), currentState(initialState),mallocAddress(addr),  isMallocedTested(false)
{
	/*
	this->address = addr;
	this->writerLocksetId = writerLocksetIn;
	this->readerLocksetId = readerLocksetIn;
	this->currentState = initialState;
	this->ownerThreadId = firstOwnerThread;
	this->mallocAddress = addr;
	this->isMallocedTested = false;
	*/
}


VOID MemoryAddr::updateState(THREADID tid, BOOL isWrite, UINT32 writerLocksetIdIn, UINT32 readerLocksetIdIn)
{

	//if (this->currentState == MEM_STATE_SHARED_MODIFIED)
		//return; //do nothing
	//else
	if (this->currentState == MEM_STATE_READ_SHARED)
	{
		if (isWrite)
			this->currentState = MEM_STATE_SHARED_MODIFIED;
	}
	else if(this->currentState == MEM_STATE_EXCLUSIVE )
	{
		if (this->ownerThreadId != tid)
		{
			if (isWrite)
				this->currentState = MEM_STATE_SHARED_MODIFIED;
			else
				this->currentState = MEM_STATE_READ_SHARED;
		}
	}
	else if(this->currentState == MEM_STATE_VIRGIN)
	{
		if (isWrite)
		{
			this->currentState = MEM_STATE_EXCLUSIVE;
			this->ownerThreadId = tid;
			this->writerLocksetId = writerLocksetIdIn;
			this->readerLocksetId = readerLocksetIdIn;
		}
	}
}

