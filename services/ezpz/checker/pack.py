#!/usr/bin/python
# -*- coding: utf8 -*-

from msgpack import Packer, Unpacker

class U(Unpacker):
	def __init__(self):
		Unpacker.__init__(self, object_hook=self.hook)

	def hook(self, obj):
		print 'unpacked obj, custom?', obj
		return obj

class P(Packer):
	def __init__(self):
		Packer.__init__(self, default=self.default)

	def default(self, obj):
		print 'packing unknown obj', obj
		return str(default)