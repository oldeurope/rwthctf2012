#!/usr/bin/env ruby

class Team
	attr_reader :id, :name
	@@teams = []

	def initialize(id)
		@id = id.to_i
		@name = Team.get_team_name(id)
	end

	def to_s()
		@name
	end

	def self.get_team_name(id)
		@@teams[id] || "Team #{id}"
	end

	def self.set_teams(teams)
		@@teams = teams
	end
end

class Flag
	attr_reader :flag, :team, :service

	def initialize(flag, team=nil, service=nil)
		@flag = flag
		@team = team
		@service = service
	end
end
