#!/bin/bash
echo $HOSTNAME
cat /proc/cpuinfo
echo
echo
echo "================================"
echo "running bench!"
echo "================================"
echo
likwid-perfctr -C 0-15 -g FLOPS_SP bin/bench

