#!/usr/bin/env ruby
require File.expand_path("../rubyver.rb", __FILE__)
require File.expand_path("../db.rb", __FILE__)
require 'open-uri'

DBHOST = "10.12.250.1"
DBPORT = 27017
URL = "http://10.12.250.1/teamlogos/"
OUTPUT = "img/teams"

db = DB.new(DBHOST, DBPORT)
teams = db.get_teams()

teams.each do |id,name|
	puts "Fetching team #{id} (#{name})..."
	File.open(OUTPUT + "/" + name.gsub("/","") + ".png", "wb") do |output|
		open(URL + id.to_s + ".png") do |download|
			output.write download.read
		end
	end
end
