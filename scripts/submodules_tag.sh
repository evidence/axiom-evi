#!/bin/bash

set -x

## This script push commit by all submodules

git tag $@

#git submodule foreach --recursive \
git submodule foreach \
    "git tag $@ || :"
