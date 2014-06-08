#!/usr/bin/python




PARSEC_DIR = "/home/onderkalaci/TOOLS/parsec-3.0/"
PIN_EXECUTABLE = "/home/alpersen/TOOLS/pin-2.12-56759-gcc.4.4.7-linux/pin"
TSAN_EXACUTABLE = "/home/onderkalaci/TOOLS/tsan-r4431-amd64-linux-self-contained.sh"
import os
import sys
import time
import  xml.etree.ElementTree as ET
import xml.dom.minidom
from time import gmtime, strftime
import subprocess as sub
import threading


noOfThread, simType,current_time = "", "",""
outputFile = ""
allExamples = []
directory = "test_results" 
ITERATION_TIME = 1
SAMPLE_RATES = [1.00,0.90,0.80,0.70,0.60,0.50,0.40,0.30,0.20,0.15,0.10,0.08,0.05, 0.03,0.02,0.01]
SINGLE_SEGMENT_SIZES = [ 25,20,15,12,10,7,5,3,2,1,0]
MAX_EXECUTION_TIMEOUT = 30
xmlTreeResults = ET.Element('Results')

class RunCmd(threading.Thread):
    def __init__(self, cmd, timeout):
        threading.Thread.__init__(self)
        self.cmd = cmd
        self.timeout = timeout
	self.output = ""
    def run(self):
		print "small run"
		self.p = sub.Popen(self.cmd, stdout=sub.PIPE, stderr=sub.PIPE)
		self.p.wait()
		self.output = self.p.stderr.read() + "\n" +  self.p.stdout.read()  
		
    def Run(self):
		self.start()
		self.join(self.timeout)
		while self.is_alive():
			print "line"
			try:
				self.p.terminate()
				self.p.kill()
			except:
				break
		self.join()
		return self.output
