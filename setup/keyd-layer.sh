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
leftalt = layer(alt)
rightalt = layer(alt)

[alt:A]
a = 1
s = 2
d = 3
f = 4
g = 5
h = 6
j = 7
k = 8
l = 9
; = 0
" > /etc/keyd/default.conf
