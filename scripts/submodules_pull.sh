#!/bin/bash

set -x

## This script push commit by all submodules

git pull $1

#git submodule foreach --recursive \
git submodule foreach \
    "git pull $1 || :"
