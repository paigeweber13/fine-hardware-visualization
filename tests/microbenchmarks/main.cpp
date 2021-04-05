// third-party imports
#include <iostream>

// first-party imports
#include <fhv_perfmon.hpp>

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
 * Kernels we're interested in, taken from likwid/bench/GCC/testcases.h. Each
 * of these is a struct of type TestCase:
 *
 * {"copy_avx" , STREAM_2, DOUBLE, 16, &copy_avx, 0, 16, "Double-precision
 *   vector copy, optimized for AVX", 1, 1, -1, 16, 11, 14},
 * {"load_avx" , STREAM_1, DOUBLE, 16, &load_avx, 0, 8, "Double-precision load,
 *   optimized for AVX", 1, 0, -1, 16, 7, 6},
 * {"store_avx" , STREAM_1, DOUBLE, 16, &store_avx, 0, 8, "Double-precision
 *   store, optimized for AVX", 0, 1, -1, 20, 7, 10},
 * {"peakflops_avx_fma" , STREAM_1, DOUBLE, 4, &peakflops_avx_fma, 30, 8,
 *   "Double-precision multiplications and additions with a single load,
 *   optimized for AVX FMAs", 1, 0, -1, 32, 19, 18},
 * {"peakflops_sp_avx_fma" , STREAM_1, SINGLE, 8, &peakflops_sp_avx_fma, 30, 4,
 *   "Single-precision multiplications and additions with a single load,
 *   optimized for AVX FMAs", 1, 0, -1, 32, 19, 18},
 *
 * 
 * seems to be possible to execute these functions with func(size, stream)
 * where size is an unsigned long long and stream is a pointer to memory
 *
 */

int main(int argc, char** argv) {
  std::cout << "Hello, World!" << std::endl;

  return 0;
}
