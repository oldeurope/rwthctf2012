#!/usr/bin/env ruby
require 'mongo'
require 'net/http'
require 'uri'
require 'json'
require 'wirble'
require 'pp'

GAMEDB=["10.12.250.1", 27017]
CUBEURL="http://10.12.250.42:1080/1.0/event/put"
CUBEINT=10

class DB
	def initialize(host, port, db='ctf')
		@client = Mongo::Connection.new(host, port)
		@db = @client[db]
		@runstats = @db['runstats']
		@captures = @db['captures']

		@last_captures = {}
		@last_capture_seen = 0

		@last_checker_errors = 0
		@last_checker_error_seen = 0
	end

	def get_stats()
		@db.stats()
	end

	def get_checker_errors()
		errs = @runstats.find({
			"retval" => 3,
			"timestamp" => {"$gt" => @last_checker_error_seen}
		}).count()

		@last_checker_error_seen = @runstats.find({retval:3},
			:fields => 'timestamp',
			:sort => [["timestamp", Mongo::DESCENDING]],
			:limit => 1
		).to_a.first['timestamp']

		@last_checker_errors += errs
	end

	def get_captures()
		c = @captures.group(
			:key => "service",
			:initial => {"sum" => 0},
			:reduce => "function(doc, prev) { prev.sum += 1}",
			:cond => {"time" => {"$gt" => @last_capture_seen}}
		)

		@last_capture_seen = @captures.find({},
			:fields => 'time',
			:sort => [["time", Mongo::DESCENDING]],
			:limit => 1
		).to_a.first['time']

		c.each do |i|
			@last_captures[i['service']] ||= 0
			@last_captures[i['service']] += i['sum']
		end

		return @last_captures
	end
end

class Cube
	def initialize(url)
		@url = URI.parse(url)
		@post = Net::HTTP::Post.new(@url.path, {'Content-Type' =>'application/json'})
		@http = Net::HTTP.new(@url.host, @url.port)
	end

	def emit(data)
		submit = {
			type: "gameserver",
			data: data,
			time: Time.now
		}
		post(submit)
	end

	private
	def post(body)
		@post.body = [ body ].to_json
		response = @http.start do |http|
			http.request(@post)
		end
		puts Wirble::Colorize.colorize(body.pretty_inspect) + " => " + response.inspect
		puts
	end
end

db = DB.new(*GAMEDB)
cube = Cube.new(CUBEURL)

loop do
	caps = db.get_captures()
	cube.emit({
		checker_errors: db.get_checker_errors(),
		captures: caps,
		captures_total: caps.map{ |k,v| v }.reduce(&:+),
		stats: db.get_stats()
	})
	sleep(CUBEINT)
end
