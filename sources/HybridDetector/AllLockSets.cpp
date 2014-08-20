/*
 * AllLockSets.cpp
 *
 *  Created on: Jun 11, 2013
 *      Author: onder
 */

#include "AllLockSets.h"
//default constructor, sadece
/*
 *  See handling of locksets in the thesis
 *  All locksets are handled by this class
 *
 */

AllLockSets::AllLockSets()
{
	InitLock(&allLockSetsLock);
	LockSet* emptyLockSet = new LockSet();
	//allAvaliableLockSets.reserve(100000);
	bzero(allAvaliableLockSets,sizeof(LockSet*)*MAX_LOCK_SET_COUNT);
	allAvaliableLockSets[0] = emptyLockSet; // 0(sifir) id'li lockset'i ekle, empty lockset her zaman avaliable olacak
//	this->isIntersectionEmptyCacheData.reserve(MAX_LOCK_SET_COUNT);
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
/*
 * after a new lockset is generated,
 * add it to lockset map
 */
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

//http://en.wikipedia.org/wiki/pairing_function

#define min(X, Y)  ((X) < (Y) ? (X) : (Y))
#define max(X, Y)  ((X) >= (Y) ? (X) : (Y))
#define calculateKeyPair (X, Y)  (( (X + Y) / 2 ) ) * (X + Y + 1) + max(X, Y)

/*
 * For caching purposes generate a key from two locks
 */

UINT64 AllLockSets::calculateKey(const UINT32 &locksetid1, const UINT32& locksetid2) const
{
	if (locksetid1 >= locksetid2)
		return locksetid2 * 100000 + locksetid1;

	return locksetid1 * 100000 + locksetid2;
}


/*
 * for performance purposes, chcek intersection of small cache before
 * actually taking intersection
 */
UINT32 AllLockSets::isIntersectionEmptyCache(UINT64 key)
{

	if (this->isIntersectionEmptyCacheData.count(key) == 0)
		return NO_INTERSECTION_CACHE_FOUND;

	return this->isIntersectionEmptyCacheData[key];
}

UINT32 cacheHit = 0;
int nonCacheHit = 0;
/*
 * check if two locksets has common lock or not
 * if there exists common lock return true
 * else false
 */
BOOL AllLockSets::isIntersectionEmpty(ADDRINT locksetid1, ADDRINT locksetid2)
{
//	return false;
	if (locksetid1 !=0 && locksetid1 == locksetid2)
		return false;
	UINT64 key = calculateKey(locksetid1, locksetid2);

	UINT32 cache = isIntersectionEmptyCache(key);

	if (cache != NO_INTERSECTION_CACHE_FOUND)
	{
		//++cacheHit;
		return cache;
	}
	//++nonCacheHit;


	LockSet* lockSet_1 = allAvaliableLockSets[locksetid1];
	LockSet* lockset_2 = allAvaliableLockSets[locksetid2];

	BOOL retVal = lockSet_1->isIntersectionEmpty(lockset_2);
	this->isIntersectionEmptyCacheData[key] = retVal;

	return retVal;

}


/**
 * When a thread acquires a lock, the previous lock id and new
 * lock address is given to this function
 * if the new lockset formed by the id+prev lockset is already exist return id
 * else add new lockset and its id is returned
 *
 */
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

				//++smallerCount;
				//++opCount;
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
		//++greaterCount;
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
