#include "peakflops_sp_avx_fma.hpp"

// ------------ PEAKFLOPS SINGLE-PRECISION ------------ //
flopsResult peakflops_sp_manual_parallel(ull num_i, ull n, float* array) {
  const double FLOPS_TO_MFLOPS = 1e-6;

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
    numFlopsPerThread[me] = num_i * n * (ull)peakflops_sp_avx_fma_kernel.flops;
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

flopsResult peakflops_sp_fhv_parallel(ull num_i, ull n, float* array,
    bool output_to_json) {
  const ull FLOPS_PER_AVX_OP = 8;

  // unused: remove
  // const ull FLOPS_PER_FMA_OP = 2;

  // groups measured by default (all are necessary to create visualization):
  // "MEM_DP|FLOPS_SP|L3|L2|PORT_USAGE1|PORT_USAGE2"
  const ull NUM_GROUPS_FOR_FHV_VISUALIZATION = 7;

  fhv_perfmon::init(FHV_REGION_PEAKFLOPS_SP_PARALLEL);

  if (num_i < NUM_GROUPS_FOR_FHV_VISUALIZATION) {
    std::cout << "ERROR: in peakflops_sp_fhv_parallel: num_i must be >= "
      << NUM_GROUPS_FOR_FHV_VISUALIZATION
      << std::endl;
    return flopsResult{};
  }
  num_i /= NUM_GROUPS_FOR_FHV_VISUALIZATION;

  for (ull i = 0; i < NUM_GROUPS_FOR_FHV_VISUALIZATION; i++) {
    #pragma omp parallel 
    {
      fhv_perfmon::startRegion(FHV_REGION_PEAKFLOPS_SP_PARALLEL.c_str());
      for (ull i = 0; i < num_i; i++) {
        peakflops_sp_avx_fma(n, array);
      }
      fhv_perfmon::stopRegion(FHV_REGION_PEAKFLOPS_SP_PARALLEL.c_str());
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
    if (result.region_name == FHV_REGION_PEAKFLOPS_SP_PARALLEL
      && result.aggregation_type == fhv::types::aggregation_t::sum
      && result.result_name == sp_avx_256_flops_event_name){
        numFlops = result.result_value * NUM_GROUPS_FOR_FHV_VISUALIZATION * FLOPS_PER_AVX_OP;

    }
    else if (result.region_name == FHV_REGION_PEAKFLOPS_SP_PARALLEL
      && result.aggregation_type == fhv::types::aggregation_t::sum
      && result.result_name == mflops_sp_metric_name){
        megaFlopsRate = result.result_value;
    }

    // if we've already found things, don't keep looking
    if (numFlops != 0 && megaFlopsRate != 0) break;
  }

  return flopsResult{numFlops, megaFlopsRate};
}

