/*
 * AllVariables.cpp
 *
 *  Created on: Jun 20, 2013
 *      Author: onder
 */

#include "AllVariables.h"

extern UINT64 totalNumberOfHashMaps;
extern UINT64 totalNumberOfMemoryAddresses;

AllMemomryAddresses::AllMemomryAddresses()
{


}

void AllMemomryAddresses::initilaze()
{
	singleHashMapSize = (int) totalNumberOfMemoryAddresses / totalNumberOfHashMaps;
	cout << "AllMemomryAddresses: totalNumberOfHashMaps" << totalNumberOfHashMaps << endl;
	cout << "singleHashMapSize::" << singleHashMapSize << endl;
	cout << "totalNumberOfHashMaps:" << totalNumberOfHashMaps << endl;
	int totalNumberOfHashMapsSize = totalNumberOfHashMaps;

	for (int i = 0; i < totalNumberOfHashMapsSize; ++i)
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

UINT64 AllMemomryAddresses::calculateAllCapacity()
{
	UINT64 totalCount = 0;
	int totalNumberOfHashMapsSize = totalNumberOfHashMaps;
	for (int i = 0; i < totalNumberOfHashMapsSize; ++i)
	{
		UINT64 tmpCount = 0;
		UINT64 locCount = 0;
		unordered_map<UINT64, MemoryAddr*>* tmpHashMap =(unordered_map<UINT64, MemoryAddr*>*) variableHashMaps.at(i);
		std::unordered_map<ADDRINT, MemoryAddr*>::iterator it;
		for (it = tmpHashMap->begin(); it != tmpHashMap->end(); ++it)
		{
			tmpCount = tmpCount + (*it).second->readerSegmentSet.segments.size()
							    + (*it).second->writerSegmentSet.segments.size();
			++locCount;
		}
		cout <<"Total Count:" << totalCount << "   tmpCount:" << tmpCount  << "  Location Count:" << locCount<< endl;

		totalCount = totalCount + tmpCount;
	}
	cout << "Total Count:" << totalCount << endl;

	return 0;
}

