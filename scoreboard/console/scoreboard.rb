#!/usr/bin/env ruby
# rwthctf2012 fancy console scoreboard (fw)
require 'open-uri'
require 'json'
require 'colorize'

RANDOM = false

MARGIN_TOP   = 2
MARGIN_LEFT  = 5
SERVICE_CELL = " " * 3

API = "http://10.12.250.1/json/score"

COLORS = [ :on_green, :on_red, :on_yellow, :on_blue, :to_s ]
ANSI_CLEAR = "\e[H\e[2J"

class Team
	def initialize(hash)
		@hash = hash
		@@total['ap'] += hash['ap']
		@@total['dp'] += hash['dp']
		@@max['ap'] = [ @@max['ap'], hash['ap'] ].max
		@@max['dp'] = [ @@max['dp'], hash['dp'] ].max
		@@name_max_len = [ hash['name'].length, @@name_max_len ].max
	end

	def score(type=nil)
		if type then
			return 1 if @@total[type] == 0
			return @hash[type].to_f / @@total[type].to_f
		end
		return (score("ap") + score("dp")) / 2.0
	end

	def scorep(width, type=nil)
		s = "%#{width}.2f%%" % (100.0 * score(type))
		if type and @hash[type] == @@max[type] then
			s = s.green
		end
		return s
	end

	def method_missing(m, *args)
		@hash.fetch(m.to_s)
	end

	def state(service)
		return rand 4 if RANDOM
		return 4 unless @hash['states'] and @hash['states'][service]
		return @hash['states'][service]['state']
	end

	def colored_state(service, str)
		COLORS.map{ |m| str.send(m) }[state(service)]
	end

	def self.width()
		@@name_max_len
	end

	def self.reset()
		@@total = { "ap" => 0, "dp" => 0 }
		@@max = { "ap" => 0, "dp" => 0 }
		@@name_max_len = 0
	end
end

class Table
	def initialize(scores, services)
		@teams = scores.map{ |team| Team.new(team) }.sort_by{ |team| (-10000.0 * team.score).to_i }
		@services = services
	end

	def th()
		r = ""
		services_max_len = @services.map{ |s| s.length }.max
		(0..services_max_len-1).each do |i|
			r += "  " + " " * MARGIN_LEFT
			r += "%#{Team.width}s   " % (i == services_max_len-1 ? "Team" : "")
			@services.each do |service|
				r += " " + service.rjust(services_max_len)[i] + " "
			end
			r += "\n" if i < services_max_len-1
		end
		r += "   " + [ "Offense", "Defense", " Score " ].join("   ")
		return r
	end

	def tr(team, hilight=nil)
		name = team.name.rjust(Team.width)
		if team.id == hilight then
			name = name.green
		end
		r = (" " * MARGIN_LEFT) + "| #{name} | "
		@services.each do |service|
			r += team.colored_state(service, SERVICE_CELL)
		end
		r += [ "", team.scorep(6, "ap"), team.scorep(6, "dp"), team.scorep(6), "" ].join(" | ")
		return r
	end

	def hr()
		(" " * MARGIN_LEFT) + "+-" + ("-" * Team.width) + "-+" + ("-" * (2 + @services.length * SERVICE_CELL.length)) + ("+"+("-"*9))*3 + "+"
	end

	def render(lines=nil, hilight)
		lines = @teams.length unless lines
		([ th(), hr() ] + @teams[0,lines].map{ |team| tr(team, hilight) } + [ hr() ]).join("\n")
	end
end

class Scoreboard
	def initialize(url=API)
		@url = url
		update()
	end

	def update()
		Team.reset()
		begin
			json = open(@url).read
			data = JSON.parse(json)
			scores = data['scores']
			services = data['services'].map{ |s| s['name'] }
			@table = Table.new(scores, services)
		rescue => e
			if $logger then
				$logger.error(e.inspect)
			else
				STDERR.puts(e.inspect)
			end
			exit if @table == nil
		end
	end

	def render(lines=nil, team=nil)
		ANSI_CLEAR + ("\n" * MARGIN_TOP) + @table.render(lines, team)
	end

	def width()
		@table.hr().length()
	end
end

if __FILE__ == $0
	puts Scoreboard.new().render(ARGV[0] ? ARGV[0].to_i : 20)
end
