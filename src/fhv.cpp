// things CLI will need to do:
//  - benchmark machine
//  - create visualization from output data

#include <boost/program_options.hpp>
#include <cstdlib>
#include <ctime>
#include <immintrin.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <omp.h>

#include "../lib/architecture.h"
#include "../lib/computation_measurements.h"
#include "../lib/performance_monitor.h"
#include "../lib/saturation_diagram.h"
#include "likwid.h"

using namespace std;
namespace po = boost::program_options;

#define BYTES_TO_GBYTES 1e-9

// ---- default values
const std::uint64_t FLOAT_NUM_ITERATIONS_SHORT = 1000000000;
const std::uint64_t FLOAT_NUM_ITERATIONS =       10000000000;

// each argument vector has num_iterations, mem_size_kb

// 100000 iterations for a good average
// 100 kilobytes to fit well inside L2 cache
std::vector<std::uint64_t> l2_args = {100000, 100};

// 10000 iterations for a good average
// 1000 kilobytes to fit well inside L3 cache
std::vector<std::uint64_t> l3_args = {10000, 1000};

// 20 iterations for a good average
// 100000 kilobytes (100MB) so it can't all be cached
std::vector<std::uint64_t> ram_args = {20, 100000};

// enums
enum precision { SINGLE_P, DOUBLE_P };
enum output_format { pretty, csv };

// likwid results file path
const char *filepath = performance_monitor::likwidOutputFilepath.c_str();

void benchmark_flops(precision p, uint64_t num_iter)
{
#pragma omp parallel
  {
    if(p == precision::SINGLE_P){
      #pragma omp barrier
      likwid_markerStartRegion("flops_sp");
      flops_sp(num_iter);
      likwid_markerStopRegion("flops_sp");
      #pragma omp barrier
    } else if (p == precision::DOUBLE_P){
      #pragma omp barrier
      likwid_markerStartRegion("flops_dp");
      flops_dp(num_iter);
      likwid_markerStopRegion("flops_dp");
      #pragma omp barrier
    }
    #pragma omp barrier
    likwid_markerNextGroup();
  }
}

void benchmark_memory_bw(std::string memory_type, uint64_t num_iterations,
                         uint64_t mem_size_kb){
  // memory type becomes the performance group. Typically this is "L2", "L3",
  // or "RAM"
  bandwidth_rw(memory_type.c_str(), 1, num_iterations, 1, mem_size_kb);
}

void benchmark_cache_and_memory(std::uint64_t num_iterations,
                          std::uint64_t data_size_kb)
{
  setenv("LIKWID_EVENTS",
         "L2|L3|MEM|DATA",
         1);

  likwid_markerInit();

#pragma omp parallel
  {
    likwid_markerThreadInit();
    likwid_markerRegisterRegion("copy_bw");
    likwid_pinThread(omp_get_thread_num());
  }
  
  bandwidth_rw("copy_bw", 4, num_iterations, 1, data_size_kb);
}

