#!/bin/bash

# config
LIKWID_PATH=/usr/local/likwid-master
NUM_CORES=4

NUM_TESTS=3
TESTS=(ReadWrite Read Write)
LIKWID_TEST_NAME=(copy_avx load_avx store_avx)

# must correspond to size of arrays below

NUM_LEVELS=3

# sizes should comfortably fit inside the cache/memory they are meant for
# (typically, this means they should be half the size of that system)

LIKWID_GROUP=( L2          L3         MEM)
       SIZES=( 100         2000       2000000 )
#      ITERS=( 2500000000  100000000  1000    ) # 20 minutes or more each
       ITERS=( 25000000    1000000    100     ) # about 15 seconds each
#      ITERS=( 2000000     100000     10      ) # less than 3 seconds each

export LD_LIBRARY_PATH=$LIKWID_PATH/lib:$LD_LIBRARY_PATH

#### Bandwidth

for test_num in $(seq 0 $(($NUM_TESTS - 1)) ); do
  for i in $(seq 0 $(($NUM_LEVELS-1)) ); do
    echo
    echo
    echo "${TESTS[$test_num]} Bandwidth for ${LIKWID_GROUP[$i]}. " \
      "Size: ${SIZES[$i]}, num_iter: ${ITERS[$i]}."
    echo
  
    SECONDS=0
  
    $LIKWID_PATH/bin/likwid-perfctr -C S0:0-$(( $NUM_CORES - 1 )) \
        -g ${LIKWID_GROUP[$i]} -m -M 1 $LIKWID_PATH/bin/likwid-bench \
        -t copy_avx -i ${ITERS[$i]} -w S0:${SIZES[$i]}kB:$NUM_CORES \
      | grep -B 1 -A 20 -E "Metric.*Sum"
  
    # time
    echo "$(($SECONDS / 3600))h $((($SECONDS / 60) % 60))m $(($SECONDS % 60))s"\
      " elapsed."
  done
done

