/*
 * HeapInfo.h
 *
 *  Created on: Jul 26, 2013
 *      Author: onder
 */

#ifndef HEAPINFO_H_
#define HEAPINFO_H_
#include "definitions.h"

class HeapInfo
{

public:
	ADDRINT baseMallocAddr;
	UINT32 size;
	HeapInfo* nextHeapInfo;

	HeapInfo(ADDRINT baseMallocAddrIn, UINT32 sizeIn);


};



#endif /* HEAPINFO_H_ */
