#!/bin/bash

set -x

## This script push commit by all submodules

git push $1

#git submodule foreach --recursive \
git submodule foreach \
    "git push $1 || :"
