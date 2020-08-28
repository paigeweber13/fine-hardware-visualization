#!/bin/bash

LIKWID_PREFIX=/usr/local/likwid-master
POLY_PARAMS_CPU_STRESS="67108864 100 80"
POLY_PARAMS_MEM_STRESS="67108864 1 800"
LIKWID_EXEC=$LIKWID_PREFIX/bin/likwid-perfctr 
POLY_BASIC_EXEC=./polynomial_likwid
POLY_OPT_EXEC=./polynomial_block_likwid

DATE=$(date +%F_%H%M)

# perfgroups taken from output of `likwid-perfctr -a` for skylake
PERFGROUPS="CYCLE_ACTIVITY MEM_SP ICACHE TMA RECOVERY UOPS_EXEC L3 UOPS_RETIRE MEM FLOPS_DP L3CACHE CYCLE_STALLS FALSE_SHARE DIVIDE FLOPS_SP PORT_USAGE_TEST CLOCK ENERGY PORT_USAGE2 L2CACHE UOPS_ISSUE PORT_USAGE3 FLOPS_AVX TLB_DATA PORT_USAGE1 TLB_INSTR L2 MEM_DP BRANCH UOPS DATA"

export LD_LIBRARY_PATH=$LIKWID_PREFIX/lib

# create directory for results
dir=data/perfgroup-investigation-$DATE
mkdir -p $dir

for perfgroup in $PERFGROUPS; do 

  # 31 perfgroups to test
  # 2 tests per exec (cpu/mem)
  # 2 execs (basic/optimized)
  # 
  # CPU heavy basic : ~05m02s
  # CPU heavy opt   : ~00m32s
  # MEM heavy basic : ~22m16s
  # MEM heavy opt   : ~00m45s
  #
  # so big time-eater is 22m mem heavy basic run. About 30 minutes for
  # everything. With 31 perfgroups, this would take 15.5 hours. This can be run
  # overnight. Alternatively, if we reduce mem-heavy parameters to 
  # `67108864 1 400`, basic code runs in ~11m16. This would reduce runtime to 
  # 9 hours... Let's just keep max params.

  likwid_cmd="$LIKWID_EXEC -C S0:0-3 -g $perfgroup -M 1 -m "
  
  # for poly_exec in POLY_BASIC_EXEC POLY_OPT_EXEC; do
  #   for poly_params in POLY_PARAMS_CPU_STRESS POLY_PARAMS_MEM_STRESS; do
  #     full_cmd=$likwid_cmd $poly_exec $poly_params
  #     echo $full_cmd
  #     $full_cmd
  #   done
  # done

  basic_cpu_cmd="$likwid_cmd $POLY_BASIC_EXEC $POLY_PARAMS_CPU_STRESS"
  basic_cpu_cmd_file="$dir/poly_basic_$(echo $perfgroup)_cpu.txt"

  opt_cpu_cmd="$likwid_cmd $POLY_OPT_EXEC $POLY_PARAMS_CPU_STRESS" 
  opt_cpu_cmd_file="$dir/poly_opt_$(echo $perfgroup)_cpu.txt"

  basic_mem_cmd="$likwid_cmd $POLY_BASIC_EXEC $POLY_PARAMS_MEM_STRESS"
  basic_mem_cmd_file="$dir/poly_basic_$(echo $perfgroup)_mem.txt"

  opt_mem_cmd="$likwid_cmd $POLY_OPT_EXEC $POLY_PARAMS_MEM_STRESS"
  opt_mem_cmd_file="$dir/poly_opt_$(echo $perfgroup)_mem.txt"

  echo $basic_cpu_cmd
  echo "Redirecting command output (including stderr) to $basic_cpu_cmd_file"
  $basic_cpu_cmd > $basic_cpu_cmd_file 2>&1

  echo $opt_cpu_cmd
  echo "Redirecting command output (including stderr) to $opt_cpu_cmd_file"
  $opt_cpu_cmd > $opt_cpu_cmd_file 2>&1

  echo $basic_mem_cmd
  echo "Redirecting command output (including stderr) to $basic_mem_cmd_file"
  $basic_mem_cmd > $basic_mem_cmd_file 2>&1

  echo $opt_mem_cmd
  echo "Redirecting command output (including stderr) to $opt_mem_cmd_file"
  $opt_mem_cmd > $opt_mem_cmd_file 2>&1
done


unset LD_LIBRARY_PATH
