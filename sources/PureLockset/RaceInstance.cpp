
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
 * RaceInstance.cpp
 *
 *  Created on: May 9, 2013
 *      Author: onder
 */

#include "RaceInstance.h"
#include <sstream>
#include <math.h>
//extern Logger logger;
RaceInstance::RaceInstance(string fileNameIn, UINT32 lineIn, ADDRINT memoryAddrIn, ADDRINT insPtrIn, UINT32 addressSizeIn, ADDRINT stackPtrI)
{



	exactFileName = fileNameIn;
	exactLineNumber = lineIn;
	memoryAddr = memoryAddrIn;
	insPtr = insPtrIn;
	print = true;
	stackPtr = stackPtrI;
	addressSize = addressSizeIn;
}

void RaceInstance::fillLineNumberInfo()
{
	string fileName;
	int lineNumber,col;
    PIN_LockClient();
    PIN_GetSourceLocation(insPtr, &col, &lineNumber, &fileName);
    PIN_UnlockClient();

   this->exactFileName = fileName;
   this->exactLineNumber = lineNumber;
}

string RaceInstance::toStringSimple()
{

    ostringstream os;
    os << endl << "-----------------------RACE INFO STARTS-----------------------------" << endl;
    os << "The Exact Place:"  << exactFileName << "@" << exactLineNumber   << endl;
    os << "-----------------------RACE INFO ENDS-----------------------------" << endl;

	return os.str();
}


string RaceInstance::toStringDetailed()
{
    ostringstream os;
    os << endl << "-----------------------RACE INFO STARTS-----------------------------:" << hex << memoryAddr  << dec << endl;
    os << "The Exact Place:"  << exactFileName << "@" << exactLineNumber << "   stackPtr:" << hex << stackPtr << dec   << endl;
    os << "Difference:" << abs((INT64)(stackPtr - memoryAddr)) << endl;

	os << "-----------------------RACE INFO ENDS-----------------------------" << endl;
	return os.str();
}
