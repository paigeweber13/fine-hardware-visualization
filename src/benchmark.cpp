#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <omp.h>

#include "../lib/computation_measurements.h"
#include "../lib/performance_monitor.h"


int main(int argc, char* argv[])
{
  //                                         100 000 000 one hundred million
  const std::uint64_t FLOAT_NUM_ITERATIONS = 100000000;

  // we do 5 fma, so 10 total operations on 8 floats each
  const std::uint64_t FLOP_PER_ITERATION = 64; 

  // should total 8 billion flops, getting 4 billion from likwid
  // likwid counts FMA as a single operation?

  const std::uint64_t INT_NUM_ITERATIONS = 1000000000;
  const std::uint64_t IOP_PER_ITERATION = 80; //32 adds + 32 adds + 16 muls

  std::uint64_t NUM_CORES;

  __m256 d;
  __m256i e;

  performance_monitor perfmon;

  // FLOPS ----------------------------
  perfmon.init("FLOPS_SP");
  // perfmon.init("FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE:PMC0");
  auto start_time = std::chrono::high_resolution_clock::now();
  #pragma omp parallel
  {
    NUM_CORES = omp_get_num_threads();
    // std::cout << "I am processor #" << omp_get_thread_num() << std::endl;

    perfmon.startRegion("flops");
    d = flops(FLOAT_NUM_ITERATIONS);
    perfmon.stopRegion("flops");
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
  const double microseconds_to_seconds = 1e-6;

  perfmon.close();
  
  double total_float_ops = FLOAT_NUM_ITERATIONS * FLOP_PER_ITERATION * NUM_CORES;
  const double flops_to_tflops = 1e-12;

  std::cout << "time taken: " << duration*1.0e-6 << " seconds." << std::endl;
  std::cout << "total floating point operations: " << total_float_ops << std::endl;
  std::cout << (total_float_ops*flops_to_tflops) / (duration*microseconds_to_seconds) << " TFlop/s" << std::endl;

  // IOPS ----------------------------
  start_time = std::chrono::high_resolution_clock::now();
  #pragma omp parallel
  {
      e = iops(INT_NUM_ITERATIONS);
  }
  end_time = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
  
  std::uint64_t total_int_ops = INT_NUM_ITERATIONS * IOP_PER_ITERATION * NUM_CORES;

  // std::cout << "time taken: " << duration*1.0e-6 << " seconds." << std::endl;
  // std::cout << "total integer operations: " << total_int_ops << std::endl;
  std::cout << total_int_ops / (duration*1.0e6) << " TIop/s" << std::endl;
}