class ParsecExample():
	def __init__(self, name, executable_path, sim_small_input_path, sim_middle_input_path, sim_large_input_path, no_of_threads, segmentCount):
		self.name = name
		self.executable_path = executable_path
		self.sim_small_input_path = sim_small_input_path
		self.sim_middle_input_path = sim_middle_input_path
		self.sim_large_input_path = sim_large_input_path
		self.no_of_threads = str(no_of_threads)
		self.segment_count = str(segmentCount)
	
	def tail(self,fileName, n):	
		stdin,stdout = os.popen2("tail -n "+n+" "+fileName)
		stdin.close()
		lines = stdout.readlines()
		stdout.close()
		return lines
		
	def generateSimSmallExecution(self):
		return self.executable_path +  " " + self.sim_small_input_path
	def generateSimMediumExecution(self):
		return self.executable_path + " " + self.sim_middle_input_path
	def generateSimLargeExecution(self):
		return self.executable_path + " " + self.sim_large_input_path
	
	def findRaceCount(self,implementationType, fileName ):
		
		if implementationType.lower() == "google":
			lines = self.tail(fileName, "1")
			if len(lines) > 0:
				if len(lines[0].split("reported ")) > 0:
					if len(lines[0].split("reported ")[1].split(" ")) > 0:
						return lines[0].split("reported ")[1].split(" ")[0].strip()
			
		elif implementationType.lower() == "own":
			lines = self.tail(fileName, "1")
			if len(lines) > 0:
				if len(lines[0].split(":")) > 1:
					return lines[0].split(":")[1].strip()
			
		
		return "NOT READY YET"
		
	def addResultToXML(self, implementationType, parameterMap):   #implementationType is either GOOGLE or OWN
		
		xmlTmpElement = ET.SubElement(xmlTreeResults, implementationType)
		
		for parameter in parameterMap:
			xmlTmpElementType = ET.SubElement(xmlTmpElement, str(parameter))
			xmlTmpElementType.text = str(parameterMap[parameter])

	
	def executeOurImpelemtation(self, algorithm, sharedObj, parameterMap ,outputFileName):
		executeStr = PIN_EXECUTABLE + " -t " + sharedObj
		for parameter in parameterMap:
			if parameterMap[parameter]  == "" and parameter != "enable_signle_access": 
				continue
			executeStr = executeStr + " -" +  parameter + " " + parameterMap[parameter] 
		
		executeStr = executeStr  +  " -- "
		if simType == "simsmall":
			executeStr = executeStr +  self.generateSimSmallExecution()
		elif simType == "simmedium":
			executeStr = executeStr + self.generateSimMediumExecution()
		elif simType == "simlarge":
			executeStr = executeStr + self.generateSimLargeExecution()
		else:
			print "OPPS, execute tsan with simtype:", simType

		fileName = current_time.replace(" ", "_") + "_onder_" + outputFileName + "_" + self.name + "_" + simType+".txt"
		executeStr = executeStr + " > "+ fileName + " 2>&1"
		#print executeStr
		print executeStr 
		startTime = time.time()
		#endTime = time.time()
		#f = open(fileName, "w")
		#output = RunCmd(executeStr.split(" "), MAX_EXECUTION_TIMEOUT).Run()
		os.system(executeStr)
		#print strToExecute
		#print self.generateSimSmallExecution()
		#os.system(self.generateSimSmallExecution())
		endTime = time.time()
		#f = open(fileName, "w")
		#f.write(output)
		#f.close()
		parameterMap["ALGORITHM"] = algorithm
		parameterMap["RACE_COUNT"] = self.findRaceCount("OWN", fileName)
		parameterMap["TIME"] =  str( endTime - startTime)
		parameterMap["APP_NAME"] = self.name
		parameterMap["INPUT_TYPE"] = simType
		
		self.addResultToXML("OWN", parameterMap)
		
	def executeTsan(self, isHappensBefore):
		
		executeStr = TSAN_EXACUTABLE +  " --pure-happens-before="+ isHappensBefore + " --ignore=tsan.ignore --free-is-write=no "
		if simType == "simsmall":
			executeStr = executeStr +  self.generateSimSmallExecution()
		elif simType == "simmedium":
			executeStr = executeStr + self.generateSimMediumExecution()
		elif simType == "simlarge":
			executeStr = executeStr + self.generateSimLargeExecution()
		else:
			print "OPPS, execute tsan with simtype:", simType
		
		if isHappensBefore == "true":
			executeStr = executeStr + " > tsan_purehb_" + self.name + "_" + simType +".txt 2>&1"
		else:
			executeStr = executeStr + " > tsan_hybrid_" + self.name + "_" + simType +".txt 2>&1"

		startTime = time.time()
		os.system(executeStr)
		endTime = time.time()
		fileName,algorithm = "",""
		if isHappensBefore == "true":
			algorithm = "happensbefore"
			fileName = "tsan_purehb_" + self.name + "_" + simType +".txt"
		else:
			algorithm = "hybrid"
			fileName = "tsan_hybrid_" + self.name + "_" + simType +".txt"
		
		
		
		self.addResultToXML("GOOGLE", algorithm, fileName,  str( endTime - startTime))
		
	

def initObjects(noOfThreads, simType):
	
	netFerret = ParsecExample("canneal",PARSEC_DIR +  "pkgs/kernels/canneal/inst/amd64-linux.gcc-pthreads/bin/canneal " + noOfThreads, "10000 2000 " + PARSEC_DIR + "pkgs/kernels/canneal/run/100000.nets 32", "15000 2000 " + PARSEC_DIR + "pkgs/kernels/canneal/run/200000.nets 64", "15000 2000 " + PARSEC_DIR + "pkgs/kernels/canneal/run/400000.nets 128", noOfThreads)	
	strToExecute = "tar -C " + PARSEC_DIR + "pkgs/kernels/canneal/run " + "-xvf " + PARSEC_DIR + "pkgs/kernels/canneal/inputs/input_" + simType +  ".tar"
	#print netFerret.name, " is prepared:" , strToExecute
	os.system(strToExecute)
	
	
	
	streamCluster = ParsecExample("streamcluster",  PARSEC_DIR + "pkgs/kernels/streamcluster/inst/amd64-linux.gcc-pthreads/bin/streamcluster", "10 20 32 4096 4096 1000 none output.txt " + noOfThreads , "10 20 64 8192 8192 1000 none output.txt " + noOfThreads,   "10 20 128 16384 16384 1000 none output.txt " + noOfThreads , noOfThreads)
	
	
	fluidanimate = ParsecExample("fluidanimate",  PARSEC_DIR + "pkgs/apps/fluidanimate/obj/amd64-linux.gcc-pthreads/fluidanimate", noOfThreads + " 5 " + PARSEC_DIR + "pkgs/apps/fluidanimate/run/in_35K.fluid out.fluid" , noOfThreads + " 5 " + PARSEC_DIR + "pkgs/apps/fluidanimate/run/in_100K.fluid out.fluid" , noOfThreads + " 5 " + PARSEC_DIR + "pkgs/apps/fluidanimate/run/in_300K.fluid out.fluid" , noOfThreads)
	strToExecute = "tar -C " + PARSEC_DIR + "pkgs/apps/fluidanimate/run " + "-xvf " + PARSEC_DIR + "pkgs/apps/fluidanimate/inputs/input_" + simType +  ".tar"
	#print fluidanimate.name, " is prepared:" , strToExecute
	os.system(strToExecute)


	

