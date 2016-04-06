#!/bin/bash

set -x

## This script push commit by all submodules

git pull

#git submodule foreach --recursive \
git submodule foreach \
    'git pull || :'
