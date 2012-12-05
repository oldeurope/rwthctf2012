#!/bin/bash

/usr/bin/gource \
	--padding 1.0 \
	--user-scale 2.5 \
	--hide root,filenames,dirnames \
	--highlight-dirs \
	--title "rwthCTF 2012" \
	-1280x720 \
	--log-format custom \
	--logo img/itsec_logo.png \
	--key \
	--file-idle-time 500 \
	--user-image-dir img/teams/ \
	--background-colour 0A0E14 \
	--max-user-speed 2500 \
	$*
