
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
 * RaceInstance.h
 *
 *  Created on: May 9, 2013
 *      Author: onder
 */

#ifndef RACEINSTANCE_H_
#define RACEINSTANCE_H_

#include "definitions.h"


class RaceInstance
{



public:
	RaceInstance(string fileName, UINT32 lineNumber, ADDRINT memoryAddr, ADDRINT insPtr, UINT32 addressSizeIn,ADDRINT stackPtrIn);
	string toStringSimple();
	string toStringDetailed();
	ADDRINT memoryAddr;
	ADDRINT insPtr;
	void fillLineNumberInfo();

	string exactFileName;
	int exactLineNumber;
	ADDRINT stackPtr;


	UINT32 addressSize;
	BOOL print;

};


#endif /* RACEINSTANCE_H_ */
