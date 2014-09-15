
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
 * AllRaceInstances.cpp
 *
 *  Created on: May 12, 2013
 *      Author: onder
 */

#include "AllRaceInstances.h"



bool AllRaceInstances::isInstructionHasRace(ADDRINT instPtr)
{
	return false;
	if (allRacesInstPointers.count(instPtr) == 0)
		return false;

	return true;
}

bool AllRaceInstances::hasRaceHappenedOnThisLine(string fileNameIn, int lineNumberIn)
{
	return false;

	if (fileNameIn == "" || lineNumberIn == 0) //line bilgisini alamazsan return et
		return false;


	std::map<ADDRINT, RaceInstance*>::iterator it;
	for (it = allRaces.begin(); it != allRaces.end(); ++it)
		if (it->second != NULL)
			try
			{
				if ( (it->second->exactFileName == fileNameIn	&& it->second->exactLineNumber == lineNumberIn)	)
					return true;

			}
			catch(...)
			{
				cout << "except" << endl;
			}

	return false;

}

void AllRaceInstances::removeSameLineRaces()
{
	std::map<ADDRINT, RaceInstance*>::iterator first_it;
	std::map<ADDRINT, RaceInstance*>::iterator second_it;
	for (first_it = allRaces.begin(); first_it != allRaces.end(); ++first_it)
	{
		for (second_it = allRaces.begin(); second_it != allRaces.end(); ++second_it)
		{
			if ( first_it != second_it 													&&
				first_it->second->print 												&&
				first_it->second->exactFileName == second_it->second->exactFileName     &&
				first_it->second->exactLineNumber == second_it->second->exactLineNumber &&
				first_it->second->exactFileName != ""
			)
				second_it->second->print = false;
		}

	}
}


void AllRaceInstances::removeSameInstPtr()
{
	std::map<ADDRINT, RaceInstance*>::iterator first_it;
	std::map<ADDRINT, RaceInstance*>::iterator second_it;
	for (first_it = allRaces.begin(); first_it != allRaces.end(); ++first_it)
	{
		for (second_it = allRaces.begin(); second_it != allRaces.end(); ++second_it)
		{
			if ( first_it != second_it 													&&
				first_it->second->print 												&&
				first_it->second->stackPtr == second_it->second->stackPtr
			)
				second_it->second->print = false;
		}

	}
}

bool AllRaceInstances::isMemoryAlreadyHasRace(ADDRINT memoryAddr)
{
	if (allRaces.count(memoryAddr) == 0)
		return false;

	return true;
}

void AllRaceInstances::allRaceAddresses()
{

	std::map<ADDRINT, RaceInstance*>::iterator it;
		for (it = allRaces.begin(); it != allRaces.end(); ++it)
			if (it->second != NULL)
				try
				{
					cout << it->second->insPtr << endl;
				}
				catch(...)
				{
					cout << "except" << endl;
				}
}

int AllRaceInstances::getRaceCount()
{
	return allRaces.size();
}

bool AllRaceInstances::isRaceOK(ADDRINT memoryAddr, ADDRINT instPtr,   isMallocAreaStrct isMalloced)
{

	//if (allRacedArrayAddresses.count(isMalloced.addrToUse ) > 0)
		//cout << "address malloced raced:" << memoryAddr << endl;
	//return true;
	if ( (allRaces.count(memoryAddr) != 0  ) || allRacedArrayAddresses.count(isMalloced.addrToUse) > 0 || allRacesInstPointers.count(instPtr) > 0)
		return false;

	return true;
}

void AllRaceInstances::addToRaces(RaceInstance* currentRace, isMallocAreaStrct isMalloced)
{

	if (!isRaceOK( currentRace->memoryAddr,  currentRace->insPtr, isMalloced))
	{
		if (currentRace != NULL)
			delete currentRace;
		return;
	}
		//cout << hex << "AddToRaces:" <<  currentRace->memoryAddr  << dec << endl;
	allRaces[currentRace->memoryAddr] = currentRace;
	allRacesInstPointers[currentRace->insPtr] = currentRace;


	if (isMalloced.isMallocArea)
	{

		allRacedArrayAddresses[isMalloced.addrToUse] = currentRace->memoryAddr;
		;//cout << "RACE ARE MALLOCED:" << hex<< isMalloced.addrToUse <<  endl;

	}
	else
	;//	cout << "RACE ARE NOT MALLOCED" << endl;



	//allRacesInstPointers[currentRace->segmentOneInstPtr] = currentRace;
	//allRacesInstPointers[currentRace->segmentTwoInstPtr] = currentRace;


}



