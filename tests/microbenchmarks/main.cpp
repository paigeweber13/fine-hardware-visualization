// third-party imports
#include <chrono>
#include <iostream>
// #include <stdlib.h>
// #include <stdio.h>

// first-party imports
#include <fhv_perfmon.hpp>
#include <test_types.h>
// #include <testcases.h>

/* 
 * struct for test case data. Described in likwid/bench/includes/test_types.h.
 * 
 * typedef struct {
 *     char* name;
 *     Pattern streams;
 *     DataType type;
 *     int stride;
 *     FuncPrototype kernel;
 *     int  flops;
 *     int  bytes;
 *     char* desc;
 *     int loads;
 *     int stores;
 *     int branches;
 *     int instr_const;
 *     int instr_loop;
 *     int uops;
 *     int loadstores;
 *     void* dlhandle;
 * } TestCase;
 * 
 * 
 * seems to be possible to execute these functions with func(size, stream)
 * where size is an unsigned long long and stream is a pointer to memory
 *
 */

typedef unsigned long long ull;

// Kernels we're interested in, taken from likwid/bench/GCC/testcases.h

extern "C" void copy_avx();
extern "C" void load_avx();
extern "C" void store_avx();
extern "C" void peakflops_avx_fma(ull, double*);
extern "C" void peakflops_sp_avx_fma(ull, float*);

#define NUMKERNELS 5

#define peakflops_sp_avx_fma_kernel 4

static const TestCase kernels[NUMKERNELS] = {
  {(char*)"copy_avx" , STREAM_2, DOUBLE, 16, NULL, 0, 16, 
    (char*)"Double-precision vector copy, optimized for AVX", 1, 1, -1, 16,
    11, 14},
  {(char*)"load_avx" , STREAM_1, DOUBLE, 16, NULL, 0, 8, 
    (char*)"Double-precision load, optimized for AVX", 1, 0, -1, 16, 7, 6},
  {(char*)"store_avx" , STREAM_1, DOUBLE, 16, NULL, 0, 8, 
    (char*)"Double-precision store, optimized for AVX", 0, 1, -1, 20, 7, 10},
  {(char*)"peakflops_avx_fma" , STREAM_1, DOUBLE, 4, NULL, 30, 8,
    (char*)"Double-precision multiplications and additions with a single "
    "load, optimized for AVX FMAs", 1, 0, -1, 32, 19, 18},
  {(char*)"peakflops_sp_avx_fma" , STREAM_1, SINGLE, 8, NULL, 30, 4,
    (char*)"Single-precision multiplications and additions with a single "
    "load, optimized for AVX FMAs", 1, 0, -1, 32, 19, 18},
};

void peakflops_manual_parallel(ull num_i, ull n, float* array) {
  auto kernel = kernels[peakflops_sp_avx_fma_kernel];

  unsigned num_procs;
  #pragma omp parallel
  {
    #pragma omp single
    {
      num_procs = omp_get_num_threads();
    }
  }

  auto start = std::chrono::steady_clock::now();
  #pragma omp parallel for
  for (ull i = 0; i < num_i; i++) {
    peakflops_sp_avx_fma(n, array);
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> duration = end-start;

  double flops = (double)num_procs * (double)num_i * (double)n * 
    (double)kernel.flops / duration.count();

  std::cout << "Size: " << n << ", Duration (s): " << duration.count()
    << " FLOP/s: " << flops << std::endl;
}

void peakflops_manual(ull num_i, ull n, float* array, bool parallel=false) {
  auto kernel = kernels[peakflops_sp_avx_fma_kernel];

  auto start = std::chrono::steady_clock::now();
  for (ull i = 0; i < num_i; i++) {
    peakflops_sp_avx_fma(n, array);
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> duration = end-start;

  double flops = (double)num_i * (double)n * (double)kernel.flops / 
    duration.count();

  std::cout << "Size: " << n << ", Duration (s): " << duration.count()
    << " FLOP/s: " << flops << std::endl;
}

void peakflops_fhv(ull num_i, ull n, float* array) {

  fhv_perfmon::init("", "peakflops_sp_avx_fma");

  if (num_i < 10) {
    std::cout << "ERROR: in peakflops_fhv: num_i must be >= 10"
      << std::endl;
    return;
  }
  num_i /= 10;

  for (ull i = 0; i < 10; i++) {
    fhv_perfmon::startRegion("peakflops_sp_avx_fma");
    peakflops_sp_avx_fma(n, array);
    fhv_perfmon::stopRegion("peakflops_sp_avx_fma");
    fhv_perfmon::nextGroup();
  }

  fhv_perfmon::close();
  fhv_perfmon::printAggregateResults();
  const unsigned BYTES_PER_FLOAT = 4;
  std::string paramString = "array n: " + std::to_string(n) + 
    " array size bytes: " + std::to_string(n * BYTES_PER_FLOAT) +
    " num iterations: " + std::to_string(num_i);
  fhv_perfmon::resultsToJson(paramString);
}

int main(int argc, char** argv) {
  // if (argc < 3) {
  //   std::cout << "Usage: " << argv[0] << " measurement_type array_n num_iter" 
  //     << std::endl;
  //   std::cout << "       " << "measurement_type must be one of 'none', "
  //     << "'manual', manual_parallel', 'fhv', or 'fhv_parallel'."
  //     << std::endl;
  //   return 0;
  // }

  const ull max_n = 1000000001;
  ull num_i;
  float *array = (float*)aligned_alloc(64, max_n * sizeof(float));

  num_i = 100000000;
  for (ull n = 100; n < max_n; n *= 10) {
    peakflops_manual(num_i, n, array);
    num_i /= 10;
  }

  num_i = 100000000;
  for (ull n = 100; n < max_n; n *= 10) {
    peakflops_manual_parallel(num_i, n, array);
    num_i /= 10;
  }

  num_i = 100000000;
  ull n = 100;
  peakflops_fhv(num_i, n, array);

  free(array);
}

