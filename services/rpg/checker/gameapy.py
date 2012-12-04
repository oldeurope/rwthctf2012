import socket
import telnetlib
import struct
import re
import sys

colorcleanRe = re.compile("\x1B\[[^m]+m")
textcleanRe = re.compile("\x1B\[[0-9;]*[HDJmK]|\(press any key to continue\)\n?")
textNaviRe = re.compile("\x1B\[[0-9]+K")

TIMEOUT = 10
HOMELINEOFF = 1

class DungeonTimeout(Exception): pass
class DungeonLogicError(Exception): 
	def __init__(self, msg, board = None):
		Exception.__init__(self, msg)
		self.board = board

class DungeonSession(object):
	def __init__(self, termHeight, termWidth):
		self.termHeight = termHeight
		self.termWidth = termWidth
		self.currX = 1
		self.currY = 1

	def connectTo(self, host, port):
		self.con = telnetlib.Telnet()
		self.con.set_option_negotiation_callback(lambda s, c, o : self.__telnetOptionCB(s,c,o))
		self.con.open(host,port,TIMEOUT)
		data = self.con.read_until("#",TIMEOUT)
		if not data.endswith("#"):
			raise DungeonTimeout("Waiting for board begin")

		# writing the gameboard is followed by statusbar, indicated by VT100 redir
		boarddata =  "#" + self.con.read_until("\x1B[1;1H",TIMEOUT)
		if not boarddata.endswith("\x1B[1;1H"):
			raise DungeonTimeout("Waiting for board end")
		self.board = DungeonBoard(boarddata)
#		self.board.dumpBoard()

	def quit(self):
		self.con.write("\x04")
		self.con.close()

	def __telnetOptionCB(self, sock, cmd, opt):
		if cmd == telnetlib.WILL:
			if opt == "\x01":
				# send our negotiation data
				sock.send(telnetlib.IAC + telnetlib.DO + "\x01") # do echo
				sock.send(telnetlib.IAC + telnetlib.WILL + "\x1F") # will term size
		elif cmd == telnetlib.DO:
			if opt == "\x1F":
				# do term size
				termStr = struct.pack(">HH", self.termWidth, self.termHeight)
				sock.send(telnetlib.IAC + telnetlib.SB + "\x1F" + termStr + telnetlib.IAC + telnetlib.SE)

#		print("sock {}, cmd {}, opt {}".format(sock,cmd,opt))

	def navigateToNPC(self, npcchar):
		# find npc
		try:
			npcX, npcY = self.board.getNPCPos(npcchar)
		except ValueError:
			raise DungeonLogicError("NPC not found")
		# compute the route to npc
		route = self.board.navigationSequence(self.currX, self.currY, npcX, npcY)
		self.con.write(route)
		# apply first part of route to internal pos, don't apply last one, its not executed but used to activate npc
		for char in route[:-1]:
			if char == "w":
				self.currY = self.currY - 1
			if char == "a":
				self.currX = self.currX - 1
			if char == "s":
				self.currY = self.currY + 1
			if char == "d":
				self.currX = self.currX + 1

	def readNPCText(self, clean = 1):
		clearTextRe = re.compile("\x1B\[{};1H\x1B\[J".format(self.board.height+HOMELINEOFF+1))
		inputRe = re.compile("\x1B\[\?25h")
		continueRe = re.compile("\(press any key to continue\)")
		text = ""
		while 1:
			i, match, t = self.con.expect([clearTextRe, inputRe, continueRe], TIMEOUT)
			text = text + t
#			print("re {}".format(i))
			if i == -1:
				raise DungeonTimeout("Waiting for npc text\nCurrent buffer:" + text)
			if i == 2:
				self.con.write("\n")
			else:
				break
		if clean == 1:
			return self.cleanText(text)
		else:
			return text

	def readTextUntilCursorHome(self, clean = 1):
		homestr  = "\x1B[{};1H".format(self.board.height+HOMELINEOFF+1)
		text = self.con.read_until(homestr,TIMEOUT)
		if not text.endswith(homestr):
			raise DungeonTimeout("Waiting for cursor home")
		if clean == 1:
			return self.cleanText(text)
		else:
			return text

	def cleanText(self, text):
		# make text processable 
		# first drop garbage 
		text = textcleanRe.sub("", textNaviRe.sub("\n",text))
