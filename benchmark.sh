#!/bin/bash

# config
LIKWID_PATH=/usr/local
NUM_CORES=$(nproc)
export LD_LIBRARY_PATH=$LIKWID_PATH/lib:$LD_LIBRARY_PATH

### variable definitions
## bandwidth
BW_NUM_TESTS=3
BW_TESTS=("Read/write bandwidth" "Read bandwidth" "Write bandwidth")
BW_LIKWID_TEST_NAME=(copy_avx load_avx store_avx)

# must correspond to size of arrays below
BW_NUM_LEVELS=4

# portion of cache size test should take up
BW_CACHE_PORTION="1/4"


# --- CACHE AND BW TEST SIZES --- #
# these commands use likwid-topology to get information on the current system
# and automatically set appropriate test sizes

# get output of likwid-topology command
TOPO_STRING=$(likwid-topology -c -O)

# parse L1 cache size from likwid-topology output. First grep for 'Level:,1'
# string and print that line and the line after. Then, grep for 'Size' to
# isolate the line with the size on it. Finally, use 'sed' to capture just the
# number and suffix (i.e. suffix == 'kB' or 'MB') and replace the entire output
# with just the number+suffix
CACHE_SIZE_L1=$(echo "$TOPO_STRING" | grep 'Level:,1' -A 1 | grep 'Size' | sed 's/Size:,\([0-9]\+\) \([a-zA-Z]\+\),\+/\1\2/')

# get just the number from the cache size string
BW_SIZE_L1_NUMBER=$(echo $CACHE_SIZE_L1 | sed 's/[a-zA-Z]\+//')

# multiply that value by the ratio we specified earlier
BW_SIZE_L1_NUMBER=$(( $BW_SIZE_L1_NUMBER * $BW_CACHE_PORTION))

# check to make sure our integer division did not result in 0
if [ $(( $BW_SIZE_L1_NUMBER == 0 )) -eq 1 ]; then BW_SIZE_L1_NUMBER=1; fi

# get just the suffix from the cache size string
BW_SIZE_L1_SUFFIX=$(echo $CACHE_SIZE_L1 | sed 's/[0-9]\+//')

# combine the number and suffix to get the full string likwid expects
BW_SIZE_L1=$BW_SIZE_L1_NUMBER$BW_SIZE_L1_SUFFIX

# repeat that process for all cache levels
CACHE_SIZE_L2=$(echo "$TOPO_STRING" | grep 'Level:,2' -A 1 | grep 'Size' | sed 's/Size:,\([0-9]\+\) \([a-zA-Z]\+\),\+/\1\2/')
BW_SIZE_L2_NUMBER=$(echo $CACHE_SIZE_L2 | sed 's/[a-zA-Z]\+//')
BW_SIZE_L2_NUMBER=$(( $BW_SIZE_L2_NUMBER * $BW_CACHE_PORTION))
if [ $(( $BW_SIZE_L2_NUMBER == 0 )) -eq 1 ]; then BW_SIZE_L2_NUMBER=1; fi
BW_SIZE_L2_SUFFIX=$(echo $CACHE_SIZE_L2 | sed 's/[0-9]\+//')
BW_SIZE_L2=$BW_SIZE_L2_NUMBER$BW_SIZE_L2_SUFFIX

CACHE_SIZE_L3=$(echo "$TOPO_STRING" | grep 'Level:,3' -A 1 | grep 'Size' | sed 's/Size:,\([0-9]\+\) \([a-zA-Z]\+\),\+/\1\2/')
BW_SIZE_L3_NUMBER=$(echo $CACHE_SIZE_L3 | sed 's/[a-zA-Z]\+//')
BW_SIZE_L3_NUMBER=$(( $BW_SIZE_L3_NUMBER * $BW_CACHE_PORTION))
if [ $(( $BW_SIZE_L3_NUMBER == 0 )) -eq 1 ]; then BW_SIZE_L3_NUMBER=1; fi
BW_SIZE_L3_SUFFIX=$(echo $CACHE_SIZE_L3 | sed 's/[0-9]\+//')
BW_SIZE_L3=$BW_SIZE_L3_NUMBER$BW_SIZE_L3_SUFFIX

