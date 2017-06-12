#!/bin/sh -e

[ -e /etc/axiomrc ] && sed -i "s/MASTER=no/MASTER=yes/" /etc/axiomrc
/usr/bin/axiom-restart.sh
