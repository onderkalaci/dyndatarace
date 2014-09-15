
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



MemoryAddr::MemoryAddr(ADDRINT addr)
{
	this->address = addr;
	this->writerVectorClock = new VectorClock(NON_THREAD_VECTOR_CLOCK);
	this->readerVectorClock = new VectorClock(NON_THREAD_VECTOR_CLOCK);
}


MemoryAddr::MemoryAddr(ADDRINT addr, BOOL isMoved)
{
	this->address = addr;

	if (!isMoved)
	{
		this->writerVectorClock = new VectorClock(NON_THREAD_VECTOR_CLOCK);
		this->readerVectorClock = new VectorClock(NON_THREAD_VECTOR_CLOCK);
	}
	else
	{
		this->writerVectorClock = NULL;
		this->readerVectorClock = NULL;
	}
}


