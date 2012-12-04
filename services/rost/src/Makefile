.PHONY: all

all: server_min.js client_min.js checker_full.js
full: server_full.js client_full.js

server_full.js: main.js server.js
	cat main.js server.js > server_full.js

client_full.js: main.js client.js
	cat main.js client.js > client_full.js

server_min.js: main.js server.js externs.js
	java -jar compiler.jar --compilation_level ADVANCED_OPTIMIZATIONS --externs externs.js --js main.js --js server.js --js_output_file server_min.js

client_min.js: main.js client.js externs.js
	java -jar compiler.jar --compilation_level ADVANCED_OPTIMIZATIONS --externs externs.js --js main.js --js client.js --js_output_file client_min.js

checker_full.js: main.js checker.js
	cat main.js checker.js > checker_full.js
