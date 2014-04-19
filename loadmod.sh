#!/bin/bash
modprobe mkov.ko
echo "TESTING TO SEE IF ITS IN THE RIGHT PLACES"
cat /proc/modules | grep -i Mkov
cat /proc/devices | grep -i Mkov
