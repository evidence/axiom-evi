#!/bin/bash

set -x

## This script push commit by all submodules

git push --tags $1

#git submodule foreach --recursive \
git submodule foreach \
    "git push --tags $1 || :"
