#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <omp.h>

#include "../lib/computation_measurements.h"
#include "../lib/performance_monitor.h"

const std::uint64_t FLOAT_NUM_ITERATIONS_SHORT = 1000000000;
const std::uint64_t FLOAT_NUM_ITERATIONS =       10000000000;

void benchmark_sp_flops(performance_monitor perfmon)
{
  __m256 d;
  __m256i e;

  perfmon.init("FLOPS_SP");
  std::cout << "starting single precision flop benchmark" << std::endl;
#pragma omp parallel
  {
    perfmon.startRegion("flops");
    d = flops(FLOAT_NUM_ITERATIONS);
    perfmon.stopRegion("flops");
  }
  perfmon.close();
  // perfmon.printDetailedResults();
  perfmon.printOnlyAggregate();
  perfmon.printComparison();
}

void benchmark_l2_bw(performance_monitor perfmon){
  perfmon.init("L2");
  std::cout << "starting L2 rw bandwidth benchmark" << std::endl;
  // 10000 iterations for a good average
  // 100 kilobytes to fit well inside L2 cache
  bandwidth_rw(10000, 100);

  perfmon.close();
  // perfmon.printDetailedResults();
  perfmon.printOnlyAggregate();
  perfmon.printComparison();
}

void benchmark_l3_bw(performance_monitor perfmon){
  perfmon.init("L3");
  std::cout << "starting L3 rw bandwidth benchmark" << std::endl;
  // 1000 iterations for a good average
  // 1000 kilobytes to fit well inside L3 cache
  bandwidth_rw(1000, 1000);

  perfmon.close();
  // perfmon.printDetailedResults();
  perfmon.printOnlyAggregate();
  perfmon.printComparison();
}

void benchmark_ram_bw(performance_monitor perfmon){
  perfmon.init("MEM_DP");
  std::cout << "starting RAM rw bandwidth benchmark" << std::endl;
  // 10 iterations for a good average
  // 100000 kilobytes (100MB) so it can't all be cached
  bandwidth_rw(10, 100000);

  perfmon.close();
  // perfmon.printDetailedResults();
  perfmon.printOnlyAggregate();
  perfmon.printComparison();
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " 0|1" << std::endl;
    std::cout << "       0 for FLOPS_SP benchmark" << std::endl;
    std::cout << "       1 for L2 r/w bandwidth benchmark" << std::endl;
    return 1;
  }

  int choice = std::stoi(argv[1]);
  performance_monitor perfmon;

  switch (choice)
  {
  case 0:
    benchmark_sp_flops(perfmon);
    break;
  case 1:
    benchmark_l2_bw(perfmon);
    break;
  case 2:
    benchmark_l3_bw(perfmon);
    break;
  case 3:
    benchmark_ram_bw(perfmon);
    break;
  }
}