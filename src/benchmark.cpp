#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <omp.h>

#include "../lib/computation_measurements.h"
#include "../lib/performance_monitor.h"

const std::uint64_t FLOAT_NUM_ITERATIONS_SHORT = 1000000000;
const std::uint64_t FLOAT_NUM_ITERATIONS =       10000000000;

void benchmark_sp_flops()
{
  __m256 d;
  __m256i e;

  performance_monitor perfmon;

  perfmon.init("FLOPS_SP");
  std::cout << "starting single precision flop benchmark" << std::endl;
#pragma omp parallel
  {
    perfmon.startRegion("flops");
    d = flops(FLOAT_NUM_ITERATIONS);
    perfmon.stopRegion("flops");
  }
  perfmon.close();
  perfmon.printOnlyAggregate();
}

void benchmark_l2_bw(){
  performance_monitor perfmon;

  perfmon.init("L2");
  std::cout << "starting rw bandwidth benchmark" << std::endl;
  bandwidth_rw(10000, 100);

  perfmon.close();
  perfmon.printOnlyAggregate();
}

int main(int argc, char* argv[])
{
  benchmark_sp_flops();
}