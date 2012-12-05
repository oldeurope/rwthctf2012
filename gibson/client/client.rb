#!/usr/bin/env ruby
require File.expand_path("../connection.rb", __FILE__)
require File.expand_path("../db.rb", __FILE__)

if ARGV.length < 2 or ARGV.length > 4
	STDERR.puts "Usage: #{$0} <server> <port> [<db server> <db port>]"
	exit
end

ARGV[2] = ARGV[0] unless ARGV[2]
ARGV[3] = ARGV[1] unless ARGV[3]

db = DB.new(ARGV[2], ARGV[3])
Team.set_teams(db.get_teams())

conn = EventConnection.new(ARGV[0], ARGV[1])

begin
	conn.each_event do |event|
		event["timestamp"] = Time.now.to_i unless event["timestamp"]
		STDERR.puts "raw: " + event.to_json

		next if event["service"] == "ping"

		if event["slave"].class == Fixnum then
			event["slave"] = "Gameserver (Slave #{event["slave"]})"
		end

		team = Team.new(event["team_id"])
		flag = Flag.new(event["flag"], team, event["service"])

		if event["type"] == "put" then
			EventDispatcher.put(event["slave"], flag, event["status"], event["timestamp"])
		elsif event["type"] == "get" then
			EventDispatcher.get(event["slave"], flag, event["status"], event["timestamp"])
		elsif event["type"] == "submit" then
			EventDispatcher.submit(event["submitter"], flag, event["timestamp"])
		end
	end
rescue Interrupt
	exit 0
end
