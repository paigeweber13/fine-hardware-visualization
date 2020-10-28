#!/bin/bash
# start in root directory of this project

make

export LD_LIBRARY_PATH=/usr/local/likwid-master/lib:~/code/fine-hardware-visualization/build/lib

colors=("128 128 128 227 74 51" "200 200 200 43 140 190"
  "128 128 128 43 140 190" "128 128 128 5 189 11" "128 128 128 5 63 189"
  "227 74 51 43 140 190")

for color in "${colors[@]}"; do
  echo $color
  build/bin/fhv --test-color-lerp -c $color
done
