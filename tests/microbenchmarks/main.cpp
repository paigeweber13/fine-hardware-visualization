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

struct flopsResult
{
  ull numFlops;
  double megaFlopsRate;
};

const std::string FHV_REGION_PEAKFLOPS = "peakflops_sp_avx_fma";
const std::string FHV_REGION_PEAKFLOPS_PARALLEL = 
  "peakflops_sp_avx_fma_parallel";

const unsigned BYTES_PER_SP_FLOAT = 4;

// Kernels we're interested in, taken from likwid/bench/GCC/testcases.h
const size_t NUMKERNELS = 5;
const size_t peakflops_sp_avx_fma_kernel = 4;

static const TestCase kernels[NUMKERNELS] = {
  {(char*)"copy_avx" , STREAM_2, DOUBLE, 16, NULL, 0, 16, 
    (char*)"Double-precision vector copy, optimized for AVX", 1, 1, -1, 16,
    11, 14},
  {(char*)"load_avx" , STREAM_1, DOUBLE, 16, NULL, 0, 8, 
    (char*)"Double-precision load, optimized for AVX", 1, 0, -1, 16, 7, 6},
  {(char*)"store_avx" , STREAM_1, DOUBLE, 16, NULL, 0, 8, 
    (char*)"Double-precision store, optimized for AVX", 0, 1, -1, 20, 7, 10},
  {(char*)"peakflops_avx_fma" , STREAM_1, DOUBLE, 4, NULL, 30 /*flops*/, 8,
    (char*)"Double-precision multiplications and additions with a single "
    "load, optimized for AVX FMAs", 1, 0, -1, 32, 19, 18},
  {(char*)"peakflops_sp_avx_fma" , STREAM_1, SINGLE, 8, NULL, 30 /*flops*/, 4,
    (char*)"Single-precision multiplications and additions with a single "
    "load, optimized for AVX FMAs", 1, 0, -1, 32, 19, 18},
};

extern "C" void copy_avx();
extern "C" void load_avx();
extern "C" void store_avx();
extern "C" void peakflops_avx_fma(ull, double*);
extern "C" void peakflops_sp_avx_fma(ull, float*);

