/*
 * AllLockSets.h
 *
 *  Created on: Jun 11, 2013
 *      Author: onder
 */

#ifndef ALLLOCKSETS_H_
#define ALLLOCKSETS_H_

#include "definitions.h"
#include "LockSet.h"
#define MAX_LOCK_SET_COUNT 100000
#define NO_INTERSECTION_CACHE_FOUND 9999999

class AllLockSets
{
public:
	AllLockSets();//default constructor
	LockSet* allAvaliableLockSets[MAX_LOCK_SET_COUNT];
	unordered_map<ADDRINT, vector<ADDRINT> > lockAddrMap;
	ADDRINT getLockSet(UINT32 currentLockSetID, ADDRINT newLockAddr, BOOL added); //bu fonksyona eski currentlocksetid ve (ekledigi/cikardigi lock'un adresini verecek, biz de ona yeni adresini verecegiz)
	string toString();
	void addTolockAddrMap(ADDRINT newLockid,LockSet* tmpLockSet);
	BOOL isIntersectionEmpty(ADDRINT locksetid1, ADDRINT locksetid2);
	inline UINT32 isIntersectionEmptyCache(UINT64 key) ;
	inline UINT64 calculateKey(const UINT32 &l1, const UINT32 &l2) const;
private:
	PIN_LOCK allLockSetsLock;
	map<UINT64, UINT32> isIntersectionEmptyCacheData;
	//UINT64 values[3*MAX_LOCK_SET_COUNT];
};



#endif /* ALLLOCKSETS_H_ */
