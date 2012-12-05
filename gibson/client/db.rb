#!/usr/bin/env ruby
require 'mongo'

class DB
	def initialize(host, port, db='ctf')
		@client = Mongo::Connection.new(host, port)
		@db = @client[db]
		@coll = @db['teams']
	end

	def get_teams()
		@coll.find({}, :fields => ["id", "name"]).to_a.reduce(Hash.new) do |hash,item|
			hash[item["id"].to_i] = item["name"]
			hash
		end
	end
end
