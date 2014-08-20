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
