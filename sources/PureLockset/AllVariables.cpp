
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
 * AllVariables.cpp
 *
 *  Created on: Jun 20, 2013
 *      Author: onder
 */

#include "AllVariables.h"


AllMemomryAddresses::AllMemomryAddresses()
{
	singleHashMapSize = (int) totalNumberOfMemoryAddresses / totalNumberOfHashMaps;
	cout << "singleHashMapSize::" << singleHashMapSize << endl;
	cout << "totalNumberOfHashMaps:" << totalNumberOfHashMaps << endl;
	for (int i = 0; i <  totalNumberOfHashMaps; ++i)
	{
		//init hashMap
		variablesHashMap* tmpHashMap = new variablesHashMap();
		tmpHashMap->reserve(singleHashMapSize);
		variableHashMaps.push_back(tmpHashMap);
	}
	generateBitMask();

}

void AllMemomryAddresses::generateBitMask()
{
	bitMask = 0;
	int logValue = log2(totalNumberOfHashMaps);
	bitMask = totalNumberOfHashMaps -1 ;
	cout << "Total Bit to Use:" << logValue << "  bitMask:" << bitMask << endl;
}

variablesHashMap*AllMemomryAddresses:: getVariableHashMap(ADDRINT effectiveAddr)
{

	//if ((effectiveAddr & requiredBits) != 0)
		//cout << "Upps error_2 : " << effectiveAddr << "  "  << requiredBits << endl;

	return variableHashMaps[ (effectiveAddr >> 3) & bitMask];
}



