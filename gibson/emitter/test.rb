#!/usr/bin/env ruby
require './gibson.rb'

g = Gibson.new(ARGV[0], ARGV[1])

attacker = [*0..10]
teams = [*1..35]
services = [*1..9].map{ |i| "service#{i}" }
slaves = [*1..9].map{ |i| "Gameserver (Slave #{i})" }
status = [0,1,2,3]
types = ["put", "get", "submit"]

class Array
	def rand()
		self[Kernel.rand(self.length)]
	end
end

flags = []

loop do
	type = types.rand

	data = {
		"type" => type,
		"team_id" => teams.rand,
		"service" => services.rand
	}

	if type == "put" then
		data['status'] = status.rand
		data['slave'] = slaves.rand
		data['flag'] = (1..10).map{ "%02x" % rand(256) }.join
		if data['status'] == 0
			flags << [ data['flag'], data['team_id'], data['service'] ]
		end
	elsif type == "get" then
		next if flags == []
		foo = flags.pop
		data['slave'] = slaves.rand
		data['status'] = status.rand
		data['flag'] = foo[0]
		data['team_id'] = foo[1]
		data['service'] = foo[2]
	elsif type == "submit" then
		next if flags == []
		foo = flags.rand
		data['flag'] = foo[0]
		data['team_id'] = foo[1]
		data['service'] = foo[2]
		data["submitter"] = "10.12.%d.%d" % [ attacker.rand, 100 + rand(100) ]
	end

	p data
	g.emit(data)
	sleep(rand() / (ARGV[2] || 1).to_f)
end
