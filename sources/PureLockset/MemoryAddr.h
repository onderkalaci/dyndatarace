
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
 * MemoryAddr.h
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */

#ifndef MEMORYADDR_H_
#define MEMORYADDR_H_

#include "definitions.h"

#define MEM_STATE_VIRGIN 0
#define MEM_STATE_EXCLUSIVE 1
#define MEM_STATE_READ_SHARED 2
#define MEM_STATE_SHARED_MODIFIED 3




class MemoryAddr
{
public:
	MemoryAddr(ADDRINT addr, ADDRINT writerLocksetIn, ADDRINT readerLocksetIn, ADDRINT firstOwnerThread, ADDRINT initialState);

	ADDRINT address;
	UINT32 writerLocksetId;
	UINT32 readerLocksetId;
	THREADID ownerThreadId;
	UINT32 currentState;
	ADDRINT mallocAddress;
	BOOL isMallocedTested;
	void updateState(THREADID tid, BOOL isWrite, UINT32 writerLocksetIdIn, UINT32 readerLocksetIdIn);
};




#endif /* MEMORYADDR_H_ */
