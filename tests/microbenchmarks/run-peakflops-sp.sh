#!/bin/bash

make >&2
makeCode=$?
if [ $makeCode -ne 0 ]; then
  echo "make failed, exiting..."
  exit $makeCode
fi

echo "array_n,array_size_bytes,num_i,"\
     "manual_reported_num_flops,fhv_reported_num_flops,"\
     "manual_reported_Mflop_rate,fhv_reported_Mflop_rate,"\
     "num_flops_diff_factor,Mflop_rate_diff_factor"

max_n=100000001
min_num_i=1000

n=100
num_i=100000000
while [ $n -lt $max_n ]; do
  if [ $num_i -lt $min_num_i ]; then
    num_i=$min_num_i
  fi

  export FHV_OUTPUT="data/peakflops_sp_avx_fma_$(echo $n)_$(echo $num_i).json"
  ./bin/microbenchmarks peakflops_sp $n $num_i

  ((n *= 10))
  ((num_i /= 10))
done

fhv -v data/*.json >&2

