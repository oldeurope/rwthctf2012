#!/usr/bin/env ruvy
if RUBY_VERSION[0..2] == "1.8" then
	STDERR.puts "Please use Ruby >= 1.9"
	exit -1
end