flopsResult peakflops_manual_parallel(ull num_i, ull n, float* array) {
  const double FLOPS_TO_MFLOPS = 1e-6;

  auto kernel = kernels[peakflops_sp_avx_fma_kernel];

  unsigned num_procs = 1;
  #pragma omp parallel
  {
    #pragma omp single
    {
      num_procs = omp_get_num_threads();
    }
  }

  ull * numFlopsPerThread = new ull[num_procs];
  double * megaFlopsRatePerThread = new double[num_procs];

  // this method of timing waits until all threads join before timing is
  // stopped. Have each thread track its own flop rate and then sum them
  #pragma omp parallel
  {
    auto start = std::chrono::steady_clock::now();
    for (ull i = 0; i < num_i; i++) {
      peakflops_sp_avx_fma(n, array);
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = end-start;

    auto me = omp_get_thread_num();
    numFlopsPerThread[me] = num_i * n * (ull)kernel.flops;
    megaFlopsRatePerThread[me] = (double)numFlopsPerThread[me] * 
      FLOPS_TO_MFLOPS / duration.count();
  }

  ull numFlops = 0;
  double megaFlopsRate = 0;

  for (unsigned i = 0; i < num_procs; i++){
    numFlops += numFlopsPerThread[i];
    megaFlopsRate += megaFlopsRatePerThread[i];
  }

  delete[] numFlopsPerThread;
  delete[] megaFlopsRatePerThread;

  return flopsResult{numFlops, megaFlopsRate};
}

flopsResult peakflops_fhv_parallel(ull num_i, ull n, float* array,
    bool output_to_json=true) {
  const ull FLOPS_PER_AVX_OP = 8;

  // unused: remove
  // const ull FLOPS_PER_FMA_OP = 2;

  // groups measured by default (all are necessary to create visualization):
  // "MEM_DP|FLOPS_SP|L3|L2|PORT_USAGE1|PORT_USAGE2|PORT_USAGE3"
  const ull NUM_GROUPS_FOR_FHV_VISUALIZATION = 7;

  fhv_perfmon::init(FHV_REGION_PEAKFLOPS_PARALLEL);

  if (num_i < NUM_GROUPS_FOR_FHV_VISUALIZATION) {
    std::cout << "ERROR: in peakflops_fhv_parallel: num_i must be >= "
      << NUM_GROUPS_FOR_FHV_VISUALIZATION
      << std::endl;
    return flopsResult{};
  }
  num_i /= NUM_GROUPS_FOR_FHV_VISUALIZATION;

  for (ull i = 0; i < NUM_GROUPS_FOR_FHV_VISUALIZATION; i++) {
    #pragma omp parallel 
    {
      fhv_perfmon::startRegion(FHV_REGION_PEAKFLOPS_PARALLEL.c_str());
      for (ull i = 0; i < num_i; i++) {
        peakflops_sp_avx_fma(n, array);
      }
      fhv_perfmon::stopRegion(FHV_REGION_PEAKFLOPS_PARALLEL.c_str());
    }
    fhv_perfmon::nextGroup();
  }

  fhv_perfmon::close();
  // fhv_perfmon::printAggregateResults();

  if (output_to_json) {
    std::string paramString = "array n: " + std::to_string(n) + 
      " array size bytes: " + std::to_string(n * BYTES_PER_SP_FLOAT) +
      " num iterations: " + std::to_string(num_i);
    fhv_perfmon::resultsToJson(paramString);
  }

  auto results = fhv_perfmon::get_aggregate_results();
  ull numFlops = 0;
  double megaFlopsRate = 0;

  for (const auto &result : results) {
    if (result.region_name == FHV_REGION_PEAKFLOPS_PARALLEL
      && result.aggregation_type == fhv::types::aggregation_t::sum
      && result.result_name == sp_avx_256_flops_event_name){
        numFlops = result.result_value * NUM_GROUPS_FOR_FHV_VISUALIZATION * FLOPS_PER_AVX_OP;

    }
    else if (result.region_name == FHV_REGION_PEAKFLOPS_PARALLEL
      && result.aggregation_type == fhv::types::aggregation_t::sum
      && result.result_name == mflops_metric_name){
        megaFlopsRate = result.result_value;
    }

    // if we've already found things, don't keep looking
    if (numFlops != 0 && megaFlopsRate != 0) break;
  }

  return flopsResult{numFlops, megaFlopsRate};
}


int main(int argc, char** argv) {
  // measurement types
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " array_n num_iter" 
      << std::endl;
    std::cout << "  program will print out a comparison between manual "
      << "benchmark results and fhv results in CSV format. The format is "
      << "described below:"
      << std::endl;
    std::cout << "  array_n,array_size_bytes,num_i,"
      << "manual_reported_num_flops,fhv_reported_num_flops,"
      << "manual_reported_Mflop_rate,fhv_reported_Mflop_rate,"
      << "num_flops_diff_factor,Mflop_rate_diff_factor"
      << std::endl;
    return 0;
  }

  ull n = std::stoull(argv[1], NULL);
  ull num_i = std::stoull(argv[2], NULL);

  if (num_i < 1000) {
    std::cout << "WARNING: num_i should be above 1000 to mininmize error "
      << "between fhv and manual reporting. Fhv reporting must run a "
      << "multiple of seven times, so if num_i is small the error from "
      << "integer division will be high."
      << std::endl;
  }

  float *array = (float*)aligned_alloc(64, n * sizeof(float));

  auto resultManual = peakflops_manual_parallel(num_i, n, array);
  auto resultFhv = peakflops_fhv_parallel(num_i, n, array);

  ull diffNumFlops = resultManual.numFlops < resultFhv.numFlops 
    ? resultFhv.numFlops - resultManual.numFlops 
    : resultManual.numFlops - resultFhv.numFlops;
  double diffFlopsRate = resultManual.megaFlopsRate - resultFhv.megaFlopsRate 
    ? resultFhv.megaFlopsRate - resultManual.megaFlopsRate 
    : resultManual.megaFlopsRate - resultFhv.megaFlopsRate;

  // array_n,array_size_bytes,
  std::cout 
    << n << ","
    << n * BYTES_PER_SP_FLOAT << ","
    << num_i << ","
    << resultManual.numFlops << ","
    << resultFhv.numFlops << ","
    << resultManual.megaFlopsRate << ","
    << resultFhv.megaFlopsRate << ","
    << (double)diffNumFlops / (double)resultManual.numFlops << ","
    << diffFlopsRate / resultManual.megaFlopsRate << ","
    << std::endl;

  free(array);
}
