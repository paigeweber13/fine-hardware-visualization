#include <boost/program_options.hpp>
#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <omp.h>

#include "../lib/computation_measurements.hpp"
#include "../lib/performance_monitor.hpp"

#define NUM_MEM_OPERATIONS_PER_ITER 2

const double microseconds_to_seconds = 1e-6;

namespace po = boost::program_options;

enum output_format { pretty, csv };
enum test_type { none, flops, mem, both };

struct bw_results {
  double duration_seconds;
  double gb_transferred;
  double bandwidth;
  double thing_computed;
};

struct flop_results {
  double duration_seconds;
  double num_fp_ops;
  double mflops;
  // double thing_computed;
};

bw_results bandwidth_rw_bench_compare(
  std::uint64_t num_iterations, std::uint64_t size_kib)
{
  // Is this code getting optimized out??? is that why I'm getting huge
  // bandwidth numbers?

  // I'm no longer getting huge bandwidth numbers but my manual calculations
  // differ from likwid by a factor of 2

  const unsigned num_inner_iterations = 1;
  const double kb_to_mb = 1e-3;
  const double kb_to_gb = 1e-6;
  const double gb_to_mb = 1e3;

  unsigned num_threads;
  double duration;
  std::chrono::high_resolution_clock::time_point start_time, end_time;

  const char * tag = "RAM";
  // align to cache line, which is 512 bits or 64 bytes
  std::size_t array_size_bytes = size_kib * KILO_BYTE_SIZE;
  std::uint64_t num_doubles = array_size_bytes/BYTES_PER_DOUBLE;
  double * array = static_cast<double *>(
    aligned_alloc(64, array_size_bytes));
  double * copy_array = static_cast<double *>(
    aligned_alloc(64, array_size_bytes));

#pragma omp parallel
  {
    std::uint64_t i, j, k;
    __m256d buffer;
    num_threads = omp_get_num_threads();

    // thr_num = omp_get_thread_num();
    for (i = 0; i < num_iterations; i++)
    {
      //Get time snapshot just for one iteration
      if (i == num_iterations / 2)
      {
        //	start = system_clock::now();
        //	Maybe start likwid region here
        //    printf("likwid start region %s on thread %d\n", bw->mark_tag, omp_get_thread_num());
        // performance_monitor::startRegion(tag);
        likwid_markerStartRegion(tag);
        start_time = std::chrono::high_resolution_clock::now();
      }

      for (k = 0; k < num_inner_iterations; k++){
        for (j = 0; j < num_doubles; j += DOUBLES_PER_VECTOR)
        {
          // Loading 256-bits into memory address of array
          buffer = _mm256_load_pd(array + j);
          // Storing 256-bits from buffer into address of cpy_arr
          _mm256_store_pd(copy_array + j, buffer);
        }
      }

      //Get time snapshot just for one iteration
      if (i == num_iterations / 2)
      {
        //	end = system_clock::now();
        //	Maybe stop likwid regin here
        //    printf("likwid stop region %s on thread %d\n", bw->mark_tag, omp_get_thread_num());
        // performance_monitor::stopRegion(tag);
        likwid_markerStopRegion(tag);
        end_time = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
      }
      asm(""); //Say no to loop optimization
    }
  }

  bw_results results;
  results.gb_transferred = num_inner_iterations * NUM_MEM_OPERATIONS_PER_ITER 
                         * size_kib * kb_to_gb * num_threads;
  results.duration_seconds = duration * microseconds_to_seconds;
  results.bandwidth = gb_to_mb * results.gb_transferred/results.duration_seconds;
  results.thing_computed = copy_array[static_cast<size_t>(copy_array[0]) % num_doubles];

  free(array);
  free(copy_array);
  return results;
}

