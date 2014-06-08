dyndatarace
===========

Dynamic Data Race Detection

Four different pintools are under the folder "pintools". Their names are self explanatory.


===========
Inputs for httpd and pbzip2 is under "inputs" folder. We do not include the inputs for PARSEC 3.0 benchmark applications.
They are already freely avalible on the web "parsec.cs.princeton.edu/download.htm".

1) inputs/httpd:
 a) index.html is the 8.3k static web page that is used in the experiments.
 b) generator.py is the script that generates the clients which connect to the httpd web server, and GETs the 8.3k web page.
 c) executeHTTPDTest.py is the script that executes httpd web server with insturmentation, and writes the results to a file.

2) inputs/pbzip
 a) executePBZIPTest.py is the script that executes pbzip compressor with insturmentation, and writes the results to a file.
 b) example.txt is the file that is used as the input to the pbzip2 compression tool.
 c) generator.py is the script that is used to generate some other input files for different tests.
                    
3) inputs/parsec
  a) As already mentioned, we do not include the inputs for parsec 3.0, which are already freely avalible on the web "parsec.cs.princeton.edu/download.htm".
  b) How to run parsec experiments:
   i) Firstly, set the directories for parsec and pin executable from the first two lines of the script. (PARSEC_DIR and PIN_EXECUTABLE variables)
   ii) Add the following lines to the for loop on the 296th line of the script:
   
     For lockset implementataion: example.executeOurImpelemtation("purelocset", "PureLocksetImp.so",{} , "purelockset_")
     For hb implementataion: example.executeOurImpelemtation("purehb", "PureHappensBeforeImp.so",{}, "purehb_")
     For hybrid implementataion with no optimizations enabled: example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{} , "tsan_1_0")
   iii) How to execute segment based hybird implementation with optimizations:
      Add optimizations with values to the "executeOurImpelemtation" call as the 3rd input.
      
      Optimization 1 :maxVCHistoryCount
      Optimization 2: enable_signle_access
      Optimization 3: segmentCountForThisExecution (this must be given relative to total segment count in the original execution. Refer to the paper.)
      Optimization 4: sample_rate
   

			example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{ "enable_signle_access" : ""}, "tsan_131072_1")
            example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"maxVCHistoryCount" :100] } , "tsan_1_0")
			example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"sample_rage" :0.1] } , "tsan_1_0")
			executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"max_single_segment_set_size":12000 } , "tsan_1_0")
			or combinations:
			example.executeOurImpelemtation("hybrid", "SegmentBasedHybridImp.so",{"max_single_segment_set_size":segmentCountForThisExecution, "enable_signle_access" : "", "sample_rate" :str(SAMPLE_RATES[counter]) } , "tsan_1_0")
		

  c)How to get output
     Fourth parameter given to "executeOurImpelemtation" is the file where the outputs are written for each application.
     Moreover, in the end, a file with name format "HH:MM:SS_DD_MM_YYYY_execution_history.txt" is written, which includes all the test results executed with "executeOurImpelemtation" function call. This file is an XML file, which is easy to read and understand.
     ie:
     ```xml
     	<OWN>
		  <RACE_COUNT>0</RACE_COUNT>
		  <ALGORITHM>hybrid</ALGORITHM>
		  <maxVCHistoryCount>10</maxVCHistoryCount>
		  <APP_NAME>swaptions</APP_NAME>
	  	  <segment_history_table_size>63.6363636364</segment_history_table_size>
		  <TIME>70.6933810711</TIME>
		  <INPUT_TYPE>simsmall</INPUT_TYPE>
		  <enable_signle_access/>
	    </OWN>
       ```
  
  
  
===========         

