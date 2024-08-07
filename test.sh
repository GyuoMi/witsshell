#!/bin/bash

make clean
make
mv witsshell src
cd src

./test-witsshell.sh
