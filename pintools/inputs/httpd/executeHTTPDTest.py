#!/usr/bin/python
import os
import sys
import time

DEBUG_MODE = False
PIN_EXECUTABLE = "/home/onder/Desktop/PIN/pin-2.12-56759-gcc.4.4.7-linux/pin -follow_execv "
KILLHTTPD = " sudo killall -SIGINT apache2"
HTTPDSTARTCOMMAND = "sudo /usr/sbin/apache2ctl -X "
CLIENT_COUNT = "20"
REQUEST_COUNT = "100"
GENERATOREXECUTER = "./generator.py -n " + CLIENT_COUNT + " -c " + REQUEST_COUNT

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
		killHTTPD()
		sharedObjectName = sharedObject.split(".")[0]
		strToExecute = PIN_EXECUTABLE + " -t ../" + sharedObject + " -- " + HTTPDSTARTCOMMAND + " > " + sharedObjectName + "_httpd.txt 2>&1 &"
		
		
		os.system(strToExecute)
		os.system("sleep 20")
		print "apache2 started...."
		startTime = time.time()
		os.system(GENERATOREXECUTER)
		endTime = time.time()
		totalTime = str( endTime - startTime)
		killHTTPD()
		print "killed apache"
		os.system("sleep 3")
		f.write("************************\n")
		strToWrite = sharedObjectName  +  "  time : "  +    totalTime + " race:" + findExactTotalRace( sharedObjectName + "_httpd.txt")
		f.write(strToWrite)
		f.write("\n************************\n")
	f.close()
executePIN()
	
