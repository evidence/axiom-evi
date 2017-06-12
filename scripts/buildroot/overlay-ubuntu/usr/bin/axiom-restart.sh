#!/bin/sh -e

killall axiom-init || true
rmmod axiom_netdev || true
rmmod axiom_mem_dev || true
rmmod axiom_mem_manager || true
/usr/bin/axiom-start.sh
