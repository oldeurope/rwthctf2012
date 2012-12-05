#!/usr/bin/env ruby
require File.expand_path("../ctf.rb", __FILE__)

COLORS = {
	:status => ["FFFFFF", "E6ACAC", "E8C6A2", "ACACE6"],
	:fail => "FF4D79",
	:win => "00AA00"
}

MASTER = "Gameserver (Master)"

class Team
	def path()
		@name.gsub("/", "").gsub(".", "")
	end
end

class Flag
	def path()
		[ @team.path, "services", @service, "flags", @flag ].join("/")
	end

	def lost_path()
		[ @team.path, "services", @service, "lost_flags", @flag ].join("/")
	end
end

class GourceEvent
	def initialize(username, type, file, color=nil, timestamp=Time.now)
		@username = username
		@type = type
		@file = file
		@color = color
		@timestamp = timestamp
	end

	def to_s()
		s = [@timestamp.to_i, @username, @type, @file].join("|")
		s += "|" + @color if @color
		return s
	end

	def dispatch()
		puts self.to_s
	end
end

class EventDispatcher
	# The gameserver put'ed a flag
	def self.put(slave, flag, status, timestamp=Time.now)
		if status == 0
			GourceEvent.new(slave, "A", flag.path, COLORS[:status][status], timestamp).dispatch()
		end
		GourceEvent.new(MASTER, "A", "Gameserver/" + flag.service + "/" + flag.flag + "_" + slave + ".put", COLORS[:status][status], timestamp).dispatch()
	end

	# The gameserver get'ed a flag
	def self.get(slave, flag, status, timestamp=Time.now)
		GourceEvent.new(slave, "M", flag.path, COLORS[:status][status], timestamp).dispatch()
		GourceEvent.new(MASTER, "M", "Gameserver/" + flag.service + "/" + flag.flag + "_" + slave + ".get", COLORS[:status][status], timestamp).dispatch()
	end

	# A team successfully submitted another team's flag
	def self.submit(submitter_ip, flag, timestamp=Time.now)
		team = Team.new(submitter_ip.split(".")[2].to_i)
		GourceEvent.new(team.to_s, "M", flag.lost_path + "_captured_by.+" + team.path + " - win", COLORS[:fail], timestamp).dispatch()
		GourceEvent.new(team.to_s, "A", team.path + "/captured_flags/" + flag.service + "_" + flag.flag + ".-" + flag.team.path + " - fail", COLORS[:win], timestamp).dispatch()
	end
end
