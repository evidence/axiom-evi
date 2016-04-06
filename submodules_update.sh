#!/bin/bash

set -x

## This script inits all submodules and checkouts the rigth branch ##

git submodule update --init --recursive
git submodule foreach --recursive \
    'git checkout \
    $(git config -f $toplevel/.gitmodules submodule.$name.branch || echo master)'
