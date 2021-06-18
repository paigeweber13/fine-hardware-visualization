#pragma once

// stl
#include <chrono>
#include <iostream>
#include <string>

// likwid, fhv
#include <fhv_perfmon.hpp>
#include <test_types.h>

// openmp
#include <omp.h>

// this application
#include "types.hpp"

extern "C" void peakflops_sp_avx_fma(ull, float*);

const unsigned BYTES_PER_SP_FLOAT = 4;

const std::string FHV_REGION_PEAKFLOPS_SP_PARALLEL = 
  "peakflops_sp_avx_fma_parallel";

const TestCase peakflops_sp_avx_fma_kernel = 
  {(char*)"peakflops_sp_avx_fma" , STREAM_1, SINGLE, 8, NULL, 30 /*flops*/, 4,
    (char*)"Single-precision multiplications and additions with a single "
    "load, optimized for AVX FMAs", 1, 0, -1, 32, 19, 18};

flopsResult peakflops_sp_manual_parallel(ull num_i, ull n, float* array);
flopsResult peakflops_sp_fhv_parallel(ull num_i, ull n, float* array,
  bool output_to_json=true);
