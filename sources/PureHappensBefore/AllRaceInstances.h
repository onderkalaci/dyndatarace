
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
	void addToRaces(RaceInstance* currentRace,  isMallocAreaStrct isMalloced);
	map<ADDRINT,RaceInstance*> allRaces;
	map<ADDRINT,RaceInstance*> allRacesInstPointers;
	map<ADDRINT,ADDRINT> allRacedArrayAddresses;


	bool hasRaceHappenedOnThisLine(string fileName, int lineNumber);
	void removeSameLineRaces();
	void removeSameInstPtr();





	int getRaceCount();
	bool isRaceOK(ADDRINT memoryAddr, ADDRINT instPtr,  isMallocAreaStrct isMalloced);
	void allRaceAddresses();

};




#endif /* ALLRACEINSTANCES_H_ */
