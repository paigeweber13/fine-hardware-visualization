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

extern "C" void peakflops_avx_fma(ull, double*);
extern "C" void peakflops_sp_avx_fma(ull, double*);

#define NUMKERNELS 5

#define peakflops_sp_avx_fma_kernel 4

static const TestCase kernels[NUMKERNELS] = {
  // {"copy_avx" , STREAM_2, DOUBLE, 16, &copy_avx, 0, 16, "Double-precision "
  //   "vector copy, optimized for AVX", 1, 1, -1, 16, 11, 14},
  // {"load_avx" , STREAM_1, DOUBLE, 16, &load_avx, 0, 8, "Double-precision load,"
  //   " optimized for AVX", 1, 0, -1, 16, 7, 6},
  // {"store_avx" , STREAM_1, DOUBLE, 16, &store_avx, 0, 8, "Double-precision "
  //   "store, optimized for AVX", 0, 1, -1, 20, 7, 10},
  // {"peakflops_avx_fma" , STREAM_1, DOUBLE, 4, &peakflops_avx_fma, 30, 8,
  //   "Double-precision multiplications and additions with a single load, "
  //   "optimized for AVX FMAs", 1, 0, -1, 32, 19, 18},
  {"peakflops_sp_avx_fma" , STREAM_1, SINGLE, 8, NULL, 30, 4,
    "Single-precision multiplications and additions with a single load, "
    "optimized for AVX FMAs", 1, 0, -1, 32, 19, 18},
};

void peakflops_manual(ull num_i, ull n, double* array) {
  auto kernel = kernels[peakflops_sp_avx_fma_kernel];

  auto start = std::chrono::steady_clock::now();
  for (ull i = 0; i < num_i; i++) {
    peakflops_sp_avx_fma(n, array);
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> duration = end-start;
  std::cout << "Size: " << n << ", Duration (s): " << duration.count()
    << "FLOP/s: " 
    << (double)num_i * (double)n * (double)kernels->flops / duration.count()
    << std::endl;
}

int main(int argc, char** argv) {

  // c

  // const ull n = 1024;
  // double *array = (double*)malloc(n * sizeof(double));
  // peakflops_avx_fma(n, array);
  // free(array);

  // c++

  // n above ~10,000 cause segfault?
  const ull n = 8192;
  const ull num_i = 1000000;
  double *array = new double[n];

  peakflops_manual(num_i, n, array);

  delete[] array;
}
