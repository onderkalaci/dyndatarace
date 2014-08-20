/*
 * Lock.h
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */


#ifndef LOCK_H_
#define LOCK_H_

#include "pin.H"

#define INITIAL_LOCK_ADDR -99999


class Lock
{
public:
	Lock(ADDRINT addr);
	Lock();

	bool type; //READER == 0 , WRITER == 1
	ADDRINT addr;
	bool operator<(const Lock& vRight) const;
	bool operator>(const Lock& vRight) const;
	bool operator==(const Lock& vRight) const;



};


#endif /* LOCK_H_ */