def initObjects(noOfThreads, simType):
	
	netFerret = ParsecExample("canneal",PARSEC_DIR +  "pkgs/kernels/canneal/inst/amd64-linux.gcc-pthreads/bin/canneal " + noOfThreads, "10000 2000 " + PARSEC_DIR + "pkgs/kernels/canneal/run/100000.nets 32", "15000 2000 " + PARSEC_DIR + "pkgs/kernels/canneal/run/200000.nets 64", "15000 2000 " + PARSEC_DIR + "pkgs/kernels/canneal/run/400000.nets 128", noOfThreads, 1400)	
	strToExecute = "tar -C " + PARSEC_DIR + "pkgs/kernels/canneal/run " + "-xvf " + PARSEC_DIR + "pkgs/kernels/canneal/inputs/input_" + simType +  ".tar"
	#print netFerret.name, " is prepared:" , strToExecute
	os.system(strToExecute)
	
	
	
	streamCluster = ParsecExample("streamcluster",  PARSEC_DIR + "pkgs/kernels/streamcluster/inst/amd64-linux.gcc-pthreads/bin/streamcluster", "10 20 32 4096 4096 1000 none output.txt " + noOfThreads , "10 20 64 8192 8192 1000 none output.txt " + noOfThreads,   "10 20 128 16384 16384 1000 none output.txt " + noOfThreads , noOfThreads, 175000)
	
	
	fluidanimate = ParsecExample("fluidanimate",  PARSEC_DIR + "pkgs/apps/fluidanimate/obj/amd64-linux.gcc-pthreads/fluidanimate", noOfThreads + " 5 " + PARSEC_DIR + "pkgs/apps/fluidanimate/run/in_35K.fluid out.fluid" , noOfThreads + " 5 " + PARSEC_DIR + "pkgs/apps/fluidanimate/run/in_100K.fluid out.fluid" , noOfThreads + " 5 " + PARSEC_DIR + "pkgs/apps/fluidanimate/run/in_300K.fluid out.fluid" , noOfThreads, 2170000)
	strToExecute = "tar -C " + PARSEC_DIR + "pkgs/apps/fluidanimate/run " + "-xvf " + PARSEC_DIR + "pkgs/apps/fluidanimate/inputs/input_" + simType +  ".tar"
	#print fluidanimate.name, " is prepared:" , strToExecute
	os.system(strToExecute)

	vips = ParsecExample("vips",  PARSEC_DIR + "pkgs/apps/vips/inst/amd64-linux.gcc-pthreads/bin/vips im_benchmark",  PARSEC_DIR + "pkgs/apps/vips/run/pomegranate_1600x1200.v " +  PARSEC_DIR + "pkgs/apps/vips/run/output.v " ,  PARSEC_DIR + "pkgs/apps/vips/run/vulture_2336x2336.v " +  PARSEC_DIR + "pkgs/apps/vips/run/output.v " ,  PARSEC_DIR + "pkgs/apps/vips/run/bigben_2662x5500.v "+  PARSEC_DIR + "pkgs/apps/vips/run/output.v " ,  noOfThreads, 4000)
	strToExecute = "tar -C " + PARSEC_DIR + "pkgs/apps/vips/run " + "-xvf " + PARSEC_DIR + "pkgs/apps/vips/inputs/input_" + simType +  ".tar"
	#print vips.name, " is prepared:" , strToExecute
	os.system(strToExecute)
	
	
	bodyTrack = ParsecExample("bodytrack",  PARSEC_DIR + "pkgs/apps/bodytrack/inst/amd64-linux.gcc-pthreads/bin/bodytrack ",  PARSEC_DIR + "pkgs/apps/bodytrack/run/sequenceB_1 4 1 1000 5 0 " +  noOfThreads , PARSEC_DIR + "pkgs/apps/bodytrack/run/sequenceB_2 4 2 2000 5 0 " +  noOfThreads , PARSEC_DIR + "pkgs/apps/bodytrack/run/sequenceB_4 4 4 4000 5 0 " +  noOfThreads ,  noOfThreads, 10000)
	strToExecute = "tar -C " + PARSEC_DIR + "pkgs/apps/bodytrack/run " + "-xvf " + PARSEC_DIR + "pkgs/apps/bodytrack/inputs/input_" + simType +  ".tar"
	#print vips.name, " is prepared:" , strToExecute
	os.system(strToExecute)
	
	swaptions = ParsecExample("swaptions",  PARSEC_DIR + "pkgs/apps/swaptions/inst/amd64-linux.gcc-pthreads/bin/swaptions ",  "-ns 16 -sm 10000 -nt " +  noOfThreads , "-ns 32 -sm 20000 -nt " +  noOfThreads , "-ns 64 -sm 40000 -nt " +  noOfThreads ,  noOfThreads, 500)
	#print vips.name, " is prepared:" , strToExecute
	
	 
	x264 = ParsecExample("x264", PARSEC_DIR + "pkgs/apps/x264/inst/amd64-linux.gcc-pthreads/bin/x264 ",   "--quiet --qp 20 --partitions b8x8,i4x4 --ref 5 --direct auto --b-pyramid --weightb --mixed-refs --no-fast-pskip --me umh --subme 7 --analyse b8x8,i4x4 --threads " + noOfThreads + " -o "+  PARSEC_DIR + "pkgs/apps/x264/run/"  +"eledream.264 " + PARSEC_DIR + "pkgs/apps/x264/run/eledream_640x360_8.y4m",	  "--quiet --qp 20 --partitions b8x8,i4x4 --ref 5 --direct auto --b-pyramid --weightb --mixed-refs --no-fast-pskip --me umh --subme 7 --analyse b8x8,i4x4 --threads " + noOfThreads + " -o "+  PARSEC_DIR + "pkgs/apps/x264/run/"  +"eledream.264 " + PARSEC_DIR + "pkgs/apps/x264/run/eledream_640x360_32.y4m",	  "--quiet --qp 20 --partitions b8x8,i4x4 --ref 5 --direct auto --b-pyramid --weightb --mixed-refs --no-fast-pskip --me umh --subme 7 --analyse b8x8,i4x4 --threads " + noOfThreads + " -o "+  PARSEC_DIR + "pkgs/apps/x264/run/"  +"eledream.264 " + PARSEC_DIR + "pkgs/apps/x264/run/eledream_640x360_128.y4m",   noOfThreads, 7500)
	strToExecute = "tar -C " + PARSEC_DIR + "pkgs/apps/x264/run " + "-xvf " + PARSEC_DIR + "pkgs/apps/x264/inputs/input_" + simType +  ".tar"
	#print vips.name, " is prepared:" , strToExecute
	os.system(strToExecute)

	freqmine = ParsecExample("freqmine", PARSEC_DIR + "pkgs/apps/freqmine/inst/amd64-linux.gcc/bin/freqmine ",  PARSEC_DIR + "pkgs/apps/freqmine/run/kosarak_250k.dat 220"  , PARSEC_DIR + "pkgs/apps/freqmine/run/kosarak_500k.dat 410"  , PARSEC_DIR + "pkgs/apps/freqmine/run/kosarak_990k.dat 790"  , noOfThreads, 1400)
	strToExecute = "tar -C " + PARSEC_DIR + "pkgs/apps/freqmine/run " + "-xvf " + PARSEC_DIR + "pkgs/apps/freqmine/inputs/input_" + simType +  ".tar"
	#print vips.name, " is prepared:" , strToExecute
	os.system(strToExecute)
	
	allExamples.append(netFerret)
	allExamples.append(streamCluster)
	allExamples.append(fluidanimate)
	allExamples.append(vips)
	allExamples.append(bodyTrack)   #9 fakika
	allExamples.append(swaptions)
	allExamples.append(x264)
	allExamples.append(freqmine)
	return allExamples
