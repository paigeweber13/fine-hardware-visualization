#!/bin/bash

make
makeCode=$?
if [ $makeCode -ne 0 ]; then
  echo "make failed, exiting..."
  exit $makeCode
fi

max_n=1000000001

for mt in manual fhv manual_parallel ; do
  echo "Measurement type: $(echo $mt)"
  rm -f data/peakflops_sp_avx_fma_$(echo $mt)_output.txt

  n=100
  num_i=100000000
  while [ $n -lt $max_n ]; do
    export FHV_OUTPUT="data/peakflops_sp_avx_fma_$(echo $n)_$(echo $num_i).json"
    ./bin/microbenchmarks $mt $n $num_i | tee -a data/peakflops_sp_avx_fma_$(echo $mt)_output.txt

    ((n *= 10))
    ((num_i /= 10))
  done

  echo
done

fhv -v data/*.json
