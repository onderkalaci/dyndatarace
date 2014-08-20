/*
 * LockSet.h
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */

#ifndef LOCKSET_H_
#define LOCKSET_H_
#include "Lock.h"
#include <sstream>
#include <algorithm>
using namespace std;

/*
 * Lock Pointer'larini Compare etmek icin
 * */
class ComparatorLockAddr {
  public:
    bool operator() (Lock *l1, Lock *l2) {
      return (*l1).addr < (*l2).addr;
    }
};


class LockSet
{
public:
	LockSet();
	vector<Lock*> locks;
	void addLock(Lock *l);
	void removeLock(Lock *l);
	bool isIntersectionEmpty(LockSet* vRight);
	bool areEqual(LockSet* vRight);
	string toString();
	bool operator==(const LockSet &vRight);
	bool operator!=(const LockSet &vRight);
	LockSet& operator=(LockSet& rhs);
	bool isSorted();
	bool sorted;

};


#endif /* LOCKSET_H_ */
