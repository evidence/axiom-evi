#!/bin/sh -e

[ -e /etc/axiomrc ] && . /etc/axiomrc
modprobe axiom-netdev || true
if [ x"${MASTER}" = x"yes" ]; then
    axiom-init -m &
else
    axiom-init &
fi
