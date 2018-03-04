#!/bin/bash

socat -u FILE:ps4-pkg2usb.bin TCP:192.168.1.124:9020
socat - tcp-listen:9023