#!/usr/bin/env bash
barids=$(pgrep -a waybar | grep utilities)
barid=${barids/ *}

function colorpicker {
	kill -SIGUSR1 $barid
	hyprpicker -a
}

function clipboard {
	kill -SIGUSR1 $barid
	# cliphist list | rofi -dmenu | cliphist decode | wl-copy
	cclip list preview | rofi -dmenu | wl-copy
}

case $1 in
	colorpicker) colorpicker ;;
	clipboard) clipboard ;;
	*) exit ;;
esac
