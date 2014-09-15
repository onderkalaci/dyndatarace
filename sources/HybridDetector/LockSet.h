
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
