#!/bin/bash

# get stuff
while true; do
	fahrenheit=$(sensors -f | grep "id 0" | awk '{print $4}' | sed 's/+//' | sed 's/\..*//')
	temporary=$((fahrenheit - 30))
	temperature=$((temporary / 2))
	 
	# do stuff
	if [ "$temperature" -le 25 ]; then
		echo "{\"text\": \"    \", \"class\":\"default\"}" | jq --unbuffered --compact-output
	elif [ "$temperature" -gt 25 ] && [ "$temperature" -lt 60 ]; then 
		echo "{\"text\": \"    \", \"class\":\"mid\"}" | jq --unbuffered --compact-output
	elif [ "$temperature" -gt 60 ] && [ "$temperature" -lt 80 ]; then
		echo "{\"text\": \"    \", \"class\":\"warning\"}" | jq --unbuffered --compact-output
	elif [ "$temperature" -ge 80 ]; then
		echo "{\"text\": \"    \", \"class\":\"critical\"}" | jq --unbuffered --compact-output
	fi
	sleep 1
done
