/*
 * NOTE: THIS FILE HAS BEEN DEPRECATED
 * 
 * It was leftover from when fhv was also intended to run the benchmark to get
 * baseline performance. Since then that functionality has moved to the
 * `benchmark.sh` script, which is much more stable and consistent, as it
 * relies on the very well-written likwid-bench tool
 * 
 * This code is included for reference only, as it can give maintainers an idea
 * of how one might try to go about saturating FLOP/S
 * 
 * Note that the bandwidth benchmark was never very good and often gave results
 * that were off by as much as a factor of two.
 * 
 */

#include "computation_measurements.hpp"

__m256 flops_sp(std::uint64_t num_iterations)
{
  __m256 a = _mm256_setr_ps(1., 2., 3., 4., 5., 6., 7., 8.);
  __m256 b = _mm256_setr_ps(10., 20., 30., 40., 50., 60., 70., 80.);
  __m256 c = _mm256_setr_ps(11., 21., 31., 41., 51., 61., 71., 81.);

  __m256 d = _mm256_setr_ps(77., 27., 37., 47., 57., 67., 77., 87.);
  __m256 e = _mm256_setr_ps(88., 28., 38., 48., 58., 68., 78., 88.);
  __m256 f = _mm256_setr_ps(99., 29., 39., 49., 59., 69., 79., 89.);

  __m256 g = _mm256_setr_ps(22., 22., 32., 42., 52., 62., 72., 82.);
  __m256 h = _mm256_setr_ps(33., 23., 33., 43., 53., 63., 73., 83.);
  __m256 j = _mm256_setr_ps(44., 24., 34., 44., 54., 64., 74., 84.);

  __m256 k = _mm256_setr_ps(55., 25., 35., 45., 55., 65., 75., 85.);
  __m256 l = _mm256_setr_ps(66., 26., 36., 46., 56., 66., 76., 86.);
  __m256 m = _mm256_setr_ps(1111., 211., 311., 411., 511., 611., 711., 811.);

  for (std::uint64_t i = 0; i < num_iterations; i++)
  {
    // operations per loop iteration: 64
    // do four things per loop to allow for the processor to pipeline
    // instructions
    c = _mm256_fmadd_ps(a, b, c); // 2 ops on 8 floats
    f = _mm256_fmadd_ps(d, e, f); // 2 more ops on 8 floats
    j = _mm256_fmadd_ps(g, h, j); // 2 more ops on 8 floats
    m = _mm256_fmadd_ps(k, l, m); // 2 more ops on 8 floats
  }

  // return whatever is computed
  c = _mm256_fmadd_ps(f, j, c);
  a = _mm256_fmadd_ps(c, m, a);

  return a;
}

__m256d flops_dp(std::uint64_t num_iterations)
{
  __m256d a = _mm256_setr_pd(1., 2., 3., 4.);
  __m256d b = _mm256_setr_pd(10., 20., 30., 40.);
  __m256d c = _mm256_setr_pd(11., 21., 31., 41.);

  __m256d d = _mm256_setr_pd(77., 27., 37., 47.);
  __m256d e = _mm256_setr_pd(88., 28., 38., 48.);
  __m256d f = _mm256_setr_pd(99., 29., 39., 49.);

  __m256d g = _mm256_setr_pd(22., 22., 32., 42.);
  __m256d h = _mm256_setr_pd(33., 23., 33., 43.);
  __m256d j = _mm256_setr_pd(44., 24., 34., 44.);

  __m256d k = _mm256_setr_pd(55., 25., 35., 45.);
  __m256d l = _mm256_setr_pd(66., 26., 36., 46.);
  __m256d m = _mm256_setr_pd(1111., 211., 311., 411.);

  for (std::uint64_t i = 0; i < num_iterations; i++)
  {
    // operations per loop iteration: 64
    // do four things per loop to allow for the processor to pipeline
    // instructions
    c = _mm256_fmadd_pd(a, b, c); // 2 opd on 8 floats
    f = _mm256_fmadd_pd(d, e, f); // 2 more opd on 8 floats
    j = _mm256_fmadd_pd(g, h, j); // 2 more opd on 8 floats
    m = _mm256_fmadd_pd(k, l, m); // 2 more opd on 8 floats
  }

  // return whatever is computed
  c = _mm256_fmadd_pd(f, j, c);
  a = _mm256_fmadd_pd(c, m, a);

  return a;
}

