#!/bin/sh -l

cd /beatmup
mkdir build
cd build
cmake -DUSE_GLX=1 ../apps
make all
