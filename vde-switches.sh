#!/bin/sh

num_switches="$1"

for i in $(seq $num_switches); do 
	vde_switch -sock /tmp/net$((i-1)).vde -daemon
	sudo ifconfig tap$((i-1)) create
	sudo ifconfig tap$((i-1)) up
	sudo vde_plug2tap -s /tmp/net$((i-1)).vde tap$((i-1)) -daemon
done 