#		print("cleaned")
#		print(text)
		# second: select only meaningful data
		ddotPos = text.find(":")
#		print("ddotpos {}".format(ddotPos))
		if ddotPos >= 0:
			# now find the newline before
			nlPos = text.rfind("\n",0, ddotPos)
#		print("nlPos {}".format(nlPos))
			if nlPos >= 0:
				text = text[nlPos+1:]
		return text

	def enterNPCText(self, text):
		self.con.write(text + "\r\x00")
		# drop all control stuff until cursor is hidden again
		res = self.con.read_until("\x1B[?25l", TIMEOUT)
		if not res.endswith("\x1B[?25l"):
			raise DungeonTimeout("Waiting for hide cursor")

	def choiceNPCMCOption(self, textoption):
		self.con.write("s" * (textoption) + "\r\x00")
		# drop until clean of textsection
		clearseq =  "\x1B[{};1H\x1B[J".format(self.board.height+HOMELINEOFF+1)
		res = self.con.read_until(clearseq)
		if res.find(clearseq) < 0:
			raise DungeonTimeout("Waiting for clear of mc input")

class DungeonBoard(object):
	def __init__(self, boarddata):
		boarddata = boarddata.replace("\r","")
		lines = [colorcleanRe.sub("",l) for l in boarddata.split("\n")[:-1]]
		self.width = len(lines[0])
		self.height = len(lines)
		row = 0
		try:
			self.board = []
			self.npclocs = {}
			for line in lines:
				colcont = []
				for col in range(0,self.width):
					colcont.append(line[col])
					if line[col] != "#" and line[col] != " ":
						self.npclocs[line[col]] = (col,row)
				self.board.append(colcont)
				row = row + 1
		except Exception as e:
			raise DungeonLogicError("Board parse error: " + str(e), "\n".join(lines))

	def getBoardElem(self, x, y):
		return self.board[y][x]

	def navigationSequence(self, fromX, fromY, toX, toY):
		# dijkstra can be implemented by BFS in our scenario
		reach = { (fromX, fromY) : ""}
		active = [ (fromX, fromY) ]
		while len(active) > 0:
			x,y = active[0]
			r = reach[(x,y)]
			# expand it and remove it
			del active[0]

			if x > 0:
				if not reach.has_key((x-1,y)) and self.getBoardElem(x-1,y) == " ":
					reach[(x-1,y)] = r + "a"
					active.append((x-1,y))
				if x-1 == toX and y == toY:
					reach[(x-1,y)] = r + "a"
					break
			if x < self.width:
				if not reach.has_key((x+1,y)) and self.getBoardElem(x+1,y) == " ":
					reach[(x+1,y)] = r + "d"
					active.append((x+1,y))
				if x+1 == toX and y == toY:
					reach[(x+1,y)] = r + "d"
					break
			if y > 0:
				if not reach.has_key((x,y-1)) and self.getBoardElem(x,y-1) == " ":
					reach[(x,y-1)] = r + "w"
					active.append((x,y-1))
				if x == toX and y-1 == toY:
					reach[(x,y-1)] = r + "w"
					break
			if y < self.height:
				if not reach.has_key((x,y+1)) and self.getBoardElem(x,y+1) == " ":
					reach[(x,y+1)] = r + "s"
					active.append((x,y+1))
				if x == toX and y+1 == toY:
					reach[(x,y+1)] = r + "s"
					break

		if reach.has_key((toX,toY)):
			return reach[(toX,toY)]
		else:
			raise DungeonLogicError("The route you requested is unfeasible")

	def getNPCPos(self, npcchar):
		return self.npclocs[npcchar.upper()]

	def dumpBoard(self):
		sys.stderr.write("Gameboard dimension: height {1}, width {0}\n".format(self.width, self.height))
		for row in self.board:
			sys.stderr.write("".join(row) + "\n")
