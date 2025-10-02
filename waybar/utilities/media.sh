#!/bin/bash
function image() {
	while true; do
		artUrl=$(playerctl metadata | grep artUrl | awk '{print $3}' | sed 's|file://||')
		cache="/tmp/mediacache"
		thumbnail="/tmp/mediathumbnail"
		fullthumb="/tmp/fullthumbnail"
		defaultone="$HOME/.config/waybar/utilities/thumbnail.png"
		mask="$HOME/.config/waybar/utilities/mask.png"

		# DIFFERENCE=$(magick compare -metric RMSE "$thumbnail" "$cache" null: 2>&1 | awk '{print $1}')
		# if (( $(echo "$DIFFERENCE > 0.0" | bc -l) )); then
		# if [[ "$thumbnail" != "$cache" ]]; then
		if [[ "$(md5sum "$artUrl" | awk '{print $1}')" != "$(md5sum "$fullthumb" | awk '{print $1}')" ]]; then
			cp -rf $artUrl $fullthumb
			magick $artUrl -thumbnail 720x720^ -gravity center -extent 720x720 $cache
			# cp -rf $cache $thumbnail
			magick $cache -matte $mask -compose DstIn -composite $thumbnail
		fi
		if [ ! -e "$cache" ]; then
			cp -rf $defaultone $fullthumb
			cp -rf $defaultone $thumbnail
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


		if [[ "$check" == "Playing" || "Paused" ]]; then
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
