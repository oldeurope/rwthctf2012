#!/usr/bin/python

import string
import random
import sys
import hashlib
import socket
import array

TIMEOUT_CONNECT = 20
TIMEOUT_RECV = 5

###functions

def recvt(s,keyword):
	buf = ""
	while not keyword in buf:
		tmp = s.recv(1)
		if len(tmp) == 0:
			raise Exception("recvt failed")
		buf+=tmp
	return buf


#generae random string
def randomString(l):
	return ''.join(random.choice(string.ascii_uppercase + string.digits) for x in range(l))

#walk randomly through the map
def randomWalk(s,l):
	buf = array.array('l')
	a= ['n','ne','nw','s','se','sw','e','w']
	for i in range(l):
		r = random.randint(0,7)

		s.send(a[r] + '\n')
		ret = recvt(s,"> ")

		if "You can't go that way!" not in ret:
			buf.append(r)
	
	buf.reverse()
	return buf

# walk walk a given way
def backWalk(s,l):
	a = ['s','sw','se','n','nw','ne','w','e']
	for r in l:
		s.send(a[r] + '\n')
		recvt(s,"> ")

# 
def entergame(s):
	recvt(s,"> ")
	n = randomString(random.randint(1,26))
	s.send(n + "\n")
	ret = recvt(s,"> ")
	if n not in ret:
		raise Exception("entering name failed")

#
def walking(s):
	nr_of_walks = random.randint(2,6)
	w = randomWalk(s,nr_of_walks)

	s.send('look\n')
	recva = recvt(s,"> ")

	walks = randomWalk(s,nr_of_walks)
	backWalk(s,walks)

	s.send('look\n')
	recvb = recvt(s,"> ")

	if recva != recvb:
		raise Exception("walking check failed")
		
	backWalk(s,w)

	s.send('look\n')
	recvt(s,"> ") 

#
def formatcheck(s):
	s.send("e\nne\nnw\ntake power supply\nse\nsw\nw\nuse power supply on computer\ne\nne\nne\nformat\n")
	recvt(s,"Required parameter missing -")
	recvt(s,"> ")

	#tryout format

	s.send("format "+randomString(1)+":\nY\n")
	recva =  recvt(s,"> ")
		
	if not "HAHAHAA you got your Revenge" in recva:
		raise Exception("format check failed")

#
def wingame(s):
	s.send("sw\nn\ntake friend\nn\ntake bucket\nuse bucket on friend\ns\ns\nsw\ns\ne\nn\ntake gf\nuse gf on mattress\ns\nw\nn\nne\nse\ntake shoes\nuse shoes on gf\n")
	recva = recvt(s,"end.")
	if not "end." in recva:
		raise Exception("win game check failed")


##########################		Error checking
##########################
if len(sys.argv) != 4:
	print("Wrong number of arguments.")
	sys.exit(3)


##########################		Actual checking
##########################
mode = sys.argv[1]
ip = sys.argv[2]
flag = sys.argv[3]
flagkey = hashlib.md5(flag).hexdigest()[:16]
port = 12345



##########################		init socket
##########################
try:
	s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
	s.settimeout(TIMEOUT_CONNECT)
	s.connect((ip,port))
	s.settimeout(TIMEOUT_RECV)
except:
	print "service down"
	sys.exit(1)


if mode == "put":

	# check name
	try:
		entergame(s)
	except:
		print "name check failed - or premature close"
		sys.exit(2)


	# check random walking

	try:
		walking(s)
	except:
		print "walking check failed"
		sys.exit(2)


	# check format

	try:
		formatcheck(s)
	except:
		print "format check failed"
		sys.exit(2)


	#win game
	if random.randint(0,4) == 4:
		try:
			wingame(s)		
		except:
			print "error in winning game"
			sys.exit(2)


	# Put Flag

	#start with fresh connection
	try:
		s.close()
		s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
		s.settimeout(TIMEOUT_CONNECT)
		s.connect((ip,port))
		s.settimeout(TIMEOUT_RECV)
	except:
		print "service down"
		sys.exit(1)

	try:
		entergame(s)

		s.send("e\nne\ne\ntake book\nwrite page " + str(int(flagkey,16)) + " " + flag+"\n")
		recvt(s,"of the book was added to your inventory")
		recvt(s,"> ")
		if random.randint(0,1) == 0:
			s.send("hide page 1\n")
			recva = recvt(s,"> ")
			if "Perfect," not in recva:
				raise Exception("putting flag failed")
		
		else:
			s.send("write-on page " + str(random.randint(0,99999999)) +" "+ randomString(20) + "\n")
			recvt(s,"> ")
			s.send("hide pages 1 to 2\n")
			recva = recvt(s,"> ")
			if "Perfect," not in recva:
				raise Exception("putting flag failed")			

		s.send("exit\n")
		s.close()

	except:
		print "putting flag failed"
		sys.exit(2)


elif mode == "get":

	# Get Flag

	try:
		entergame(s)

		s.send("e\nne\ne\nsearch " + str(int(flagkey,16))+"\n")
		recvt(s,"foreign secrets")
		recvt(s,"> ")
		recva = recvt(s,"> ")
		if flag not in recva:
			raise Exception("receiving flag failed")

		s.send("exit\n")
		s.close()

	except:
		print "receiving flag failed"
		sys.exit(2)

else:
	print("Invalid mode " + mode)
	sys.exit(3)


sys.exit(0)
