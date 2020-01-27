#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <omp.h>

#include "../lib/computation_measurements.h"


int main(int argc, char* argv[])
{
  // must match the same definition in functions.cpp
  std::uint64_t FLOAT_NUM_ITERATIONS = 100000000;
  std::uint64_t FLOP_PER_ITERATION = 160; //multiply and add to every float, 8 floats in a register

  std::uint64_t INT_NUM_ITERATIONS = 1000000000;
  std::uint64_t IOP_PER_ITERATION = 80; //32 adds + 32 adds + 16 muls

  std::uint64_t NUM_CORES = 16;

  __m256 d;
  __m256i e;

  // FLOPS
  auto start_time = std::chrono::high_resolution_clock::now();
  #pragma omp parallel
  {
    // std::cout << "I am processor #" << omp_get_thread_num() << std::endl;
      d = flops(FLOAT_NUM_ITERATIONS);
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
  
  std::uint64_t total_float_ops = FLOAT_NUM_ITERATIONS * FLOP_PER_ITERATION * NUM_CORES;

  // std::cout << "time taken: " << duration*1.0e-6 << " seconds." << std::endl;
  // std::cout << "total floating point operations: " << total_float_ops << std::endl;
  std::cout << total_float_ops / (duration*1.0e6) << " TFlop/s" << std::endl;

  // IOPS
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