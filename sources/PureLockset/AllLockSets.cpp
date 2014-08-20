/*
 * AllLockSets.cpp
 *
 *  Created on: Jun 11, 2013
 *      Author: onder
 */

#include "AllLockSets.h"
//default constructor, sadece
AllLockSets::AllLockSets()
{
	InitLock(&allLockSetsLock);
	LockSet* emptyLockSet = new LockSet();
	//allAvaliableLockSets.reserve(100000);
	bzero(allAvaliableLockSets,sizeof(LockSet*)*MAX_LOCK_SET_COUNT);
	allAvaliableLockSets[0] = emptyLockSet; // 0(sifir) id'li lockset'i ekle, empty lockset her zaman avaliable olacak
}

string AllLockSets::toString()
{
	cout << "AllLockSets::toString()" << endl;
	ostringstream os;

	os << "AllLockSets to String:" << endl;
	for (int i=0; i < MAX_LOCK_SET_COUNT; ++i )
	{
		LockSet* tmpLockset = allAvaliableLockSets[i];
		if (tmpLockset == NULL)
			break;
		os << "id:" << i << endl;
		for (UINT32 i = 0; i < tmpLockset->locks.size(); ++i)
			os << tmpLockset->locks.at(i)->addr << " - ";

		os << "size(" << tmpLockset->locks.size() << ")";

		os << endl;
	}

	return os.str();
}

 int opCount = 0;
 int nopCount = 0;
 int totalCount = 0;
 int greaterCount = 0;
 int smallerCount = 0;

int loopCount = 0;

void AllLockSets::addTolockAddrMap(ADDRINT newLockid, LockSet* tmpLockSet)
{

	bool  alreadAdded = false;
	vector<ADDRINT>tmpLockIds;
	int size = tmpLockSet->locks.size();
	for (int i=0; i < size; ++i)
	{
	//	cout << "Update lock " << tmpLockSet->locks.at(i)->addr << "'s  lockSetIds" << endl;
		tmpLockIds = lockAddrMap[tmpLockSet->locks.at(i)->addr];
		int tmpSize = tmpLockIds.size();
		for (int j = 0; j < tmpSize; ++j)
			if (tmpLockIds.at(j) == newLockid)
				alreadAdded = true;

		if (!alreadAdded)
		{
			tmpLockIds.push_back(newLockid);
			lockAddrMap[tmpLockSet->locks.at(i)->addr] = tmpLockIds;
		//	cout << "Lock " << tmpLockSet->locks.at(i)->addr << "'  lockIdSet is updated, newLockId is added:" << newLockid << "  list size:" << lockAddrMap[tmpLockSet->locks.at(i)->addr].size() << endl;

		}
	}
}

int cacheHit = 0;
int nonCacheHit = 0;
UINT32 AllLockSets::findIntersection(ADDRINT locksetid1, ADDRINT locksetid2)
{
	if (locksetid1 == locksetid2)
		return locksetid1;
	UINT64 key = this->calculateKey(locksetid1, locksetid2) ;

	UINT32 cacheVal = findIntersectionCache(key);
	if (cacheVal != NO_INTERSECTION_CACHE_FOUND )
	{
		++cacheHit;
		return cacheVal;
	}
	++nonCacheHit;


	UINT32 returnLockSetId = 0;

	GetLock(&allLockSetsLock, PIN_ThreadId());
	LockSet* ls1 = allAvaliableLockSets[locksetid1];
	LockSet* ls2 = allAvaliableLockSets[locksetid2];


	vector<Lock*> intersectedLocks = ls1->findIntersetcion(ls2);
	int intersectedLocksSize = intersectedLocks.size();

	if (intersectedLocksSize == 0)
	{
			this->findIntersectionCacheData[key] = returnLockSetId;
			ReleaseLock(&allLockSetsLock);
			return returnLockSetId;
	}

	LockSet* tmpLockSet = new LockSet();
	for (int i = 0; i< intersectedLocksSize; ++i)
		tmpLockSet->locks.push_back(intersectedLocks.at(i));


	vector<ADDRINT> lockSetIds = lockAddrMap[intersectedLocks.at(0)->addr];

	bool thisLockSetIsNew = true;
	int size = lockSetIds.size();
	for (int i=0; i < size; ++i)
	{
		if ((allAvaliableLockSets[lockSetIds.at(i)])->areEqual(tmpLockSet))
		{
			thisLockSetIsNew = false;
			returnLockSetId = lockSetIds.at(i);
			break;
		}
	}

	if (thisLockSetIsNew == true) //yani bu lockset yok ise
	{
		LockSet* local_tmpLockset = NULL;
		for (int i= 0 ; i < MAX_LOCK_SET_COUNT; ++i )
		{
			local_tmpLockset = allAvaliableLockSets[i];
			if (local_tmpLockset == NULL)
			{
				returnLockSetId = i;
				break;
			}
		}
		allAvaliableLockSets[returnLockSetId] = tmpLockSet;
		addTolockAddrMap(returnLockSetId, tmpLockSet);
	}
	else if (thisLockSetIsNew == false)
		delete tmpLockSet;


	this->findIntersectionCacheData[key] = returnLockSetId;

	ReleaseLock(&allLockSetsLock);

	return returnLockSetId;
}

