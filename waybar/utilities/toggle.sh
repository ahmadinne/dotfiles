#!/bin/bash

id=$(ps -ef | grep '[w]aybar -c' | grep utilities | awk '{print $2}')

if [ "$id" ]; then
    kill $id
else 
    waybar -c $HOME/.config/waybar/utilities/config.jsonc -s $HOME/.config/waybar/utilities/style.css
fi
