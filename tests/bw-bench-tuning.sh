#!/bin/bash

total_iter=3000000
size_kib=100
measured_iter=1

make

# for measured_iter in {1..10}; do
while [[ $measured_iter -lt 10000 ]]; do
  date

  LD_LIBRARY_PATH=/usr/local/likwid-master/lib bin/fhv -2 \
    $total_iter $measured_iter $size_kib > tests/data/l2_rw_bw_bench.txt

  date 
  grep -A 4 params tests/data/l2_rw_bw_bench.txt --color
  grep bandwidth tests/data/l2_rw_bw_bench.txt --color

  measured_iter=$(( $measured_iter * 2 ))
done

