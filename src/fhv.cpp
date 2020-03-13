// things CLI will need to do:
//  - benchmark machine
//  - create visualization from output data

#include <boost/program_options.hpp>
#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <omp.h>

#include "../lib/computation_measurements.h"
#include "../lib/performance_monitor.h"

using namespace std;
namespace po = boost::program_options;

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

void benchmark_flops(precision p, uint64_t num_iter)
{
  if (p == precision::SINGLE_P){
    // performance_monitor::init("FLOPS_SP");
    std::cout << "starting single precision flop benchmark" << std::endl;
  } else if (p == precision::DOUBLE_P){
    // performance_monitor::init("FLOPS_DP");
    std::cout << "starting double precision flop benchmark" << std::endl;
  }
#pragma omp parallel
  {
    performance_monitor::startRegion("flops");
    if(p == precision::SINGLE_P){
      flops_sp(num_iter);
    } else if (p == precision::DOUBLE_P){
      flops_dp(num_iter);
    }
    performance_monitor::stopRegion("flops");
  }
  performance_monitor::close();
  // performance_monitor::printDetailedResults();
  performance_monitor::printOnlyAggregate();
  performance_monitor::printComparison();
}

void benchmark_memory_bw(std::string memory_type, uint64_t num_iterations,
                         uint64_t mem_size_kb){
  // memory type becomes the performance group. Typically this is "L2", "L3",
  // or "RAM"
  // performance_monitor::init(memory_type.c_str());
  std::cout << "starting " << memory_type << 
               " rw bandwidth benchmark" << std::endl;

  bandwidth_rw(memory_type.c_str(), num_iterations, mem_size_kb);

  performance_monitor::close();
  // performance_monitor::printDetailedResults();
  performance_monitor::printOnlyAggregate();
  performance_monitor::printComparison();
}

void benchmark_all(){
    performance_monitor::init("FLOPS_SP|FLOPS_DP|L2|L3|MEM");
    benchmark_flops(precision::SINGLE_P, FLOAT_NUM_ITERATIONS);
    likwid_markerNextGroup();
    benchmark_flops(precision::DOUBLE_P, FLOAT_NUM_ITERATIONS);
    likwid_markerNextGroup();
    benchmark_memory_bw("L2", l2_args[0], l2_args[1]);
    likwid_markerNextGroup();
    benchmark_memory_bw("L3", l3_args[0], l3_args[1]);
    likwid_markerNextGroup();
    benchmark_memory_bw("MEM", ram_args[0], ram_args[1]);
}

int main(int argc, char *argv[])
{
  // places where argument values will be stored
  std::uint64_t sp_flop_num_iterations;
  std::uint64_t dp_flop_num_iterations;

  std::string perfmon_output_filename;
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
    ("visualize,c", "output benchmark results in csv-style format:\n"
                    "")
    ("visualize,v", po::value<std::string>(&perfmon_output_filename), "create "
                    "a visualization from json data output in program "
                    "instrumentation")
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
         << "of this CLI. This means, for instance, that if -2, -3, and -s\n"
         << "supplied, only one will run. The benchmark that runs is \n"
         << "undefined.\n";
    return 0;
  }

  // benchmark things
  if(vm.count("csv-style-output")){
    o  = output_format::csv;
  }
  if(vm.count("benchmark-all")){
    benchmark_all();
  }
  else if (vm.count("L2"))
  {
    performance_monitor::init("L2");
    benchmark_memory_bw("L2", l2_args[0], l2_args[1]);
  }
  else if (vm.count("L3"))
  {
    performance_monitor::init("L3");
    benchmark_memory_bw("L3", l3_args[0], l3_args[1]);
  }
  else if (vm.count("mem"))
  {
    performance_monitor::init("MEM");
    benchmark_memory_bw("MEM", ram_args[0], ram_args[1]);
  }
  else if (vm.count("flops_sp"))
  {
    performance_monitor::init("FLOPS_SP");
    benchmark_flops(precision::SINGLE_P, sp_flop_num_iterations);
  }
  else if (vm.count("flops_dp"))
  {
    performance_monitor::init("FLOPS_DP");
    benchmark_flops(precision::DOUBLE_P, dp_flop_num_iterations);
  }

  // visualization things
  if (vm.count("visualize"))
  {
    // visualize(perfmon_output_filename, output_filename);
  }

  return 0;
}
