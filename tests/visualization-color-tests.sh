#!/bin/bash

make
cd examples/polynomial_expansion
make
cd ../..

OLD_FHV_OUTPUT=$FHV_OUTPUT
OLD_OMP_SCHEDULE=$OMP_SCHEDULE

echo "running basic polynomial to saturate memory"
export FHV_OUTPUT="visualizations/polynomial_basic_mem.json"
export OMP_SCHEDULE="dynamic,8"
examples/polynomial_expansion/polynomial_likwid 67108864 1 800 > /dev/null

echo "running basic polynomial to saturate cpu"
export FHV_OUTPUT="visualizations/polynomial_basic_cpu.json"
export OMP_SCHEDULE="dynamic,8"
examples/polynomial_expansion/polynomial_likwid 67108864 1000 80 > /dev/null

echo "running optimized polynomial_block to saturate memory"
export FHV_OUTPUT="visualizations/polynomial_block_mem.json" 
export OMP_SCHEDULE="dynamic,8"
examples/polynomial_expansion/polynomial_block_likwid 67108864 1 800 > /dev/null

echo "running optimized polynomial_block to saturate cpu"
export FHV_OUTPUT="visualizations/polynomial_block_cpu.json"
export OMP_SCHEDULE="dynamic,8"
examples/polynomial_expansion/polynomial_block_likwid 67108864 1000 80 > /dev/null

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

# cleanup
export FHV_OUTPUT=$OLD_FHV_OUTPUT
export OMP_SCHEDULE=$OLD_FHV_OUTPUT