int cacheHit_2 = 0;
int nonCacheHit_2 = 0;
BOOL AllLockSets::isIntersectionEmpty(ADDRINT locksetid1, ADDRINT locksetid2)
{
	if (locksetid1 !=0 && locksetid1 == locksetid2)
		return false;

	UINT32 cache = isIntersectionEmptyCache(locksetid1, locksetid2);

	if (cache != NO_INTERSECTION_CACHE_FOUND)
	{
		++cacheHit_2;
		return cache;
	}
	++nonCacheHit_2;


	LockSet* lockSet_1 = allAvaliableLockSets[locksetid1];
	LockSet* lockset_2 = allAvaliableLockSets[locksetid2];

	BOOL retVal = lockSet_1->isIntersectionEmpty(lockset_2);

	UINT64 key = calculateKey(locksetid1, locksetid2);
	this->isIntersectionEmptyCacheData[key] = retVal;

	return retVal;

}

UINT32 AllLockSets::isIntersectionEmptyCache(const ADDRINT &locksetid1,const ADDRINT& locksetid2)
{
	//return NO_INTERSECTION_CACHE_FOUND;
	UINT64 key = calculateKey(locksetid1, locksetid2);

	if (this->isIntersectionEmptyCacheData.count(key) == 0)
		return NO_INTERSECTION_CACHE_FOUND;

	return this->isIntersectionEmptyCacheData[key];
}

UINT64 AllLockSets::calculateKey(const UINT32 &locksetid1, const UINT32& locksetid2) const
{
	if (locksetid1 > locksetid2)
		return locksetid2 * 100000 + locksetid1;

	return locksetid1 * 100000 + locksetid2;
}

UINT32 AllLockSets::findIntersectionCache(ADDRINT key)
{
	//return NO_INTERSECTION_CACHE_FOUND;


	if (this->findIntersectionCacheData.count(key) == 0)
		return NO_INTERSECTION_CACHE_FOUND;


	return this->findIntersectionCacheData[key];

}

ADDRINT AllLockSets::getLockSet(UINT32 currentLockSetID, ADDRINT newLockAddr, BOOL added)
{


	ADDRINT newLockid = MAX_LOCK_SET_COUNT;
	LockSet *tmpLockSet = new LockSet();
	int tmpNewLockId = 0;
	LockSet* local_tmpLockset = NULL;
	Lock* tmpLock = new Lock(newLockAddr);
	bool deleteTmpLocks = false;

	GetLock(&allLockSetsLock, PIN_ThreadId());
	//cout << "getLockSet, currentLockSetID:" << currentLockSetID << "  newAddr:" << newLockAddr << " added:" << added << endl;

	LockSet* currentLockSet = allAvaliableLockSets[currentLockSetID];
	//cout << "Current LockSet:" << endl << currentLockSet->toString() << endl;
	++totalCount;

	if (currentLockSet == NULL)
	{
		cout << "no such id:" << currentLockSetID << "   added:" << added << endl;
		throw "No Such Lock Id";
		ReleaseLock(&allLockSetsLock);
		return 0;
	}
	*tmpLockSet = (*currentLockSet);
	if (added) //if newLockAddr lock is added
		tmpLockSet->addLock(tmpLock);
	else //newLockAddr is removed
		tmpLockSet->removeLock(tmpLock);


	//cout << "tmpLockSet:" << endl << tmpLockSet->toString() << endl;

	if (tmpLockSet->locks.size() == 0)
	{
		newLockid = 0;
		deleteTmpLocks = true;
	}
	else
	{

		vector<ADDRINT> lockSetIds = lockAddrMap[tmpLockSet->locks.at(0)->addr];
		int size = lockSetIds.size();
		for (int i=0; i < size; ++i)
		{
			if ((allAvaliableLockSets[lockSetIds.at(i)])->areEqual(tmpLockSet))
			{

				++smallerCount;
				++opCount;
				newLockid = lockSetIds.at(i) ;
				deleteTmpLocks = true;
				break;

			}
		//	else if (allAvaliableLockSets[lockSetIds.at(i)]->locks.size() == tmpLockSet->locks.size())
			//	cout << "---------------- BelowSetsAreNOTEQUAL:" << endl << allAvaliableLockSets[lockSetIds.at(i)]->toString() << endl << tmpLockSet->toString() << endl;
		}
	}


	if (newLockid == MAX_LOCK_SET_COUNT) //optimizasyon ise yaramadiysa
	{
		++nopCount;
		for (int i= 0 ; i < MAX_LOCK_SET_COUNT; ++i )
		{
			local_tmpLockset = allAvaliableLockSets[i];
			if (local_tmpLockset == NULL)
			{
				tmpNewLockId = i;
			//	cout << "New id generated:" << i << endl;
			//	cout << "LockSet i:" << i << endl << tmpLockSet->toString() << endl;
				break;
			}
		}
		++greaterCount;
		newLockid = tmpNewLockId;//allAvaliableLockSets.size();
		allAvaliableLockSets[newLockid] = tmpLockSet;
		addTolockAddrMap(newLockid, tmpLockSet);
	}

	ReleaseLock(&allLockSetsLock);

	if (deleteTmpLocks)
	{
		delete tmpLockSet;
		delete tmpLock;
	}

	return newLockid;
}
