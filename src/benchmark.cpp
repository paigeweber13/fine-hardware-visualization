#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <omp.h>

#include "../lib/computation_measurements.h"
#include "../lib/performance_monitor.h"

const std::uint64_t FLOAT_NUM_ITERATIONS_SHORT = 1000000000;
const std::uint64_t FLOAT_NUM_ITERATIONS =       10000000000;
const unsigned num_iter = 5;

int main(int argc, char* argv[])
{
  __m256 d;
  __m256i e;

  performance_monitor perfmon;

  // FLOPS ----------------------------
  perfmon.init("FLOPS_SP");
  std::cout << "starting single precision flop benchmark" << std::endl;
  // perfmon.init("FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE:PMC0");
  #pragma omp parallel
  {
    perfmon.startRegion("flops");
    // #pragma omp barrier
    d = flops(FLOAT_NUM_ITERATIONS);
    // #pragma omp barrier
    perfmon.stopRegion("flops");
  }

  likwid_markerNextGroup();
  std::cout << "starting rw bandwidth benchmark" << std::endl;
  bandwidth_rw(10000, 100);

  perfmon.close();
  perfmon.printOnlyAggregate();
}