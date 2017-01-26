#!/bin/sh

killall axiom-init
rmmod axiom_netdev
rmmod axiom_mem_dev
rmmod axiom_mem_manager
./axiom-startup.sh $1
