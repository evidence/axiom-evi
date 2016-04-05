#!/bin/bash

set -x

## This script push commit by all submodules

git push --tags

#git submodule foreach --recursive \
git submodule foreach \
    'git push --tags || :'
