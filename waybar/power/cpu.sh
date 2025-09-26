#!/bin/bash

# get stuff
while true; do
	load=$(echo $[100-$(vmstat 1 2|awk 'END {print $15}')])
	# temp=$(sensors -f | grep "id 0" | awk '{print $4}' | sed 's/+//' | sed 's/\..*//')
	# uptime=$(awk '{m=$1/60; h=m/60; printf "%sd%sh", int(h/24), int(h%24), int(m%60), int($1%60) }' /proc/uptime)
	# tc=$(ps ax -o %cpu,comm --no-headers | sort -k1 | tail -n3 | sed 's/\.[0-9]//' | sort -rnk1 | awk '{print $1"% -> "$2"\r"}')
	# tooltip='<big>Top Republicans</big>\r '$tc'\r'$load'% <small>/</small> '$temp'° <small>/</small> '$uptime''
	 
	# do stuff
	if [ "$load" -le 25 ]; then
		echo "{\"text\": \"    \", \"class\":\"default\"}" | jq --unbuffered --compact-output
	elif [ "$load" -gt 25 ] && [ "$load" -lt 60 ]; then 
		echo "{\"text\": \"    \", \"class\":\"mid\"}" | jq --unbuffered --compact-output
	elif [ "$load" -gt 60 ] && [ "$load" -lt 80 ]; then
		echo "{\"text\": \"    \", \"class\":\"warning\"}" | jq --unbuffered --compact-output
	elif [ "$load" -ge 80 ]; then
		echo "{\"text\": \"    \", \"class\":\"critical\"}" | jq --unbuffered --compact-output
	fi
	sleep 1
done
