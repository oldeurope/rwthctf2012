#!/usr/bin/env ruby
require 'eventmachine'
require 'logger'
require File.expand_path("../scoreboard.rb", __FILE__)

TIMER = 5

host = "0.0.0.0"
port = (ARGV[0].to_i > 0) ? ARGV[0].to_i : 12345

$logger = Logger.new(STDOUT)

module ScoreboardServer
	def initialize(scoreboard)
		@peer = Socket.unpack_sockaddr_in(get_peername)
		@team = @peer[1].split(".")[2].to_i
		@peer = @peer.reverse.join(":")
		$logger.info("#{@peer} connected")
		hello()
		@buf = ""
		@scoreboard = scoreboard
	end

	def unbind()
		$logger.info("#{@peer} disconnected")
	end

	def receive_data(data)
		@buf += data
		return unless idx = @buf.index("\n")
		@lines = @buf[0,idx].strip.to_i
		if @lines <= 0
			$logger.warn("#{@peer} entered invalid line number (#{@buf[0,idx].inspect})")
			close_connection()
			return
		end
		refresh()
		EventMachine::PeriodicTimer.new(TIMER) do
			refresh()
		end
	end

	def refresh()
		send(@scoreboard.render(@lines, @team))
		fill = @scoreboard.width()
		str = "Auto-update: #{TIMER} seconds"
		fill -= str.length
		send(" "*fill + str)
	end

	def send(data)
		send_data(data + "\n")
	end

	def hello()
		send("Welcome to the RWTH CTF 2012 Scoreboard")
		send("How many teams do you want to see?")
	end
end

EventMachine::run do
	scoreboard = Scoreboard.new()
	EventMachine::PeriodicTimer.new(TIMER) { scoreboard.update() }
	EventMachine::start_server(host, port, ScoreboardServer, scoreboard)
	$logger.info("Listening on #{host}:#{port}")
end
