#!/bin/bash

set -e

pushd tool
make
popd

make

tool/bin2js ps4-pkg2usb.bin > exploit/payload.js
