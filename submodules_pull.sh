#!/bin/bash

set -x

## This script push commit by all submodules

git pull --rebase

#git submodule foreach --recursive \
git submodule foreach \
    'git pull --rebase || :'
