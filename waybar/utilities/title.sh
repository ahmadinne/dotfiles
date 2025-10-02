#!/usr/bin/env bash

while true; do
	teks=$(playerctl metadata --format "{{title}}")
	if [ -z "$teks" ]; then
		teks="Nothing played rn."
	fi
	# hitung=$(echo "$teks" | wc -m)
	hitung=${#teks}
	max_len=15

	if [ "$hitung" -gt "$max_len" ]; then
		teks_gabung="${teks} ${teks}"

		for ((i=0; i<${#teks}; i++)); do
			echo "{\"text\": \"${teks_gabung:((i % ${#teks_gabung})):max_len}\", \"class\": \"\"}" | jq --unbuffered --compact-output .
				sleep 1
			done
		else
			echo "{\"text\": \"$(echo "$teks")\"}"
			sleep 2
	fi
done

#
#
#     if [ "$teks" != "$last_teks" ]; then
#         hitung=${#teks}
#
#         if [ "$hitung" -gt "$max_len" ]; then
#             teks_gabung="$teks $teks"
#             i=0
#             while [ "$teks" = "$(playerctl metadata --format "{{title}}")" ]; do
#                 potong=$(printf "%-${max_len}.${max_len}s" "${teks_gabung:i}")
#                 echo "{\"text\": \"$potong\", \"class\": \"\"}" | jq --unbuffered --compact-output .
#                 sleep 0.5
#                 ((i++))
#                 (( i >= hitung )) && i=0  # loop smoothly
#             done
#         else
#             while [ "$teks" = "$(playerctl metadata --format "{{title}}")" ]; do
#                 echo "{\"text\": \"$teks\", \"class\": \"\"}" | jq --unbuffered --compact-output .
#                 sleep 2
#             done
#         fi
#         last_teks="$teks"
#     fi
# done

