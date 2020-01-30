#!/bin/bash
echo $HOSTNAME
cat /proc/cpuinfo
echo
echo
echo "================================"
echo "running fhv!"
echo "================================"
echo
./bin/fhv
