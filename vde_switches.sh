#!/bin/sh

num_switches="$1"

for i in $(seq $num_switches); do 
	vde_switch -sock /tmp/net$i.vde -daemon
	sudo ifconfig tap$i create
	sudo ifconfig tap$i up
	sudo vde_plug2tap -s /tmp/net$i.vde tap$i -daemon
done 