flop_results flops_bench_compare(std::uint64_t num_iterations){
  //                                            100 000 000 one hundred million
  // const std::uint64_t FLOAT_NUM_ITERATIONS = 100000000;

  // we do 4 fma, so 8 total operations on 8 floats each
  const std::uint64_t FLOP_PER_ITERATION = 64; 

  std::uint64_t NUM_CORES;

  __m256 d;
  __m256i e;

  // FLOPS ----------------------------
  auto start_time = std::chrono::high_resolution_clock::now();
  #pragma omp parallel
  {
    NUM_CORES = omp_get_num_threads();
    // std::cout << "I am processor #" << omp_get_thread_num() << std::endl;

    // performance_monitor::startRegion("flops");
    likwid_markerStartRegion("flops");
    // #pragma omp barrier
    d = flops_sp(num_iterations);
    // #pragma omp barrier
    // performance_monitor::stopRegion("flops");
    likwid_markerStopRegion("flops");
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

  
  const double flops_to_mflops = 1e-6;

  flop_results results;
  results.duration_seconds = duration * microseconds_to_seconds;
  results.num_fp_ops = num_iterations * FLOP_PER_ITERATION * NUM_CORES;
  results.mflops = flops_to_mflops * results.num_fp_ops/results.duration_seconds;
  return results;
}

void custom_test(std::uint64_t num_flop_iter, unsigned num_mem_iter,
                 unsigned mem_size_kb, output_format o, test_type t){

  // performance_monitor::init("FLOPS_SP|MEM");
  setenv("LIKWID_EVENTS",
         "FLOPS_SP|MEM",
         1);
  // setenv("LIKWID_EVENTS", "MEM_DP|L2", 1);
  setenv("LIKWID_MODE", "1", 1);
  // output filepath
  setenv("LIKWID_FILEPATH", performance_monitor::likwidOutputFilepath.c_str(),
         1); 
  setenv("LIKWID_THREADS", "0,1,2,3", 1); // list of threads
  setenv("LIKWID_FORCE", "1", 1);

  likwid_markerInit();

#pragma omp parallel
  {
    likwid_markerThreadInit();
    likwid_markerRegisterRegion("flops");
    likwid_markerRegisterRegion("RAM");
    likwid_pinThread(omp_get_thread_num());
  }

  flop_results sp_flop_results;
  bw_results bandwidth_results;

  if(t == test_type::flops || t == test_type::both)
    sp_flop_results = flops_bench_compare(num_flop_iter);

  likwid_markerNextGroup();

  if(t == test_type::mem || t == test_type::both)
    bandwidth_results = bandwidth_rw_bench_compare(num_mem_iter, 
      mem_size_kb);

  // performance_monitor::close();
  likwid_markerClose();

  if (o == output_format::pretty){
    // performance_monitor::printResults();
    std::cout << "---- running benchmark and doing manual timing to compare";
    std::cout << " with likwid results ----" << std::endl;

    std::cout << "manually calculated results:\n";
    std::cout << "time taken for flops: " << sp_flop_results.duration_seconds << " seconds." << std::endl;
    std::cout << "total floating point operations: " << sp_flop_results.num_fp_ops << std::endl;
    std::cout << sp_flop_results.mflops << " MFlop/s" << std::endl;
    std::cout << "time taken for memory transfer: " << bandwidth_results.duration_seconds << " seconds." << std::endl;
    std::cout << "size of data transferred (GB): " << bandwidth_results.gb_transferred << std::endl;
    std::cout << "bandwidth (MB/s): " << bandwidth_results.bandwidth << std::endl;
  } else if (o == output_format::csv){

    auto perfmon_results = performance_monitor::get_aggregate_results();
    auto runtimes = performance_monitor::get_runtimes_by_tag();

    // manual_duration,manual_num_flops,manual_Mflops,
    // likwid_duration,likwid_num_flops,likwid_Mflops
    if(t == test_type::flops || t == test_type::both){
      std::cout << sp_flop_results.duration_seconds << "," 
                << sp_flop_results.num_fp_ops << "," 
                << sp_flop_results.mflops << ","
                << runtimes["flops"] << ","
                << perfmon_results[arithmetic_mean][event]["flops"]["FLOPS_SP"][total_sp_flops_event_name] << ","
                << perfmon_results[arithmetic_mean][event]["flops"]["FLOPS_SP"][mflops_metric_name]
                << "\n";
    }

    // manual_duration,manual_data_size_mb,manual_bandwidth_mb_per_s,
    // likwid_duration,likwid_data_size_mb,likwid_bandwidth_mb_per_s
    if(t == test_type::mem || t == test_type::both){
      std::cout << bandwidth_results.duration_seconds << "," 
                << bandwidth_results.gb_transferred << "," 
                << bandwidth_results.bandwidth << ","
                << runtimes["RAM"] << ","
                << perfmon_results[arithmetic_mean][metric]["RAM"]["MEM"][ram_data_volume_metric_name] << ","
                << perfmon_results[arithmetic_mean][metric]["RAM"]["MEM"][ram_bandwidth_metric_name]
                << "\n";
    }
  }
}

void simple_test(){
  custom_test(100000000, 10, 100000, output_format::pretty, test_type::both);
}

int main(int argc, char* argv[])
{
  po::options_description desc(
    "Comparing manual benchmarks with those from likwid");
  desc.add_options()
    ("help,h", "produce this help message")
    ("simple,s", "run a single simple test")
    ("csv-style-output,c", "Output results in a csv style instead of the "
      "detailed pretty output. Order of output follows.\n"
      "for flops:\n"
      "manual_duration,manual_num_flops,manual_Mflops,"
      "likwid_duration,likwid_num_flops,likwid_Mflops\n"
      "for mem:\n"
      "manual_duration,manual_data_size_mb,manual_bandwidth_mb_per_s,"
      "likwid_duration,likwid_data_size_mb,likwid_bandwidth_mb_per_s\n"
    )
    ("flops-test,f", po::value<std::uint64_t>(), 
      "Run a flop benchmark. Must be follwed by the number of iterations to "
      "compute flops")
    ("mem-bw-test,m", po::value<std::vector<std::uint64_t>>()->multitoken(), 
      "number of iterations to run the RAM benchmark. Must be followed by "
      "the number of iterations to run and the size of data transferred in "
      "Kb");

  po::variables_map vm;
  po::store(
    po::command_line_parser(argc, argv).options(desc).run(),
    vm);
  po::notify(vm);
  
  if (argc < 2) {
    std::cout << "no options provided, running single simple test.\n";
    std::cout << "run \"" << argv[0] << " --help\" to see more options\n";
    simple_test();
  } else if (vm.count("help")) {
    std::cout << desc << "\n";
  } else if (vm.count("simple")) {
    simple_test();
  } else {
    test_type t = none;
    output_format o = output_format::pretty;
    std::uint64_t flop_iter = 0;
    std::uint64_t mem_iter = 0;
    std::uint64_t mem_size = 0;

    if (vm.count("flops-test")) {
      t = test_type::flops;
      flop_iter = vm["flops-test"].as<std::uint64_t>();
    }
    if(vm.count("mem-bw-test")){
      if (t == test_type::flops)
        t = test_type::both;
      else
        t = test_type::mem;
      
      mem_iter = vm["mem-bw-test"].as<std::vector<std::uint64_t>>()[0];
      mem_size = vm["mem-bw-test"].as<std::vector<std::uint64_t>>()[1];
    }
    if(vm.count("csv-style-output")){
      o  = output_format::csv;
    }

    custom_test(flop_iter, mem_iter, mem_size, o, t);
  }
}
