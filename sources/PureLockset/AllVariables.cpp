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



