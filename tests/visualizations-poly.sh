#!/bin/bash
# start in root directory of this project

make
cd examples/polynomial_expansion
make
cd ../..

MEM_HEAVY_PARAMS="67108864 1 800"
CPU_HEAVY_PARAMS="67108864 1000 80"

export LD_LIBRARY_PATH="/usr/local/likwid-master/lib"

echo "running basic polynomial to saturate memory"
export FHV_OUTPUT="visualizations/polynomial_basic_mem.json"
export OMP_SCHEDULE="dynamic,8"
examples/polynomial_expansion/polynomial_fhv_perfmon $MEM_HEAVY_PARAMS > /dev/null

echo "running basic polynomial to saturate cpu"
export FHV_OUTPUT="visualizations/polynomial_basic_cpu.json"
export OMP_SCHEDULE="dynamic,8"
examples/polynomial_expansion/polynomial_fhv_perfmon $CPU_HEAVY_PARAMS > /dev/null

echo "running optimized polynomial_block to saturate memory"
export FHV_OUTPUT="visualizations/polynomial_block_mem.json" 
export OMP_SCHEDULE="dynamic,8"
examples/polynomial_expansion/polynomial_block_fhv_perfmon $MEM_HEAVY_PARAMS > /dev/null

echo "running optimized polynomial_block to saturate cpu"
export FHV_OUTPUT="visualizations/polynomial_block_cpu.json"
export OMP_SCHEDULE="dynamic,8"
examples/polynomial_expansion/polynomial_block_fhv_perfmon $CPU_HEAVY_PARAMS > /dev/null

bin/fhv -v visualizations/polynomial_basic_mem.json -o visualizations/polynomial_basic_mem.svg
bin/fhv -v visualizations/polynomial_basic_cpu.json -o visualizations/polynomial_basic_cpu.svg
bin/fhv -v visualizations/polynomial_block_mem.json -o visualizations/polynomial_block_mem.svg
bin/fhv -v visualizations/polynomial_block_cpu.json -o visualizations/polynomial_block_cpu.svg
