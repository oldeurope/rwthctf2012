#!/usr/bin/env ruby
require 'socket'
require 'json'

class Gibson
	def initialize(host, port)
		@sock = UDPSocket.new
		@host = host
		@port = port
	end

	def emit(data)
		send(data.to_json)
	end

	private
	def send(data)
		@sock.send(data, UDPSocket::NONBLOCK, @host, @port)
	end
end