if __name__ == "__main__":


	#time.sleep(60*60*2) 
	#ITERATION_TIME = len(SAMPLE_RATES)
	try:
		noOfThread = sys.argv[2]
		simType = sys.argv[1]
		simTypes = ["simsmall", "simmedium", "simlarge"]
		if simType not in simTypes:
			print "sim type must be on of " + str(simTypes)
			sys.exit(-1)
		
		if int(noOfThread) < 0 or int(noOfThread) > 50:
			print "no of threads must be between 0-50"
			sys.exit(-1)
		
	except Exception, e:
		print "Exception happened:" , e 
		sys.exit(-1)
	print noOfThread
	initObjects(str(noOfThread), simType)
	print "executions start...."
	
	counter = -1
	maxSize = 10
        vcHistoryList = ["0", "1", "2", "5", "10", "20", "50" , "100", "250" , "500", "1000"]
	ITERATION_TIME = len(vcHistoryList)  
	while counter < ITERATION_TIME-1:
		counter = counter + 1
		#maxSize = int(maxSize - 1)
		#if maxSize == 0:
		#	continue
		#counter = counter + 1000 #iteration time 
		#if counter < 2:
		#	counter = counter + 0.2
		#elif counter < 10:
		#	counter = counter + 1
		#else:
		#	counter = counter + 10 #iteration time 
		current_time = strftime("%H:%M:%S_%d_%m_%Y", gmtime())
		if not os.path.exists(directory):
			os.makedirs(directory)
		outputFile = open("test_results/" + current_time +  "_execution_history.txt", "w")


		#print "ctr:", counter
		for example in allExamples:
			#example.executeOurImpelemtation("purelocset", "PureLocksetImp.so",{} , "purelockset_")
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{} , "tsan_1_0")
			#example.executeOurImpelemtation("purehb", "PureHappensBeforeImp.so",{}, "purehb_")
			

			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{ "enable_signle_access" : ""}, "tsan_131072_1")
			
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"maxVCHistoryCount" :vcHistoryList[counter-1] } , "tsan_1_0")
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"maxVCHistoryCount" :vcHistoryList[counter-1] } , "tsan_1_0")
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{ "enable_signle_access" : ""}, "tsan_131072_1")
                        #print vcHistoryList[counter - 1]
			#example.executeOurImpelemtation("purehb", "PureHappensBeforeImp.so",{}, "purehb_")
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"max_single_segment_set_size":segmentCountForThisExecution, "enable_signle_access" : "", "sample_rate" :str(SAMPLE_RATES[counter]) } , "tsan_1_0")
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"mem_chunk_count" : "4096", "sample_rate" : str(SAMPLE_RATES[counter])} , "tsan_1_0")
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"mem_chunk_count" : "4096"}, "tsan_1_0")

			#continue
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so", "tsan_")
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so", "tsan_")
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"mem_chunk_count" :"32768" , "enable_signle_access" : ""}, "tsan_32768_1")
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"mem_chunk_count" :"131072", "enable_signle_access" : ""}, "tsan_131072_1")

			segmentCountForThisExecution = str( (( (ITERATION_TIME +1 )  - (counter+1)) / (ITERATION_TIME * 1.0 * (counter+1))) * (1.0 * int(example.segment_count)))
                        #print example.name,"segment count:", segmentCountForThisExecution, "counter:",counter, "iteration:",  ITERATION_TIME, "segment count:",  example.segment_count
			#print "COUNTER:", counter , "ITERATION_TIME:" , ITERATION_TIME
			example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"segment_history_table_size":segmentCountForThisExecution, "enable_signle_access" : "", "maxVCHistoryCount" :vcHistoryList[counter] } , "tsan_1_0")
			#sample_rate = str(round(float(1.0 / counter),2)) 
			#print "counter:", counter
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"max_single_segment_set_size" : str(SINGLE_SEGMENT_SIZES[counter])}, "tsan_0_0")
			#example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{}, "tsan_0_0")

			#example.executeOurImpelemtation("empty", "EmptyImp.so",{}, "empty")
		xmlRes 	= xml.dom.minidom.parseString(ET.tostring(xmlTreeResults))
		outputFile.write(xmlRes.toprettyxml())
		outputFile.close()
		xmlTreeResults.clear()
