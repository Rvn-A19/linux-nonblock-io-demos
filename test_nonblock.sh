#!/bin/bash


if [ ! -f ./nonblock ]; then
  gcc nonblock.c -Wall -O2 -o nonblock
fi

if [ -f ./testfile ]; then
  rm ./testfile
fi

./nonblock & ls -l ./testfile

