#!/bin/bash
make clean
make
rmmod mkov
insmod mkov.ko
mknod /dev/mkov c 89 1
echo "Testing to see if module is loaded"
cat /proc/modules | grep -i Mkov
cat /proc/devices | grep -i Mkov
