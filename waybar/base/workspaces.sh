#!/bin/bash

while true; do
  activeworkspace=$(hyprctl activeworkspace | grep "workspace ID" | awk '{print $3}')
  # totalworkspaces=$(hyprctl workspaces | grep -c "workspace ID")
  totalworkspaces=$(hyprctl workspaces | grep "workspace ID" | sort -nk3 | awk 'END {print $3}')

  echo "${activeworkspace}/${totalworkspaces}"
done
