
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
 * Lock.h
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */


#ifndef LOCK_H_
#define LOCK_H_

#include "pin.H"
#include "VectorClock.h"

#define INITIAL_LOCK_ADDR -99999


class Lock
{
public:
	Lock(ADDRINT addr);
	Lock();
	VectorClock* lockVectorClock;

	bool type; //READER == 0 , WRITER == 1
	ADDRINT addr;
	bool operator<(const Lock& vRight) const;
	bool operator>(const Lock& vRight) const;
	bool operator==(const Lock& vRight) const;



};


#endif /* LOCK_H_ */
