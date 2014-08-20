/*
 * AllRaceInstances.h
 *
 *  Created on: May 12, 2013
 *      Author: onder
 */

#ifndef ALLRACEINSTANCES_H_
#define ALLRACEINSTANCES_H_

#include "RaceInstance.h"
#include <map>
using namespace std;

struct isMallocAreaStrct
{
public:
	bool isMallocArea;
	ADDRINT addrToUse;
	ADDRINT freeSize;

	isMallocAreaStrct(bool b, ADDRINT a) : isMallocArea(b), addrToUse(a)
	{
		freeSize = 0;
	}
};


class racedArrayStruct
{
public:
	ADDRINT arrayStart;
	ADDRINT raceStart;
	UINT32 addressSize;

	racedArrayStruct(ADDRINT arrayStartIn, ADDRINT raceStartIn, UINT32 addressSizeIn) :
		arrayStart(arrayStartIn), raceStart(raceStartIn), addressSize(addressSizeIn)
	{}
};
class AllRaceInstances
{


public:
	bool isMemoryAlreadyHasRace(ADDRINT memoryAddr);
	bool isInstructionHasRace(ADDRINT insPtr);
	void addToRaces(RaceInstance* currentRace, ADDRINT isMallocedAddrToUse);
	map<ADDRINT,RaceInstance*> allRaces;
	map<ADDRINT,RaceInstance*> allRacesInstPointers;
	map<ADDRINT,ADDRINT> allRacedArrayAddresses;


	bool hasRaceHappenedOnThisLine(string fileName, int lineNumber);
	void removeSameLineRaces();




	int getRaceCount();
	bool isRaceOK(ADDRINT memoryAddr, ADDRINT instPtr,  ADDRINT isMallocedAddrToUse);

	bool isRaceOKOutside(ADDRINT memoryAddr, ADDRINT instPtr, ADDRINT isMallocedAddrToUse);

	void allRaceAddresses();

};




#endif /* ALLRACEINSTANCES_H_ */
