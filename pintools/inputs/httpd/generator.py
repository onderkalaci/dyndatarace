#!/usr/bin/python

import sys,getopt,threading,time
import urllib2



threadcount = ''
connectioncount = ''

def argParser(argv):
	global connectioncount, threadcount
	try:
		opts, args = getopt.getopt(argv,"hn:c:",["threadcount=","connectioncount="])
	except getopt.GetoptError:
		print 'test.py -n <threadcount> -c <connectioncount>'
		sys.exit(2)
	for opt, arg in opts:
		print opt, arg
		if opt == '-h':
			print 'test.py -n <threadcount> -c <connectioncount>'
			sys.exit()
		elif opt in ("-n", "--threadcount"):
			threadcount = arg
		elif opt in ("-c", "--connectioncount"):
			connectioncount = arg
	

class Connector(threading.Thread):
	def __init__(self, connectionCount):
		threading.Thread.__init__(self)
		self.connectioncount = connectionCount
	
	def run(self):
		try:
			for i in range(0, int(self.connectioncount)):
				time.sleep(0.1)
				req = urllib2.Request('http://127.0.0.1')
				r = urllib2.urlopen(req)
				if len(r.read()) != 8484:
					print "OOOPS error"
		except Exception,e:
			print "%s",e
if __name__ == "__main__":
	argParser(sys.argv[1:])
	print 'threadcount', threadcount
	print 'connectioncount', connectioncount
	allConnectors = []
	for i in range(0, int(threadcount)):
		tmp = Connector(connectioncount)
		allConnectors.append(tmp)
	
	for connector in allConnectors:
		connector.start()
