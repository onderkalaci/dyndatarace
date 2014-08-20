/*
 * AllVariables.h
 *
 *  Created on: Jun 20, 2013
 *      Author: onder
 */

#ifndef ALLVARIABLES_H_
#define ALLVARIABLES_H_

#include "definitions.h"
#include "MemoryAddr.h"

#define totalNumberOfHashMaps 1
#define totalNumberOfMemoryAddresses 30000000

typedef unordered_map<UINT64, MemoryAddr*> variablesHashMap;

class AllMemomryAddresses
{

public:
	AllMemomryAddresses();
	int singleHashMapSize;
	int bitMask;
	vector<variablesHashMap*> variableHashMaps;


	variablesHashMap* getVariableHashMap(ADDRINT effectiveAddr);
	PIN_LOCK getAssociatedLock(ADDRINT effectiveAddr);
	void generateBitMask();
	UINT64 calculateAllCapacity();

};



#endif /* ALLVARIABLES_H_ */
