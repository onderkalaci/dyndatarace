
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
 * PthreadCallInstrumentation.h
 *
 *  Created on: Apr 23, 2013
 *      Author: onder
 */

#ifndef PTHREADCALLINSTRUMENTATION_H_
#define PTHREADCALLINSTRUMENTATION_H_


#include "definitions.h"

#define libpthread "libpthread"

VOID InstrumentImage(IMG img, VOID *v);
VOID GetWriteLockAnalysis(ADDRINT arg1, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID GetReadLockAnalysisEnter(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID mutexTryLockExitAnalysis(ADDRINT exitVal, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID mutexTryLockEnterAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName , ADDRINT stackPtr);
VOID GetWriteLockAnalysisEnter(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID PthreadJoinAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID PthreadCreateAnalysis(ADDRINT lockAddr, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID GetReadLockAnalysis(ADDRINT arg1, THREADID id ,char* imageName, ADDRINT stackPtr);
VOID ReleaseLockAnalyis(ADDRINT arg1, THREADID id ,char* imageName, ADDRINT stackPtr);


VOID FreeEnter(CHAR * name, ADDRINT memoryAddrFreeStarts, THREADID threadid);
VOID freeMemoryAddress(ADDRINT memoryAddrFreeStarts, ADDRINT maxMemoryAddrToBeFreed, THREADID threadid);
VOID moveMemoryAddresses(ADDRINT startOfOldPlace, ADDRINT startOfNewPlace, UINT32 size, THREADID tid);

void initFunctionNameInstrumentPthread();
void printThreadLockSets(THREADID id);


typedef struct
{
	string functionName;
	AFUNPTR instrumentFunction;
	IPOINT iPoint;
	UINT16 callOrder;

} functionNameInstrumentFunctionName;




#endif /* PTHREADCALLINSTRUMENTATION_H_ */