# set ram test to use 100x the size of L3 cache
BW_SIZE_MEM=$(( 100* $(echo $CACHE_SIZE_L3 | sed 's/[a-zA-Z]\+//') ))
BW_SIZE_MEM=$BW_SIZE_MEM$BW_SIZE_L3_SUFFIX


# sizes should comfortably fit inside the cache/memory they are meant for
# (typically, this means they should be half the size of that system)
BW_LIKWID_GROUP=( L1           L2           L3          MEM   )
       BW_SIZES=( $BW_SIZE_L1  $BW_SIZE_L2  $BW_SIZE_L3 $BW_SIZE_MEM )

       # 20 minutes or more each
#      BW_ITERS=( 25000000000  2500000000   100000000  2000  )
 
       # about 1 minute each
#      BW_ITERS=( 2400000000   240000000    5000000    2500   )

       # about 15 seconds each
       BW_ITERS=( 600000000    60000000     1300000    700   )
 
       # less than 3 seconds each
#      BW_ITERS=( 20000000     2000000      500000     50    )

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
                                                                             
       # about 1 minute each                                         
       FLOPS_ITERS=( 130000000   130000000) 

       # about 15 seconds each                                         
#      FLOPS_ITERS=( 40000000   40000000) 
                                                                       
       # less than 3 seconds each                                      
#      FLOPS_ITERS=( 3000000   3000000) 


#### Function definitions
# params expected: test simple name, bench name, likwid group, size, num iter
# for example: Read/write bandwidth, copy_avx, L2, 10kB, 2000000
run_likwid_bench_test () { 
    echo "Test $2 ($1) for group $3. Size: $4, num_iter: $5."
  
    SECONDS=0
  
    # Measurement with likwid

    # for compatibility, this is not enabled by default. No architecture
    # currently has an L1 group, and many architectures do not have a MEM group
    #$LIKWID_PATH/bin/likwid-perfctr -C S0:0-$(( $NUM_CORES - 1 )) \
    #    -g $3 -m -M 1 $LIKWID_PATH/bin/likwid-bench \
    #    -t $2 -i $5 -w S0:$4:$NUM_CORES \
    #    | grep -B 1 -A 20 -E "Metric.*Sum" # comment this last line if you don't want to exclude benchmark info
  
    # Measurement with likwid-bench internal tracking
    echo "Running command '$LIKWID_PATH/bin/likwid-bench -t $2 -i $5 -w S0:$4:$NUM_CORES'"
    $LIKWID_PATH/bin/likwid-bench -t $2 -i $5 -w S0:$4:$NUM_CORES \
      | grep -E 'Test|MByte/s|MFlop/s|elapsed'
  
    # time
    echo "$(($SECONDS / 3600))h $((($SECONDS / 60) % 60))m $(($SECONDS % 60))s"\
      " elapsed."
    echo
}

# TODO: should just use sed to isolate values and create the machine-stats json

#### Bandwidth
for test_num in $(seq 0 $(($BW_NUM_TESTS - 1)) ); do
  for level in $(seq 0 $(($BW_NUM_LEVELS-1)) ); do
    run_likwid_bench_test "${BW_TESTS[$test_num]}" \
      "${BW_LIKWID_TEST_NAME[$test_num]}" "${BW_LIKWID_GROUP[$level]}" \
      "${BW_SIZES[$level]}" "${BW_ITERS[$level]}"
    echo
  done
done

#### FLOPS
for test_num in $(seq 0 $(($FLOPS_NUM_TESTS - 1)) ); do
  run_likwid_bench_test "${FLOPS_TESTS[$test_num]}" \
    "${FLOPS_LIKWID_TEST_NAME[$test_num]}" "${FLOPS_LIKWID_GROUP[$test_num]}" \
    "${FLOPS_SIZES[$test_num]}" "${FLOPS_ITERS[$test_num]}"
  echo
done

