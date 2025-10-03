#!/usr/bin/env bash
id=$(hyprctl activewindow | grep Window | awk '{print $2}')
ID="0x${id}"

one="YES"
two="NO"
choice=$(printf "$two\n$one" | rofi -dmenu -i -config $HOME/.config/rofi/ka-conf.rasi)

case $choice in
	$one) hyprctl dispatch killactive;;
	*) exit 1 ;;
esac
