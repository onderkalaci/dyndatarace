#!/usr/bin/python
import os
import sys
import time

DEBUG_MODE = False
PIN_EXECUTABLE = "/home/onder/Desktop/PIN/pin-2.12-56759-gcc.4.4.7-linux/pin -follow_execv "
PBZIMSTARTCOMMAND = " pbzip2 -fb15k "
FILETOZIP = "pbzip_test_data.txt"


sharedObjectList = ["PureLocksetImp.so", "ThreadSanitizerImp.so", "PureHappensBeforeImp.so"]



def killHTTPD():
	os.system(KILLHTTPD)


def findExactTotalRace(fileName):
	totalCount = 0
	f = open(fileName, "r")
	lines = f.readlines()
	for line in lines:
		if line.find("totalRaceCount:") != -1:
			totalCount = totalCount + int(line.split(":")[1])
			
	return str(totalCount)


def executePIN():
	
	
	f = open("execution_history.txt", "w")
	for sharedObject in sharedObjectList:
		sharedObjectName = sharedObject.split(".")[0]
		strToExecute = PIN_EXECUTABLE + " -t ../" + sharedObject + " -- " + PBZIMSTARTCOMMAND + " " + FILETOZIP + " > " + sharedObjectName + "_pbzip.txt 2>&1 "
		startTime = time.time()
		os.system(strToExecute)
		endTime = time.time()
		totalTime = str( endTime - startTime)
		
		f.write("************************\n")
		strToWrite = sharedObjectName  +  "  time : "  +    totalTime + " race:" + findExactTotalRace( sharedObjectName + "_pbzip.txt")
		f.write(strToWrite)
		f.write("\n************************\n")
	f.close()
executePIN()
	
