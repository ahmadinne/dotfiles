activeworkspace=$(hyprctl activeworkspace | grep "workspace ID" | awk '{print $3}')
windowstatus=$(hyprctl clients -j | jq ".[] | select(.workspace.id == ${activeworkspace})" | grep -c '"floating": false')
if [ "$windowstatus" -lt 1 ]; then
	echo "{\"class\": \"invisible\"}" | jq --unbuffered --compact-output .
else
	echo "{\"class\": \"visible\"}" | jq --unbuffered --compact-output .
fi
