#!/bin/bash

if [ $# -ne 3 ]
then
  echo "Usage: `basename $0` <path to gource binary> <server> <port>"
  exit
fi

mkdir -p logs

DATE=$(date +'%Y%m%d-%H%M%S')
LOG="logs/$DATE.gource.log.gz"
RAW="logs/$DATE.raw.log.gz"

echo Using logfiles $LOG and $RAW
echo Starting gource...

ruby1.9.1 client_gource.rb $2 $3 10.12.250.1 27017 \
	2> >((tee >(grep --line-buffered -P "^raw:" | sed -e "s/^raw: //" | gzip > $RAW) | grep -vP "^raw: ") >&2) \
	| tee >(gzip > $LOG) \
	| $1 \
	--padding 1.0 \
	--user-scale 2.5 \
	--hide date,root,bloom,filenames,dirnames,progress \
	--highlight-dirs \
	--title "rwthCTF 2012" \
	-1920x1080 \
	--realtime \
	--log-format custom \
	--logo img/itsec_logo.png \
	--key \
	--file-idle-time 500 \
	--user-image-dir img/teams/ \
	--background-colour 0A0E14 \
	--max-user-speed 2500 \
	-