void benchmark_all()
{
  setenv("LIKWID_EVENTS",
         "FLOPS_SP|FLOPS_DP|L2|L3|MEM",
         1);

  likwid_markerInit();

#pragma omp parallel
  {
    likwid_markerThreadInit();
    likwid_markerRegisterRegion("flops_sp");
    likwid_markerRegisterRegion("flops_dp");
    likwid_markerRegisterRegion("L2");
    likwid_markerRegisterRegion("L3");
    likwid_markerRegisterRegion("MEM");
    likwid_pinThread(omp_get_thread_num());
  }

  std::cout << "starting single precision flop benchmark" << std::endl;
  benchmark_flops(precision::SINGLE_P, FLOAT_NUM_ITERATIONS);

  std::cout << "starting double precision flop benchmark" << std::endl;
  benchmark_flops(precision::DOUBLE_P, FLOAT_NUM_ITERATIONS);

  std::cout << "starting L2 cache rw bandwidth benchmark" << std::endl;
  benchmark_memory_bw("L2", l2_args[0], l2_args[1]);

  std::cout << "starting L3 cache rw bandwidth benchmark" << std::endl;
  benchmark_memory_bw("L3", l3_args[0], l3_args[1]);

  std::cout << "starting RAM rw bandwidth benchmark" << std::endl;
  benchmark_memory_bw("MEM", ram_args[0], ram_args[1]);
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

int main(int argc, char *argv[])
{
  // places where argument values will be stored
  std::uint64_t sp_flop_num_iterations;
  std::uint64_t dp_flop_num_iterations;

  std::vector<std::uint64_t> cache_and_memory_args;
  // std::tuple<double, double, double, double, double, double> input_colors = {
  std::vector<double> input_colors = {
    200, 200, 200,
    43, 140, 190
  };

  // for use with csv output: keeps a consistent name no matter what vector is
  // used for arguments
  std::uint64_t num_memory_iter = 0;
  std::uint64_t memory_size_kb = 0;

  std::string perfmon_output_filename;

  char time_str[20];
  std::time_t t = std::time(nullptr);
  std::strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H%M", 
    std::localtime(&t));
  std::string image_output_filename = "perfmon_visualization_";
  image_output_filename += time_str;
  image_output_filename += ".svg";

  output_format o = pretty;

  // behavior if no arguments are supplied
  if (argc < 2)
  {
    std::cout << "no options provided, running all benchmarks.\n";
    std::cout << "alternatively, pass '-h' for more options\n";
    // sleep a couple seconds?
    benchmark_all();
    return 0;
  }

  // behavior with arguments
  po::options_description desc(
    "Benchmarking machine with likwid");
  desc.add_options()
    ("help,h", "produce this help message")
    ("flops_sp,s", po::value<std::uint64_t>(&sp_flop_num_iterations)->
                   implicit_value(FLOAT_NUM_ITERATIONS), 
                   "benchmark single-precision flops")
    ("flops_dp,d", po::value<std::uint64_t>(&dp_flop_num_iterations)->
                   implicit_value(FLOAT_NUM_ITERATIONS), 
                   "benchmark double-precision flops")
    ("L2,2", po::value<std::vector<std::uint64_t>>(&l2_args)->
             multitoken()->zero_tokens(),
             ("benchmark L2 cache bandwidth. May be followed by number "
              "of iterations and size of data transfer in kilobytes. "
              "Default: " + to_string(l2_args[0]) + " " + 
              to_string(l2_args[1])).c_str()
              )
    ("L3,3", po::value<std::vector<std::uint64_t>>(&l3_args)->
             multitoken()->zero_tokens(),
             ("benchmark L3 cache bandwidth. May be followed by number "
              "of iterations and size of data transfer in kilobytes. "
              "Default: " + to_string(l3_args[0]) + " " + 
              to_string(l3_args[1])).c_str()
              )
    ("mem,m", po::value<std::vector<std::uint64_t>>(&ram_args)->
              multitoken()->zero_tokens(),
              ("benchmark memory (ram) bandwidth. May be followed by number "
               "of iterations and size of data transfer in kilobytes. "
               "Default: " + to_string(ram_args[0]) + " " + 
               to_string(ram_args[1])).c_str()
               )
    ("benchmark-all,b", "run all benchmarks")
    ("benchmark-cache-and-memory",
      po::value<std::vector<std::uint64_t>>(&cache_and_memory_args)->
        multitoken(),
      "benchmark cache and memory. Will run benchmark once for each cache and "
        "once for ram.")
    ("csv-style-output", "output benchmark results in a csv-style format:\n")
    ("print-csv-header", "print header of csv used by fhv (this binary). "
      "The CSV output for which this is the header provides detailed memory "
      "data.")
    ("print-perfmon-csv-header", "print header of csv that perfmon uses "
      "internally. This corresponds with performance_monitor::printCsvOutput "
      "and provides details about saturation of CPU/memory and port usage.")
    ("visualize,v", po::value<std::string>(&perfmon_output_filename), "create "
                    "a visualization from data output to json during program " 
                    "instrumentation. Argument should be relative path to "
                    "aforementioned json.")
    ("visualization-output,o", 
      po::value<std::string>(&image_output_filename), 
      "Path where visualization should be output to. Region name will "
      "automatically be appended. If not supplied, a default name with "
      "current date and time will be generated. Has no effect if --visualize"
      "is not supplied.")
    ("colors,c", 
      po::value<std::vector<double>>(&input_colors)->multitoken(),
      "Specify the colors used in the visualization. Six numbers must be "
      "supplied to indicate RGB values for the min color and max color, in "
      "that order. Values should be in the range [0:255].")
    ("test-color-lerp", 
      "create band of color from least to most to test linear interpolation. "
      "respects colors specified in --colors.")
    ;

  po::variables_map vm;
  try
  {
    po::store(
        po::command_line_parser(argc, argv).options(desc).run(),
        vm);
    po::notify(vm);
  }
  catch (po::error &e)
  {
    cerr << "ERROR: " << e.what() << endl
         << endl
         << desc << endl;
    return 1;
  }

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
  setenv("LIKWID_MODE", "1", 1);
  setenv("LIKWID_FILEPATH", filepath, 1); 
  setenv("LIKWID_THREADS", "0,1,2,3", 1);
  setenv("LIKWID_FORCE", "1", 1);

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
    performance_monitor::printCsvHeader();
  }

  bool benchmark_done = false;

  // This block is where the benchark is ran
  if (vm.count("benchmark-all"))
  {
    benchmark_all();
    benchmark_done = true;
  }
  else if (vm.count("benchmark-cache-and-memory"))
  {
    num_memory_iter = cache_and_memory_args[0];
    memory_size_kb = cache_and_memory_args[1];
    benchmark_cache_and_memory(cache_and_memory_args[0],
                               cache_and_memory_args[1]);
    benchmark_done = true;
  }
  else if (vm.count("L2"))
  {
    num_memory_iter = l2_args[0];
    memory_size_kb = l2_args[1];
    setenv("LIKWID_EVENTS",
           "L2",
           1);

    likwid_markerInit();

  #pragma omp parallel
    {
      likwid_markerThreadInit();
      likwid_markerRegisterRegion("L2");
      likwid_pinThread(omp_get_thread_num());
    }
    benchmark_memory_bw("L2", l2_args[0], l2_args[1]);
    benchmark_done = true;
  }
  else if (vm.count("L3"))
  {
    num_memory_iter = l3_args[0];
    memory_size_kb = l3_args[1];
    setenv("LIKWID_EVENTS",
           "L3",
           1);

    likwid_markerInit();

  #pragma omp parallel
    {
      likwid_markerThreadInit();
      likwid_markerRegisterRegion("L3");
      likwid_pinThread(omp_get_thread_num());
    }
    benchmark_memory_bw("L3", l3_args[0], l3_args[1]);
    benchmark_done = true;
  }
  else if (vm.count("mem"))
  {
    num_memory_iter = ram_args[0];
    memory_size_kb = ram_args[1];
    setenv("LIKWID_EVENTS",
           "MEM",
           1);

    likwid_markerInit();

  #pragma omp parallel
    {
      likwid_markerThreadInit();
      likwid_markerRegisterRegion("MEM");
      likwid_pinThread(omp_get_thread_num());
    }
    benchmark_memory_bw("MEM", ram_args[0], ram_args[1]);
    benchmark_done = true;
  }
  else if (vm.count("flops_sp"))
  {
    setenv("LIKWID_EVENTS",
           "FLOPS_SP",
           1);

    likwid_markerInit();

  #pragma omp parallel
    {
      likwid_markerThreadInit();
      likwid_markerRegisterRegion("flops_sp");
      likwid_pinThread(omp_get_thread_num());
    }
    benchmark_flops(precision::SINGLE_P, sp_flop_num_iterations);
    benchmark_done = true;
  }
  else if (vm.count("flops_dp"))
  {
    setenv("LIKWID_EVENTS",
           "FLOPS_DP",
           1);

    likwid_markerInit();

  #pragma omp parallel
    {
      likwid_markerThreadInit();
      likwid_markerRegisterRegion("flops_sp");
      likwid_pinThread(omp_get_thread_num());
    }
    benchmark_flops(precision::DOUBLE_P, dp_flop_num_iterations);
    benchmark_done = true;
  }

  if(benchmark_done){
    likwid_markerClose();

    if(o == pretty){
      performance_monitor::buildResultsMaps();
      performance_monitor::compareActualWithBench();

      performance_monitor::printHighlights();
    }
    else if (o == csv){
      performance_monitor::buildResultsMaps();

      auto m = performance_monitor::get_aggregate_results()
        .at(aggregation_type::sum)
        .at(result_type::metric);
      auto e = performance_monitor::get_aggregate_results()
        .at(aggregation_type::sum)
        .at(result_type::event);

      std::cout << memory_size_kb << ","
                << num_memory_iter << ","
                << m["copy_bw"]["DATA"][load_to_store_ratio_metric_name] << ","
                << e["copy_bw"]["DATA"]["MEM_INST_RETIRED_ALL_LOADS"] 
                   * CACHE_LINE_SIZE_BYTES * BYTES_TO_GBYTES << ","
                << e["copy_bw"]["DATA"]["MEM_INST_RETIRED_ALL_STORES"] 
                   * CACHE_LINE_SIZE_BYTES * BYTES_TO_GBYTES << ","
                << (e["copy_bw"]["DATA"]["MEM_INST_RETIRED_ALL_LOADS"] 
                   + e["copy_bw"]["DATA"]["MEM_INST_RETIRED_ALL_STORES"])
                   * CACHE_LINE_SIZE_BYTES * BYTES_TO_GBYTES << ","
                << m["copy_bw"]["L2"][l2_bandwidth_metric_name] << ","
                << m["copy_bw"]["L2"][l2_data_volume_name] << ","
                << m["copy_bw"]["L2"][l2_evict_bandwidth_name] << ","
                << m["copy_bw"]["L2"][l2_evict_data_volume_name] << ","
                << m["copy_bw"]["L2"][l2_load_bandwidth_name] << ","
                << m["copy_bw"]["L2"][l2_load_data_volume_name] << ","
                << m["copy_bw"]["L3"][l3_bandwidth_metric_name] << ","
                << m["copy_bw"]["L3"][l3_data_volume_name] << ","
                << m["copy_bw"]["L3"][l3_evict_bandwidth_name] << ","
                << m["copy_bw"]["L3"][l3_evict_data_volume_name] << ","
                << m["copy_bw"]["L3"][l3_load_bandwidth_name] << ","
                << m["copy_bw"]["L3"][l3_load_data_volume_name] << ","
                << m["copy_bw"]["MEM"][ram_bandwidth_metric_name] << ","
                << m["copy_bw"]["MEM"][ram_data_volume_metric_name] << ","
                << m["copy_bw"]["MEM"][ram_evict_bandwidth_name] << ","
                << m["copy_bw"]["MEM"][ram_evict_data_volume_name] << ","
                << m["copy_bw"]["MEM"][ram_load_bandwidth_name] << ","
                << m["copy_bw"]["MEM"][ram_load_data_volume_name] << "\n"
                ;
    }
  }

  // visualization things
  if (vm.count("test-color-lerp"))
  {
    auto min_color = rgb_color(
      input_colors[0]/255.0,
      input_colors[1]/255.0,
      input_colors[2]/255.0);
    auto max_color = rgb_color(
      input_colors[3]/255.0,
      input_colors[4]/255.0,
      input_colors[5]/255.0);
    test_color_lerp(min_color, max_color, 1000, 100, 20);
  }
  if (vm.count("visualize"))
  {
    auto min_color = rgb_color(
      input_colors[0]/255.0,
      input_colors[1]/255.0,
      input_colors[2]/255.0);
    auto max_color = rgb_color(
      input_colors[3]/255.0,
      input_colors[4]/255.0,
      input_colors[5]/255.0);
    visualize(perfmon_output_filename, image_output_filename, min_color, 
      max_color);
  }

  return 0;
}