double bandwidth_rw(const char *tag, std::uint64_t num_groups,
                    std::uint64_t num_iterations_per_group,
                    std::uint64_t num_iterations_to_measure,
                    std::uint64_t size_kib) {
  // std::cout << "params:" << std::endl
  //   << "  num_groups: " << num_groups << std::endl
  //   << "  num_iterations_per_group: " << num_iterations_per_group << std::endl
  //   << "  num_iterations_to_measure: " << num_iterations_to_measure << std::endl
  //   << "  size_kib: " << size_kib << std::endl;

  // unsigned thr_num;
  // __m256d buffer;

  // align to cache line, which is 512 bits or 64 bytes
  double * array = static_cast<double *>(
    aligned_alloc(64, size_kib * KILO_BYTE_SIZE));
  double * copy_array = static_cast<double *>(
    aligned_alloc(64, size_kib * KILO_BYTE_SIZE));

  std::uint64_t num_doubles = size_kib * KILO_BYTE_SIZE/BYTES_PER_DOUBLE;

  if (num_iterations_to_measure == 0) num_iterations_to_measure = 1;
  std::uint64_t outer_iter = std::max(
    num_iterations_per_group/num_iterations_to_measure, 
    static_cast<std::uint64_t>(3));

#pragma omp parallel
  {
    std::uint64_t i, j, k, group;

    for (group = 0; group < num_groups; group++)
    {
      // thr_num = omp_get_thread_num();
      for (i = 0; i < outer_iter; i++) {
        //Get time snapshot just for one iteration
        if (i == outer_iter/ 2) {
          //	start = system_clock::now();
          //	Maybe start likwid region here
          //    printf("likwid start region %s on thread %d\n", bw->mark_tag, omp_get_thread_num());
          
          fhv_perfmon::startRegion(tag);
        }
        for (k = 0; k < num_iterations_to_measure; k++){
          for (j = 0; j < num_doubles; j += 1){
            copy_array[j] = array[j];
          }
        }
          // for (k = 0; k < num_iterations_to_measure; k++)
          // {
          //   for (j = 0; j < num_doubles; j += DOUBLES_PER_VECTOR)
          //   {
          //     // Loading 256-bits into memory address of array
          //     buffer = _mm256_load_pd(array + j);
          //     // Storing 256-bits from buffer into address of cpy_arr
          //     _mm256_store_pd(copy_array + j, buffer);
          //   }
          // }
        //Get time snapshot just for one iteration
        if (i == outer_iter/ 2) {
          //	end = system_clock::now();
          //	Maybe stop likwid regin here
          //    printf("likwid stop region %s on thread %d\n", bw->mark_tag, omp_get_thread_num());
          
          fhv_perfmon::stopRegion(tag);
        }
        asm(""); //Say no to loop optimization
      }

      fhv_perfmon::nextGroup();
    }
  }

  double result = copy_array[static_cast<std::uint64_t>(rand()) % num_doubles];

  free(array);
  free(copy_array);

  return result;
}

#define BYTES_TO_GBYTES 1e-9

// ---- default values
const std::uint64_t FLOAT_NUM_ITERATIONS_SHORT = 100000000;
const std::uint64_t FLOAT_NUM_ITERATIONS =       10000000000;
const std::string DEFAULT_DISCRETE_SCALE = "RdPu";

// each argument vector has num_iterations, mem_size_kb

// 3000000 total iterations for a good average
// 1 measured iteration
// 100 kilobytes to fit well inside L2 cache
std::vector<std::uint64_t> l2_args = {3000000, 1, 100};

// 10000 iterations for a good average
// 1000 kilobytes to fit well inside L3 cache
std::vector<std::uint64_t> l3_args = {10000, 1, 1000};

// 20 iterations for a good average
// 100000 kilobytes (100MB) so it can't all be cached
std::vector<std::uint64_t> ram_args = {20, 1, 100000};

// enums
enum class output_format { pretty, csv };

// likwid results file path
const char *filepath = likwidOutputFilepath.c_str();

