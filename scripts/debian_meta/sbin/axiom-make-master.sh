#!/bin/bash

DATE="$(date "+%Y-%m-%d %H:%M:%S")"

echo -e "Start AXIOM discovery algorithm ..."

#[ -e /etc/axiom/axiomrc ] && sed -i "s/MASTER=no/MASTER=yes/" /etc/axiom/axiomrc
axiom-utility -m

sleep 3
journalctl --no-pager -S "$DATE" -u axiom-startup.service
