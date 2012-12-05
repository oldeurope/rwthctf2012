#!/usr/bin/env ruby
require 'socket'
require 'json'

class EventConnection
	def initialize(host, port)
		@sock = TCPSocket.open(host, port)
	end

	def each_event(&block)
		while line = @sock.gets
			block.yield(JSON.parse(line))
		end
	end
end