void benchmark_flops(precision p, uint64_t num_iter)
{
  if(p == precision::SINGLE_P){
    #pragma omp parallel
    {
      for(size_t i = 0; i < 7; i++)
      {
        fhv_perfmon::startRegion("flops_sp");
        flops_sp(num_iter);
        fhv_perfmon::stopRegion("flops_sp");
        #pragma omp barrier
        fhv_perfmon::nextGroup();
      }
    }
  } else if (p == precision::DOUBLE_P){
    #pragma omp parallel
    {
      for(size_t i = 0; i < 7; i++)
      {
        fhv_perfmon::startRegion("flops_dp");
        flops_dp(num_iter);
        fhv_perfmon::stopRegion("flops_dp");
        #pragma omp barrier
        fhv_perfmon::nextGroup();
      }
    }
  }
}

void benchmark_memory_bw(std::string memory_type, uint64_t num_iterations,
                         uint64_t mem_size_kb){
  // memory type becomes the performance group. Typically this is "L2", "L3",
  // or "RAM"
  bandwidth_rw(memory_type.c_str(), 1, num_iterations, 1, mem_size_kb);
}

void benchmark_all()
{

  fhv_perfmon::init("flops_sp,flops_dp,L2,L3,MEM", "",
                    "FLOPS_SP|FLOPS_DP|L2|L3|MEM");

  std::cout << "starting single precision flop benchmark" << std::endl;
  benchmark_flops(precision::SINGLE_P, FLOAT_NUM_ITERATIONS);

  std::cout << "starting double precision flop benchmark" << std::endl;
  benchmark_flops(precision::DOUBLE_P, FLOAT_NUM_ITERATIONS);

  std::cout << "starting L2 cache rw bandwidth benchmark" << std::endl;
  benchmark_memory_bw("L2", l2_args[0], l2_args[2]);

  std::cout << "starting L3 cache rw bandwidth benchmark" << std::endl;
  benchmark_memory_bw("L3", l3_args[0], l3_args[2]);

  std::cout << "starting RAM rw bandwidth benchmark" << std::endl;
  benchmark_memory_bw("MEM", ram_args[0], ram_args[2]);
}

void print_csv_header()
{
  std::cout << "Single iteration size,"
               "Number of iterations,"
               "Load to store ratio,"
               "Load volume by retired instructions [GBytes],"
               "Store volume by retired instructions [GBytes],"
               "Total volume by retired instructions [GBytes],"
               "L2 bandwidth [MBytes/s],"
               "L2 data volume [GBytes],"
               "L2D evict bandwidth [MBytes/s],"
               "L2D evict data volume [GBytes],"
               "L2D load bandwidth [MBytes/s],"
               "L2D load data volume [GBytes],"
               "L3 bandwidth [MBytes/s],"
               "L3 data volume [GBytes],"
               "L3 evict bandwidth [MBytes/s],"
               "L3 evict data volume [GBytes],"
               "L3 load bandwidth [MBytes/s],"
               "L3 load data volume [GBytes],"
               "Memory bandwidth [MBytes/s],"
               "Memory data volume [GBytes],"
               "Memory evict bandwidth [MBytes/s],"
               "Memory evict data volume [GBytes],"
               "Memory load bandwidth [MBytes/s],"
               "Memory load data volume [GBytes]\n";
}


