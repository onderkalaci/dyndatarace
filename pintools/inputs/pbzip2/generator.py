#!/usr/bin/python


import string,random
from UserString import MutableString
def generateRandomFile(size):
	return ''.join(random.choice(string.ascii_uppercase + string.digits + string.ascii_lowercase) for x in range(size))
	
strToFile = MutableString()	
for i in range(0,10000):
	print i
	strToFile = strToFile +  generateRandomFile(10000)

f = open("pbzip_test_data.txt" , "w")
f.write(str(strToFile))
f.close()
