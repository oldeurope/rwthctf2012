#!/usr/bin/env bash
wget http://10.12.250.1/json/up -q -O - | grep ping | sed -e "s/ping: //" -e "s/,/\n/g" | awk '{ print "10.12." $1 ".9 8080" }' > peers.txt

