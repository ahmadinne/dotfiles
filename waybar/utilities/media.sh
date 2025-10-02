#!/bin/bash

# Old shit don't used anymore
# create square thumbnail
# magick "$artUrl" -thumbnail 720x720^ -gravity center -extent 720x720 "$cache"

# apply mask
# magick "$cache" -matte "$mask" -compose DstIn -composite "$thumbnail"

function image() {
	thumbnail="/tmp/mediathumbnail"
	fullthumb="/tmp/fullthumbnail"
	defaultone="$HOME/.config/waybar/utilities/thumbnail.png"
	mask="$HOME/.config/waybar/utilities/mask.png"
	lastUrl=""

	while true; do
		artUrl=$(playerctl metadata --format "{{mpris:artUrl}}" | sed 's|file://||')

		if [[ "$artUrl" != "$lastUrl" ]]; then
			lastUrl="$artUrl"

			if [ -n "$artUrl" ] && [ -e "$artUrl" ]; then
				cp -f "$artUrl" "$fullthumb"
				magick "$artUrl" -thumbnail 720x720 -gravity center -extent 720x720 -matte "$mask" -compose DstIn -composite "$thumbnail"
			else
				cp -f "$defaultone" "$fullthumb"
				cp -f "$defaultone" "$thumbnail"
			fi
		fi
		sleep 1
	done
}

function artist() {
	while true; do
		check=$(playerctl status)
		names=$(playerctl metadata | grep artist | grep -oP 'artist\s+\K.*')
		hitung=$(echo "$names" | wc -m)

		if [[ "$hitung" -gt 20 ]]; then
			names=$(echo $names | awk '{print $1 " " $2}')
		fi


		if [[ "$check" == "Playing" ]]; then
			echo "${names}"
		elif [[ "$check" == "Paused" ]]; then
			echo "${names}"
		else
			echo "Unknown"
		fi
		sleep 1
	done
}

function wmname() {
	wmname="$(xprop -id $(xprop -root -notype | awk '$1=="_NET_SUPPORTING_WM_CHECK:"{print $5}') -notype -f _NET_WM_NAME 8t | grep "WM_NAME" | cut -f2 -d \")"
	echo $wmname
}

function screenrecord() {
	while true; do
		check=$(ps -ef | grep '[w]f-recorder'| awk '{print $2}')
		if [[ "$check" ]]; then
			echo "{\"class\": \"recording\"}" | jq --unbuffered --compact-output .
		else
			echo "{\"class\": \"default\"}" | jq --unbuffered --compact-output .
		fi
	done
}

function button() {
	while true; do
		check=$(playerctl status)
		if [[ "$check" == Playing ]]; then
			echo "{\"class\": \"playing\"}" | jq --unbuffered --compact-output .
		else
			echo "{\"class\": \"paused\"}" | jq --unbuffered --compact-output .
		fi
	done
}

function autobar() {
    activeworkspace=$(hyprctl activeworkspace | grep "workspace ID" | awk '{print $3}')
    windowstatus=$(hyprctl clients -j | jq ".[] | select(.workspace.id == ${activeworkspace})" | grep -c '"floating": false')
	if [ "$windowstatus" -lt 1 ]; then
		echo "{\"class\": \"invisible\"}" | jq --unbuffered --compact-output .
	else
		echo "{\"class\": \"visible\"}" | jq --unbuffered --compact-output .
	fi
}

case $1 in
	image) image ;;
	artist) artist ;;
	button) button ;;
	wmname) wmname ;;
	screenrecord) screenrecord ;;
	autobar) autobar ;;
esac
