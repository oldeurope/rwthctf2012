#!/usr/bin/python
# -*- coding: utf8 -*-

import pack
import inspect
import nacl
BUFSIZ = 16 * 1024
EZ_REQ = 0
EZ_RES = 1
DEBUG = 1

class ezException(Exception):
	pass

class TeamConn(object):
	def __init__(self, sock, addr, ezpz):
		self.ezpz = ezpz
		self.sock, self.addr = sock, addr
		self.buf = ''
		self.pubkey = None
		self.p = pack.P()
		self.u = pack.U()

	def handle(self):
		try:
			self.negotiate()
			self.handle_ez()
		except ezException, e:
			if not 'Closed' in str(e):
				print 'exception on {0}: {1}'.format(self.addr[0], e)
				try: self.send_error(str(e))
				except: pass
		except Exception, e:
			import traceback
			traceback.print_exc()
			print 'exception on {0}: {1}'.format(self.addr[0], e)
		finally:
			try: self.sock.close()
			except: pass

	def send_error(self, e, msgid=-1):
		self.write(self.p.pack([EZ_RES, msgid, str(e), None]))

	def handle_ez(self):
		buf = self.buf
		p, u = self.p, self.u

		while True:
			tmp = self.read()
			if not tmp: break
			u.feed(tmp)
			for item in u:
				if not type(item) in (list, tuple) or len(item) < 5:
					raise ezException('Invalid request.')

				if item[0] == EZ_REQ:
					msgid, method, params, auth = item[1:]
					if method != 'register' and not self.verify(item):
						self.send_error('Verify failure.', msgid)
						continue

					if 'last' in params: params.append(self.ezpz.latest)

					fn = getattr(self.ezpz.object, method, None)
					if not callable(fn) or not fn or (not hasattr(fn, 'exposed') and not DEBUG):
						self.send_error('Fn fail: {0}'.format(method), msgid)
						continue

					argspec = inspect.getargspec(fn)
					try:
						if argspec.args and argspec.args[-1] == 'conn':
							r = fn(*params, conn=self)
						else:
							r = fn(*params)
					except TypeError:
						self.send_error('fn param fail', msgid)
						continue

					self.write(p.pack([EZ_RES, msgid, None, r]))
				elif item[1] == EZ_RES:
					self.send_error('Not supported in ezpz :(.')
				else:
					self.send_error('Invalid request opcode.')

	def negotiate(self):
		self.write('ohai (ezpzrpc) - send authenticated request dicts.\n')

		buf = self.read()
		while not '\n' in buf:
			buf += self.read()

		self.clientbanner, self.buf = buf.split('\n', 1)
		if not 'ezpzrpc client' in self.clientbanner: raise ezException('Banner failure.')

	def write(self, data):
		try:
			self.sock.sendall(data)
		except Exception as e:
			raise ezException('Couldnt write ({0}).'.format(e))

	def read(self):
		try:
			tmp = self.sock.recv(BUFSIZ)
		except Exception as e:
			raise ezException('Couldnt read ({0}).'.format(e))
		if not tmp: raise ezException('Closed.')
		return tmp

	def verify(self, item):
		msg, auth = item[:4], item[4]
		if not self.pubkey: return True
		hashed, should = "1", "2"
		try:
			data = self.p.pack(msg)
			should = nacl.crypto_sign_open(auth, self.pubkey)
			hashed = nacl.crypto_hash_sha256(data)
		except:
			#pass
			import traceback
			traceback.print_exc()

		if hashed == should or DEBUG:
			return True
		return False

	db = property(lambda self: self.ezpz.store)
