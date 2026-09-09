#!/usr/bin/env bash
check=$(pgrep -f 'waybar.*utilities.*config')
readarray -t pids < <(hyprctl layers -j | jq -r '.[].levels."2".[].pid')

for wbar in "${pids[@]}"; do
	if [[ "$wbar" == "$check" ]]; then
		kill -SIGUSR1 "$check"
	fi
done
pkill -x rofi || rofi -show drun
