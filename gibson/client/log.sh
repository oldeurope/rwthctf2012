#!/bin/bash

mkdir -p logs

DATE=$(date +'%Y%m%d-%H%M%S')
LOG="logs/$DATE.gource.log.gz"
RAW="logs/$DATE.raw.log.gz"

echo Using logfile $RAW

ruby1.9.1 client_gource.rb 10.12.250.1 5000 10.12.250.1 27017 2>&1 \
	| grep -P --line-buffered "^raw" \
	| tee $RAW
