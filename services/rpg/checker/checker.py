import gameapy
import string
import random
import re
import sys
import hashlib
import socket

if len(sys.argv) != 4:
	print("Wrong number of arguments.")
	sys.exit(3)

mode = sys.argv[1]
ip = sys.argv[2]
flag = sys.argv[3]
flagkey = hashlib.md5(flag).hexdigest()[:12]

def randomString(l):
	return ''.join(random.choice(string.ascii_uppercase + string.digits) for x in range(l))

def gotoNPC(g, s):
	try:
		g.navigateToNPC(s)
	except gameapy.DungeonLogicError:
		print("NPC " + s + " was not found")
		sys.exit(2)


# this is a simple example how to use the api
try:
	try:
		sys.stderr.write("Connecting to service...")
		sys.stderr.flush()
		g = gameapy.DungeonSession(random.randint(40,70), random.randint(60,100))
		g.connectTo(ip, 8080)
		sys.stderr.write("OK\n")
	except socket.timeout:
		print("Connect timed out - service down")
		sys.exit(1)
	except socket.error:
		print("Could not connect - service down")
		sys.exit(1)
	except gameapy.DungeonTimeout:
		print("Gameboard not sent by server (timeout)")
		sys.exit(2)
	except gameapy.DungeonLogicError as e:
		print("Cannot parse gameboard.")
		sys.stderr.write("Error: {}".format(e) + "\nBoard was:\n"+str(e.board))
		sys.exit(2)


	if mode == "put":
		##### Vendor #####
		sys.stderr.write("testing vendor...")
		sys.stderr.flush()
		try:
			gotoNPC(g, "V")
			g.readNPCText()
			g.choiceNPCMCOption(1)
			s = g.readNPCText()
			if re.search("0x[0-9a-f]+.+0x[0-9a-f]+", s) == None:
				print("Vendor is broken")
				sys.stderr.write("FAIL\n")
				sys.exit(2)
		except gameapy.DungeonTimeout as e:
			print("Vendor NPC broken")
			sys.stderr.write("FAIL\n")
			sys.stderr.write("Testing vendor: {}".format(e))
			sys.exit(2)
		sys.stderr.write("OK\n")

		##### Parrot #####
		sys.stderr.write("testing parrot...")
		sys.stderr.flush()
		try:
			gotoNPC(g, "P")
			g.readNPCText()
			query = randomString(20)
			g.enterNPCText(query)
			g.readTextUntilCursorHome()
			res = g.readTextUntilCursorHome().strip()
			if res != query:
				print("Parrot is broken")
				sys.stderr.write("FAIL\n")
				sys.exit(2)
		except gameapy.DungeonTimeout as e:
			print("Parrot broken")
			sys.stderr.write("FAIL\n")
			sys.stderr.write("Testing parrot: {}".format(e))
			sys.exit(2)
		sys.stderr.write("OK\n")

		##### Librarian PUT #####
		try:
			sys.stderr.write("testing librarian...")
			sys.stderr.flush()
			gotoNPC(g, "L")
			g.readNPCText()
			g.choiceNPCMCOption(0)

			g.enterNPCText(flagkey)
			g.enterNPCText(flag)
			g.readNPCText()
		except gameapy.DungeonTimeout as e:
			print("Librarian broken")
			sys.stderr.write("FAIL\n")
			sys.stderr.write("Testing librarian: {}".format(e))
			sys.exit(2)
		sys.stderr.write("OK\n")

	elif mode == "get":
		##### Librarian GET #####
		sys.stderr.write("testing librarian...")
		sys.stderr.flush()
		libPubPosRe = re.compile("Meticulous Librarian: Here is your\npublication:\n(\S+)")
		libNoPubRe = re.compile("Meticulous Librarian: Sorry, I was not able to\nfind this publication")
		try:
			gotoNPC(g, "L")
			g.readNPCText()
			g.choiceNPCMCOption(1)
			g.enterNPCText(flagkey)
			result = g.readNPCText()
		except gameapy.DungeonTimeout as e:
			print("Librarian broken")
			sys.stderr.write("FAIL\n")
			sys.stderr.write("Testing librarian: {}".format(e))
			sys.exit(2)
		sys.stderr.write("OK\nVerifying flag...\n")

		m = libPubPosRe.search(result)
		if m:
			res = m.group(1)
			sys.stderr.write("result: " + res + "\n")
			if res != flag:
				sys.stderr.write("Wrong flag provided\n")
				print("Librarian does provide a wrong flag")
				sys.exit(2)
			sys.stderr.write("Flag OK\n")
		else:
			m = libNoPubRe.search(result)
			if m:
				sys.stderr.write("Key not available\n")
				print("Could not retrieve flag - key not available")
			else:
				sys.stderr.write("Unable to retrieve flag. Unmatched result was: {}\n".format(result))
				print("Could not retrieve flag - librarian broken?")
			sys.exit(2)
	else:
		print("Invalid mode " + mode)
		sys.exit(3)
except EOFError as e:
	print("Server ended connection unexpectedly")
	sys.stderr.write("Server closed connection: {}".format(e))
	sys.exit(2)

g.quit()

sys.exit(0)
