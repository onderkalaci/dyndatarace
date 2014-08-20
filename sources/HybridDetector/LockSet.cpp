/*
 * LockSet.cpp
 *
 *  Created on: Apr 19, 2013
 *      Author: onder
 */

#include "LockSet.h"
extern ostringstream output_stream;
#include <iostream>
#include <algorithm>
using namespace std;

/*
 * This locset implementation is basically a set
 * such that all elements are ordered for performance reasons
 *
 */

LockSet::LockSet()
{
    sorted = true;
}


/**
 * Add lock if it is not already in the set
 */
void LockSet::addLock(Lock *l)
{
	//do not add same lock

	for (UINT32 i=0; i < locks.size() ;++i)
		if (locks.at(i)->addr == l->addr)
			return;
	this->sorted = false;
	locks.push_back(l);
}

bool myPredicate(Lock* l1, Lock* l2)
{
	 return (l1->addr == l2->addr);
}
/*
 * if locksets are equal return true
 */
bool LockSet::operator==(const LockSet& ls2)
{

	int size1 = locks.size();
	int size2 = ls2.locks.size();

	if (size1 != size2 )
		return false;

	size1 = size1 - 1;
	size2 = size2 - 1;

	//cout << "size1:" << size1 << "  size2:" << size2 << endl;
	for (INT32 i = size1, j = size2; i >= 0 && j >= 0; --i, --j)
	{

		//cout << "locks.at(i)->addr:" << locks.at(i)->addr << "  ls2.locks.at(j)->addr:" << ls2.locks.at(j)->addr << endl;
		if (locks.at(i)->addr != ls2.locks.at(j)->addr)
		{
			//if (equal(locks.begin(),	locks.end(),  ls2.locks.begin(), myPredicate )) trueCount++; else falseCount++;
			return false;
		}
	}
	//if (equal(locks.begin(),	locks.end(),  ls2.locks.begin(), myPredicate )) trueCount++; else falseCount++;
	return true;

}

/*
 * if locksets are not equal return true
 */
bool LockSet::operator!=(const LockSet& ls2)
{
	return ! (operator ==(ls2));
}

/*
 * copy constructor of lockset
 */
LockSet& LockSet::operator=(LockSet& rhs)
{
	//cout << "LockSet is Copied" << endl;
	//UINT32 size = rhs.locks.size();
	//for (UINT32 i=0; i < size; ++i)
		//	locks.push_back(new Lock(rhs.locks.at(i)->addr));
	locks = rhs.locks;
	return *this;
}


/*
 * remove a lock from the set if exists
 */
void LockSet::removeLock(Lock *l)
{
	if (locks.size() == 0)
		return;
	vector<Lock*>::iterator it = locks.begin();
	for(; it != locks.end() ; ++it )
	{
		//cout <<  l->addr << "   " << (*it)->addr << endl;
		if (  l->addr == (*it)->addr  )
		{
			it = locks.erase(it);
			this->sorted = false;
			return;
		}
		//else
			//++it;
	}
}

string LockSet::toString()
{

	ostringstream os;
	os << "Locks size:" << locks.size() << endl;
	for(UINT32 i=0;i<locks.size();++i)
	{
		//cout << "lock found" << endl;
		os << (locks.at(i))->addr;
		if (i != locks.size() -1 )
			os <<  " --- ";
		else
			return os.str();
	}
	os << endl;
	return os.str();
}

/*
 * check if two locksets are equal
 */
bool LockSet::areEqual(LockSet* ls2)
{
	if (this->locks.size() != ls2->locks.size())
		return false;

	if (this->locks.size() == 0 || ls2->locks.size() == 0)
			return false;

	vector<Lock*>::iterator first_1 = this->locks.begin();
	vector<Lock*>::iterator first_2 = ls2->locks.begin();
	vector<Lock*>::iterator last_1 ;
	vector<Lock*>::iterator last_2 ;


	sort(first_1, this->locks.end(), ComparatorLockAddr());
	sort(first_2, ls2->locks.end(), ComparatorLockAddr());


	first_1 = this->locks.begin();
	first_2 = ls2->locks.begin();
	last_1 = this->locks.end();
	last_2 = ls2->locks.end();
	//cout << "*********************" << endl;
	//for (;first_1 != last_1 && first_2 != last_2; ++first_1,++first_2)
		//cout << "addr1:" << (*first_1)->addr << "  addr2:" << (*first_2)->addr << endl;

	//cout << "**********************" << endl;
	for (;first_1 != last_1 && first_2 != last_2; ++first_1,++first_2)
		if ((*first_1 )->addr != (*first_2)->addr)
			return false;

	return true;

}


bool LockSet::isSorted()
{
	if (this->sorted )
		return true;
	int size =  this->locks.size();
	if (size < 2)
	{
		this->sorted = true;
		return true;
	}
	for (int i=0; i <size - 1; ++i)
		if (this->locks.at(i)->addr > this->locks.at(i+1)->addr)
		{
			this->sorted = false;
			return false;
		}
	this->sorted = true;
	return true;
}

/*
 * if intersection of lockset is empty
 * return true
 * else
 * false
 */
bool LockSet::isIntersectionEmpty(LockSet* ls2)
{
	if (this->locks.size() == 0 || ls2->locks.size() == 0)
			return true;

	vector<Lock*>::iterator first_1 = this->locks.begin();
	vector<Lock*>::iterator first_2 = ls2->locks.begin();
	vector<Lock*>::iterator last_1 ;
	vector<Lock*>::iterator last_2 ;


	if (!this->isSorted())
	sort(first_1, this->locks.end(), ComparatorLockAddr());
	if (!ls2->isSorted())
	sort(first_2, ls2->locks.end(), ComparatorLockAddr());


	first_1 = this->locks.begin();
	first_2 = ls2->locks.begin();
	last_1 = this->locks.end();
	last_2 = ls2->locks.end();

	//output_stream << "ls1.size:" << this->locks.size() << "   ls2->size:" << ls2->locks.size() << endl;
	//output_stream << "(first_1 != last_1):" << (first_1 != last_1) << "   (first_2 != last_2):" <<  (first_2 != last_2) << endl;

	//this code is quite complicated. Please refer to set_intersection implementation of <algorithms>  library.
	//The code below is an adaptation of that set_intersection implementation for performance reasons
	while ( (first_1 != last_1) && (first_2 != last_2) )
	{
		if ((*first_1)->addr<(*first_2)->addr)
			++first_1;
		else if ((*first_2)->addr<(*first_1)->addr)
			++first_2;
		else
			return false;

	}

  return true;
}



