#!/usr/bin/env bash

colorpicker {
	barid=$(ps -ef | grep '[w]aybar -c' | grep utilities | awk '{print $2}' )
	kill $barid
	hyprpicker -a
}

case $1 in
	colorpicker) colorpicker ;;
esac
