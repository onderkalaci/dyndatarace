
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
