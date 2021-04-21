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
     "num_flops_percent_diff,Mflop_rate_percent_diff"

max_n=1000000001

n=100
num_i=100000000
while [ $n -lt $max_n ]; do
  export FHV_OUTPUT="data/peakflops_sp_avx_fma_$(echo $n)_$(echo $num_i).json"
  ./bin/microbenchmarks $n $num_i

  ((n *= 10))
  ((num_i /= 10))
done

fhv -v data/*.json >&2

