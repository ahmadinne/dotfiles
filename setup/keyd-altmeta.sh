#!/usr/bin/env bash

if [[ -z $(pacman -Qq | grep keyd) ]]; then
	sudo pacman -S keyd
fi

if [[ ! -d "/etc/keyd" ]]; then
	sudo mkdir /etc/keyd
fi

sudo echo " [ids]
*

[main]
leftalt   = leftmeta
rightalt  = rightmeta
leftmeta  = leftalt
rightmeta = rightalt" > /etc/keyd/default.conf

if [[ ! $(systemctl | grep keyd) ]]; then
	sudo systemctl enable keyd
	sudo systemctl start keyd
else
	sudo systemctl restart keyd
fi
