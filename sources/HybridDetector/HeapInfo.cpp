/*
 * HeapInfo.cpp
 *
 *  Created on: Jul 26, 2013
 *      Author: onder
 */



#include "HeapInfo.h"

/*
 * A heapinfo data structure utilized when malloc is called
 * its base address and its size is saved
 * Then, in the execution any address is checked whether it is
 * in this heap or not
 */

HeapInfo::HeapInfo(ADDRINT baseMallocAddrIn, UINT32 sizeIn)
{
	this->baseMallocAddr = baseMallocAddrIn;
	this->size = sizeIn;
	this->nextHeapInfo = NULL;
}
