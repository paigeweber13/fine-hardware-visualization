#!/bin/bash

# config
LIKWID_PATH=/usr/local/likwid-master
NUM_CORES=4
export LD_LIBRARY_PATH=$LIKWID_PATH/lib:$LD_LIBRARY_PATH

### variable definitions
## bandwidth
BW_NUM_TESTS=3
BW_TESTS=("Read/write bandwidth" "Read bandwidth" "Write bandwidth")
BW_LIKWID_TEST_NAME=(copy_avx load_avx store_avx)

# must correspond to size of arrays below
BW_NUM_LEVELS=3

# sizes should comfortably fit inside the cache/memory they are meant for
# (typically, this means they should be half the size of that system)
BW_LIKWID_GROUP=( L2          L3         MEM     FLOPS_DP   FLOPS_SP)
       BW_SIZES=( 100kB       2MB        2GB     10kB       10kB    )

       # 20 minutes or more each
#      BW_ITERS=( 2500000000  100000000  1000    3000000000 3000000000) 
                                                                       
       # about 15 seconds each                                         
       BW_ITERS=( 25000000    1000000    100     40000000   40000000) 
                                                                       
       # less than 3 seconds each                                      
#      BW_ITERS=( 2000000     100000     5       10000000   10000000) 

## flops
FLOPS_NUM_TESTS=2
FLOPS_TESTS=("Double-precision flop/s" "Single-precision flop/s")
FLOPS_LIKWID_TEST_NAME=(peakflops_avx_fma peakflops_sp_avx_fma)

# sizes should comfortably fit inside the cache/memory they are meant for
# (typically, this means they should be half the size of that system)
FLOPS_LIKWID_GROUP=( FLOPS_DP   FLOPS_SP)
       FLOPS_SIZES=( 10kB       10kB    )

       # 20 minutes or more each
#      FLOPS_ITERS=( 3000000000 3000000000) 
                                                                       
       # about 15 seconds each                                         
       FLOPS_ITERS=( 40000000   40000000) 
                                                                       
       # less than 3 seconds each                                      
#      FLOPS_ITERS=( 3000000   3000000) 


#### Function definitions
# params expected: test simple name, bench name, likwid group, size, num iter
# for example: Read/write bandwidth, copy_avx, L2, 10kB, 2000000
run_likwid_bench_test () { 
    echo
    echo
    echo "Test $2 ($1) for group $3. Size: $4, num_iter: $5."
    echo
  
    SECONDS=0
  
    $LIKWID_PATH/bin/likwid-perfctr -C S0:0-$(( $NUM_CORES - 1 )) \
        -g $3 -m -M 1 $LIKWID_PATH/bin/likwid-bench \
        -t $2 -i $5 -w S0:$4:$NUM_CORES \
      | grep -B 1 -A 20 -E "Metric.*Sum"
  
    # time
    echo "$(($SECONDS / 3600))h $((($SECONDS / 60) % 60))m $(($SECONDS % 60))s"\
      " elapsed."
}

#### Bandwidth

# WARNING: currently likwid-perfctr is reporting unreasonably high values
# (around 1.8e19) for the bandwidth benchmark for RAM. These results should
# be discarded.

for test_num in $(seq 0 $(($BW_NUM_TESTS - 1)) ); do
  for level in $(seq 0 $(($BW_NUM_LEVELS-1)) ); do
    run_likwid_bench_test "${BW_TESTS[$test_num]}" \
      "${BW_LIKWID_TEST_NAME[$test_num]}" "${BW_LIKWID_GROUP[$level]}" \
      "${BW_SIZES[$level]}" "${BW_ITERS[$level]}"
  done
done

#### FLOPS
for test_num in $(seq 0 $(($FLOPS_NUM_TESTS - 1)) ); do
  run_likwid_bench_test "${FLOPS_TESTS[$test_num]}" \
    "${FLOPS_LIKWID_TEST_NAME[$test_num]}" "${FLOPS_LIKWID_GROUP[$test_num]}" \
    "${FLOPS_SIZES[$test_num]}" "${FLOPS_ITERS[$test_num]}"
done

