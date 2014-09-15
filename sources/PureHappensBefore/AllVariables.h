
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
