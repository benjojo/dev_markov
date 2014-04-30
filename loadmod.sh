#!/bin/bash
make clean
make
rmmod mkov
insmod mkov.ko
mknod /dev/mkov c 89 1
echo "TESTING TO SEE IF ITS IN THE RIGHT PLACES"
cat /proc/modules | grep -i Mkov
cat /proc/devices | grep -i Mkov
