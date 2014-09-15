
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
#include "VectorClock.h"

class MemoryAddr
{
public:
	MemoryAddr(ADDRINT addr);
	MemoryAddr(ADDRINT addr, BOOL isMoved);
	ADDRINT address;
	VectorClock* writerVectorClock;
	VectorClock* readerVectorClock;
};




#endif /* MEMORYADDR_H_ */
