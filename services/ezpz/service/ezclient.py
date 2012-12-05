#!/usr/bin/python
# -*- coding: utf8 -*-

import os
import sys
import random
import string
import socket
import pack

BUFSIZ = 16 * 1024
EZ_REQ = 0
EZ_RES = 1

class Broken(Exception):
	pass

class ezclient(object):
	def __init__(self, ip):
		self.ip = ip
		self.s = None
		self.u = pack.U()
		self.p = pack.P()

	def write(self, data):
		self.s.sendall(data)

	def connect(self):
		self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.s.connect((self.ip, 65432))
		self.msgid = 1
		self.write('ezpzrpc client\n')
		self.recvbanner()

	def recvbanner(self):
		buf = ''
		while not '\n' in buf:
			tmp = self.s.recv(BUFSIZ)
			if not tmp: raise Broken('recv banner fail')
			buf += tmp

		buf, rest = buf.split('\n', 1)
		if not 'ezpzrpc' in buf: raise Broken('banner fail')
		self.u.feed(rest)

	def getresponse(self, emsg):
		gotmsg = False
		buflen = 0
		result = None
		while not gotmsg:
			try: d = self.s.recv(BUFSIZ)
			except: raise Broken('recv fail')
			if not d: raise Broken('premature close?')
			self.u.feed(d)
			buflen += len(d)
			if buflen >= BUFSIZ * 10: raise Broken('too much data')
			for msg in self.u:
				if not type(msg) in (list, tuple) or len(msg) < 4: raise Broken('bad protocol msg')
				gotmsg = True
				if msg[0] != EZ_RES: raise Broken('msg is not a response')
				rmsgid, error, result = msg[1:4]
				if rmsgid == -1: print '-1:', error
				if rmsgid != self.msgid: raise Broken('msgid failure')
				if error: raise Broken(error)
				break

		return result

	def call(self, fn, *params):
		req = [EZ_REQ, self.msgid, fn, params, 'foobar']
		self.write(self.p.pack(req))
		r = self.getresponse(fn)
		self.msgid += 1
		return r

def main():
	ip = sys.argv[1]
	cli = ezclient(ip)
	cli.connect()
	print '-> hello("ezclient")'
	r = cli.call('hello', 'ezclient')
	print ' <-', r

	return 0

if __name__ == '__main__':
	try:
		sys.exit(main())
	except KeyboardInterrupt:
		pass