/*
// THIS WAS FORMERLY PART OF THE "main" FUNCTION WHICH RAN THE BENCHMARKS
  if (vm.count("help"))
  {
    cout << desc << "\n"
         << "Currently, only one type of benchmark can be specified per run \n"
         << "of this CLI. This means, for instance, that if -2, -b, and -s\n"
         << "supplied, only one will run. The benchmark that runs is \n"
         << "undefined.\n";
    return 0;
  }

  // okay, so we're actually doing things. We will set common likwid envvars
  // here: 

  // should be done by fhv_perfmon::init
  // setenv("LIKWID_MODE", "1", 1);
  // setenv("LIKWID_FILEPATH", filepath, 1); 
  // setenv("LIKWID_THREADS", "0,1,2,3", 1);
  // setenv("LIKWID_FORCE", "1", 1);


  // benchmark things
  if (vm.count("csv-style-output"))
  {
    o = output_format::csv;
  }
  if (vm.count("print-csv-header"))
  {
    print_csv_header();
  }
  if (vm.count("print-perfmon-csv-header"))
  {
    std::cout << "Sorry, csv printing has temporarily been disabled."
      << std::endl;
    // fhv_perfmon::printCsvHeader();
  }

  bool benchmark_done = false;
  std::string benchmark_parameter_info = "";

  // This block is where the benchark is ran
  if (vm.count("benchmark-all"))
  {
    benchmark_all();

    benchmark_parameter_info += fmt::format(
        "All Benchmarks. Params: DP FLOPS: {0:.2e}, SP FLOPS: {0:.2e}, "
        "L2 iterations: {1:.2e}, L2 size (KiB): {2:.2e}, ",
        "L3 iterations: {3:.2e}, L3 size (KiB): {4:.2e}, ",
        "RAM iterations: {5:.2e}, RAM size (KiB): {6:.2e}",
        FLOAT_NUM_ITERATIONS, static_cast<double>(l2_args[0]), 
        static_cast<double>(l2_args[2]), static_cast<double>(l3_args[0]),
        static_cast<double>(l3_args[2]), static_cast<double>(ram_args[0]),
        static_cast<double>(ram_args[2]));
    benchmark_done = true;
  }
  else if (vm.count("benchmark-cache-and-memory"))
  {
    // num_memory_iter = cache_and_memory_args[0];
    // memory_size_kb = cache_and_memory_args[1];
    fhv_perfmon::init("copy_bw", "", "L2|L3|MEM|DATA");
    int num_groups = 4;
    bandwidth_rw("copy_bw", num_groups, cache_and_memory_args[0], 
      cache_and_memory_args[1], cache_and_memory_args[2]);

    benchmark_parameter_info += fmt::format(
        "Cache/Mem Benchmarks. Params: "
        "L2 iterations: {:.2e}, L2 measured iterations: {:d}, L2 size (KiB): {:.2e}, ",
        "L3 iterations: {:.2e}, L3 measured iterations: {:d}, L3 size (KiB): {:.2e}, ",
        "RAM iterations: {:.2e}, RAM measured iterations: {:d}, RAM size (KiB): {:.2e}",
        static_cast<double>(l2_args[0]),
        static_cast<double>(l2_args[1]),
        static_cast<double>(l2_args[2]),
        static_cast<double>(l3_args[0]),
        static_cast<double>(l3_args[1]),
        static_cast<double>(l3_args[2]),
        static_cast<double>(ram_args[0]),
        static_cast<double>(ram_args[1]),
        static_cast<double>(ram_args[2]));
    benchmark_done = true;
  }
  else if (vm.count("L2"))
  {
    const char * region_name = "region_l2";
    fhv_perfmon::init(region_name);
    int num_groups = 7;
    bandwidth_rw(region_name, num_groups, l2_args[0], l2_args[1], l2_args[2]);

    benchmark_parameter_info += fmt::format(
        "L2 Benchmark. Params: L2 iterations: {:.2e}, "
        "L2 measured iterations: {:d}, L2 size (KiB): {:.2e}, ",
        static_cast<double>(l2_args[0]), l2_args[1],
        static_cast<double>(l2_args[2]));
    benchmark_done = true;
  }
  else if (vm.count("L3"))
  {
    const char * region_name = "region_l3";
    fhv_perfmon::init(region_name);
    int num_groups = 7;
    bandwidth_rw(region_name, num_groups, l3_args[0], l3_args[1], l3_args[2]);

    benchmark_parameter_info += fmt::format(
        "L3 Benchmark. Params: L3 iterations: {:.2e}, "
        "L3 measured iterations: {:d}, L3 size (KiB): {:.2e}, ",
        static_cast<double>(l3_args[0]),
        l3_args[1],
        static_cast<double>(l3_args[2]));
    benchmark_done = true;
  }
  else if (vm.count("mem"))
  {
    const char * region_name = "region_mem";
    fhv_perfmon::init(region_name);
    int num_groups = 7;
    bandwidth_rw(region_name, num_groups, ram_args[0], ram_args[1], ram_args[2]);

    benchmark_parameter_info += fmt::format(
        "RAM Benchmark. Params: RAM iterations: {:.2e}, "
        "RAM measured iterations: {:d}, RAM size (KiB): {:.2e}, ",
        static_cast<double>(ram_args[0]),
        ram_args[1],
        static_cast<double>(ram_args[2]));
    benchmark_done = true;
  }
  else if (vm.count("flops_sp"))
  {
    const char * region_name = "region_flops_sp";
    fhv_perfmon::init(region_name);
    benchmark_flops(precision::SINGLE_P, sp_flop_num_iterations);

    benchmark_parameter_info += fmt::format(
        "SP FLOPS Benchmark. Num iterations: {:.2e}, ", 
        static_cast<double>(sp_flop_num_iterations));
    benchmark_done = true;
  }
  else if (vm.count("flops_dp"))
  {
    const char * region_name = "region_flops_dp";
    fhv_perfmon::init(region_name);
    benchmark_flops(precision::DOUBLE_P, dp_flop_num_iterations);

    benchmark_parameter_info += fmt::format(
        "DP FLOPS Benchmark. Num iterations: {:.2e}, ", 
        static_cast<double>(dp_flop_num_iterations));
    benchmark_done = true;
  }

  if(benchmark_done){
    fhv_perfmon::close();
    fhv_perfmon::resultsToJson(benchmark_parameter_info);

    if(o == output_format::pretty){
      fhv_perfmon::printHighlights();
    }
    else if (o == output_format::csv){
      std::cout << "Sorry, csv printing has temporarily been disabled." 
        << std::endl;
      // auto m = fhv_perfmon::get_aggregate_results()
      //   .at(fhv_perfmon::aggregation_t::sum)
      //   .at(fhv_perfmon::result_t::metric);
      // auto e = fhv_perfmon::get_aggregate_results()
      //   .at(fhv_perfmon::aggregation_t::sum)
      //   .at(fhv_perfmon::result_t::event);

      // std::cout << memory_size_kb << ","
      //           << num_memory_iter << ","
      //           << m["copy_bw"]["DATA"][load_to_store_ratio_metric_name] << ","
      //           << e["copy_bw"]["DATA"]["MEM_INST_RETIRED_ALL_LOADS"] 
      //              * CACHE_LINE_SIZE_BYTES * BYTES_TO_GBYTES << ","
      //           << e["copy_bw"]["DATA"]["MEM_INST_RETIRED_ALL_STORES"] 
      //              * CACHE_LINE_SIZE_BYTES * BYTES_TO_GBYTES << ","
      //           << (e["copy_bw"]["DATA"]["MEM_INST_RETIRED_ALL_LOADS"] 
      //              + e["copy_bw"]["DATA"]["MEM_INST_RETIRED_ALL_STORES"])
      //              * CACHE_LINE_SIZE_BYTES * BYTES_TO_GBYTES << ","
      //           << m["copy_bw"]["L2"][l2_bandwidth_metric_name] << ","
      //           << m["copy_bw"]["L2"][l2_data_volume_name] << ","
      //           << m["copy_bw"]["L2"][l2_evict_bandwidth_name] << ","
      //           << m["copy_bw"]["L2"][l2_evict_data_volume_name] << ","
      //           << m["copy_bw"]["L2"][l2_load_bandwidth_name] << ","
      //           << m["copy_bw"]["L2"][l2_load_data_volume_name] << ","
      //           << m["copy_bw"]["L3"][l3_bandwidth_metric_name] << ","
      //           << m["copy_bw"]["L3"][l3_data_volume_name] << ","
      //           << m["copy_bw"]["L3"][l3_evict_bandwidth_name] << ","
      //           << m["copy_bw"]["L3"][l3_evict_data_volume_name] << ","
      //           << m["copy_bw"]["L3"][l3_load_bandwidth_name] << ","
      //           << m["copy_bw"]["L3"][l3_load_data_volume_name] << ","
      //           << m["copy_bw"]["MEM"][ram_bandwidth_metric_name] << ","
      //           << m["copy_bw"]["MEM"][ram_data_volume_metric_name] << ","
      //           << m["copy_bw"]["MEM"][ram_evict_bandwidth_name] << ","
      //           << m["copy_bw"]["MEM"][ram_evict_data_volume_name] << ","
      //           << m["copy_bw"]["MEM"][ram_load_bandwidth_name] << ","
      //           << m["copy_bw"]["MEM"][ram_load_data_volume_name] << "\n"
      //           ;
    }
  }
  */
