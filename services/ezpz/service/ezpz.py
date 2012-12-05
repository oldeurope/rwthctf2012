#!/usr/bin/python
# -*- coding: utf8 -*-

import os
import sys
import gevent, gevent.server
import random
import tempfile
import rwthctfstore.btree as rage
import proto
import ezprocess
import nacl

EZPZ_ROOT = os.path.abspath(os.path.dirname(__file__))
EZPZ_PORT = 65432

elog = open('./log/ezlog', 'w')

class ezpzsrv(object):
	def __init__(self, obj):
		self.object = obj
		self.listen(port=EZPZ_PORT)

	def listen(self, host='', port=0):
		self.listener = gevent.server.StreamServer((host, port), self.handle)
		self.listener.start()
		print 'server started.'

	def handle(self, sock, addr):
		tc = proto.TeamConn(sock, addr, self)
		tc.handle()

	def run(self):
		print 'serving...'
		try: self.listener.serve_forever()
		except KeyboardInterrupt: 'exiting on ctrl+c.'

	latest = property(lambda self: self.object.latest)
	store = property(lambda self: self.object.store)

def expose(fn):
	fn.exposed = fn.__name__
	return fn

class ezfunctions(object):
	def __init__(self):
		self.latest = set()
		self.pub, self.priv = nacl.crypto_sign_keypair()
		self.g1 = gevent.spawn(self.flush)

	def flush(self):
		while True:
			elog.flush()
			gevent.sleep(5)

	@expose
	def admin_check(self, kw, count, conn):
		# authorization
		if conn.pubkey == self.pub:
			return self.store.fwmkeys(kw, count)
		else:
			return 'Denied.'

	@expose
	def hello(self, msg, msg2=''):
		return 'hello {0}{1}'.format(msg, msg2)

	@expose
	def list(self, kw):
		r = self.store.fwmkeys(kw, 50)
		return random.sample(self.store.fwmkeys(kw), min(20, len(r)))

	@expose
	def get(self, kw, kw2=None):
		return self.store.get(kw2 or 'item_{0}'.format(kw))

	# careful here, needed for gameserver as well
	@expose
	def evaluate(self, data):
		tf = tempfile.NamedTemporaryFile(prefix='ezpz', suffix='.py', dir='./code', delete=False)
		tf.write(data)
		tf.close()

		ep = ezprocess.ezprocess(tf.name, self)
		try: ep.run()
		except:
			import traceback
			traceback.print_exc()
			return 'eval fail'

		return ep.output, ep.errput

	@expose
	def put(self, k, v):
		self.store.put('item_{0}'.format(k), str(v))
		print >>elog, 'item_{0}'.format(k), '->', str(v)
		self.keeptrack(v)
		return True

	@expose
	def register(self, pk, conn):
		conn.pubkey = pk
		return True

	@expose
	def whois(self):
		return self.store.get('pub').encode('hex')

	def getlatest(self):
		return list(self.latest)

	def keeptrack(self, obj):
		self.latest.add(obj)
		while len(self.latest) > 10:
			self.latest.pop()

	def find(self, kw):
		return self.store.fwmkeys(kw, 50)

	def initstore(self):
		self.store = rage.BTree()
		self.store.open('./ezdata', rage.BDBOWRITER | rage.BDBOCREAT)
		self.store['priv'] = self.priv
		self.store['pub'] = self.pub

def main():
	if not os.path.exists('./code'):
		os.mkdir('./code')

	obj = ezfunctions()
	obj.initstore()
	s = ezpzsrv(obj)
	s.run()
	return 0

if __name__ == '__main__':
	sys.exit(main())
