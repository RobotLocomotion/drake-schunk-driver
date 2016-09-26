#!/bin/bash

set -e
(
rm -rf build
mkdir build
cd build
cmake ..
make
)

target=./build/schunk-driver-prefix/src/schunk-driver-build/bin/schunk_driver

if [ -x $target ] ; then
    echo "built: $target"
else
    echo "Failure"
    exit 1
fi
