#!/bin/bash
modprobe devMkov.ko
echo "TESTING TO SEE IF ITS IN THE RIGHT PLACES"
cat /proc/modules | grep Mkov
cat /proc/devices | grep Mkov
