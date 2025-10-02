#!/usr/bin/env bash
barid=$(ps -ef | grep '[w]aybar -c' | grep utilities | awk '{print $2}' )

function colorpicker {
	kill $barid
	hyprpicker -a
}

function clipboard {
	kill $barid
	cliphist list | rofi -dmenu | cliphist decode | wl-copy
}

choice=$1
case $choice in
	colorpicker) colorpicker ;;
	clipboard) clipboard ;;
	*) exit ;;
esac
