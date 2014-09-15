
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
 * Lock.cpp
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */


#include "Lock.h"



/***
 * Lock Implementation
 *
 * **/


Lock::Lock()
{
		this->addr = INITIAL_LOCK_ADDR;
		this->type = -1;

}

Lock::Lock(ADDRINT addrIn)
{
		this->addr = addrIn;
		this->type = -1;

}

bool Lock::operator<(const Lock& vRight) const
{
	if (this->addr < vRight.addr)
		return true;
	return false;
}

bool Lock::operator>(const Lock& vRight) const
{
	if (this->addr > vRight.addr)
		return true;
	return false;
}
bool Lock::operator==(const Lock& vRight) const
{
	if (this->addr == vRight.addr)
		return true;
	return false;
}
