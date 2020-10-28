#!/bin/bash
# start in root directory of this project

make
cd examples/polynomial_expansion
make
cd ../..

MEM_HEAVY_PARAMS="67108864 1 800"
CPU_HEAVY_PARAMS="67108864 1000 80"

export LD_LIBRARY_PATH=/usr/local/likwid-master/lib:~/code/fine-hardware-visualization/build/lib

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

bin/fhv -v visualizations/polynomial_basic_mem.json -o visualizations/polynomial_basic_mem_128_128_128_227_74_51.svg -c 128 128 128 227 74 51
bin/fhv -v visualizations/polynomial_basic_cpu.json -o visualizations/polynomial_basic_cpu_128_128_128_227_74_51.svg -c 128 128 128 227 74 51
bin/fhv -v visualizations/polynomial_block_mem.json -o visualizations/polynomial_block_mem_128_128_128_227_74_51.svg -c 128 128 128 227 74 51
bin/fhv -v visualizations/polynomial_block_cpu.json -o visualizations/polynomial_block_cpu_128_128_128_227_74_51.svg -c 128 128 128 227 74 51

bin/fhv -v visualizations/polynomial_basic_mem.json -o visualizations/polynomial_basic_mem_200_200_200_43_140_190.svg -c 200 200 200 43 140 190
bin/fhv -v visualizations/polynomial_basic_cpu.json -o visualizations/polynomial_basic_cpu_200_200_200_43_140_190.svg -c 200 200 200 43 140 190
bin/fhv -v visualizations/polynomial_block_mem.json -o visualizations/polynomial_block_mem_200_200_200_43_140_190.svg -c 200 200 200 43 140 190
bin/fhv -v visualizations/polynomial_block_cpu.json -o visualizations/polynomial_block_cpu_200_200_200_43_140_190.svg -c 200 200 200 43 140 190

bin/fhv -v visualizations/polynomial_basic_mem.json -o visualizations/polynomial_basic_mem_128_128_128_43_140_190.svg -c 128 128 128 43 140 190
bin/fhv -v visualizations/polynomial_basic_cpu.json -o visualizations/polynomial_basic_cpu_128_128_128_43_140_190.svg -c 128 128 128 43 140 190
bin/fhv -v visualizations/polynomial_block_mem.json -o visualizations/polynomial_block_mem_128_128_128_43_140_190.svg -c 128 128 128 43 140 190
bin/fhv -v visualizations/polynomial_block_cpu.json -o visualizations/polynomial_block_cpu_128_128_128_43_140_190.svg -c 128 128 128 43 140 190

bin/fhv -v visualizations/polynomial_basic_mem.json -o visualizations/polynomial_basic_mem_128_128_128_5_189_11.svg -c 128 128 128 5 189 11
bin/fhv -v visualizations/polynomial_basic_cpu.json -o visualizations/polynomial_basic_cpu_128_128_128_5_189_11.svg -c 128 128 128 5 189 11
bin/fhv -v visualizations/polynomial_block_mem.json -o visualizations/polynomial_block_mem_128_128_128_5_189_11.svg -c 128 128 128 5 189 11
bin/fhv -v visualizations/polynomial_block_cpu.json -o visualizations/polynomial_block_cpu_128_128_128_5_189_11.svg -c 128 128 128 5 189 11

bin/fhv -v visualizations/polynomial_basic_mem.json -o visualizations/polynomial_basic_mem_128_128_128_5_63_189.svg -c 128 128 128 5 63 189
bin/fhv -v visualizations/polynomial_basic_cpu.json -o visualizations/polynomial_basic_cpu_128_128_128_5_63_189.svg -c 128 128 128 5 63 189
bin/fhv -v visualizations/polynomial_block_mem.json -o visualizations/polynomial_block_mem_128_128_128_5_63_189.svg -c 128 128 128 5 63 189
bin/fhv -v visualizations/polynomial_block_cpu.json -o visualizations/polynomial_block_cpu_128_128_128_5_63_189.svg -c 128 128 128 5 63 189

bin/fhv -v visualizations/polynomial_basic_mem.json -o visualizations/polynomial_basic_mem_227_74_51_43_140_190.svg -c 227 74 51 43 140 190
bin/fhv -v visualizations/polynomial_basic_cpu.json -o visualizations/polynomial_basic_cpu_227_74_51_43_140_190.svg -c 227 74 51 43 140 190
bin/fhv -v visualizations/polynomial_block_mem.json -o visualizations/polynomial_block_mem_227_74_51_43_140_190.svg -c 227 74 51 43 140 190
bin/fhv -v visualizations/polynomial_block_cpu.json -o visualizations/polynomial_block_cpu_227_74_51_43_140_190.svg -c 227 74 51 43 140 190
