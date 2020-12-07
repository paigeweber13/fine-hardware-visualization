// things CLI will need to do:
//  - benchmark machine
//  - create visualization from output data

#include <boost/program_options.hpp>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <immintrin.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <omp.h>

// #include "architecture.hpp"
#include "computation_measurements.hpp"
//#include "fhv_perfmon.hpp"
#include "types.hpp"
#include "performance_monitor_defines.hpp"
#include "saturation_diagram.hpp"
#include "likwid.h"

using namespace std;
using json = nlohmann::json;
namespace po = boost::program_options;

#define BYTES_TO_GBYTES 1e-9

// ---- default values
const std::uint64_t FLOAT_NUM_ITERATIONS_SHORT = 1000000000;
const std::uint64_t FLOAT_NUM_ITERATIONS =       100000000000;
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
      likwid_markerStartRegion("flops_sp");
      flops_sp(num_iter);
      likwid_markerStopRegion("flops_sp");
    }
  } else if (p == precision::DOUBLE_P){
    #pragma omp parallel
    {
      likwid_markerStartRegion("flops_dp");
      flops_dp(num_iter);
      likwid_markerStopRegion("flops_dp");
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

/* ---- visualize ----
 * high level function that loads data and creates diagrams for each region
 */
void visualize(
  std::string perfmon_output_filename,
  std::string image_output_filename,
  std::string color_scale)
{
  // std::string image_output_filename = "perfmon_output.svg";

  // read a JSON file
  std::ifstream i(perfmon_output_filename);
  if(!i){
    std::cout << "ERROR: The json specified for visualization does not exist!"
      << std::endl;
    return;
  }
  json j;
  i >> j;

  std::string params = j[json_info_section][json_parameter_key];

  std::string region_name;
  for(auto &saturation_item: j[json_results_section].items())
  {
    region_name = saturation_item.key();
    std::cout << "Creating visualization for region " << region_name 
      << std::endl;

    std::size_t pos = 0;
    while(image_output_filename.find('.', pos+1) != std::string::npos){
      pos = image_output_filename.find('.', pos+1);
    }
    std::string ext = image_output_filename.substr(pos);
    std::string this_image_output_filename = 
      image_output_filename.substr(0, pos) + "_" + 
      region_name + ext;

    saturation_diagram::draw_diagram_overview(j, color_scale, region_name,
                                              this_image_output_filename);
    std::cout << "Visualization saved to " << this_image_output_filename 
      << std::endl;
  }
}

int main(int argc, char *argv[])
{
  // places where argument values will be stored
  std::uint64_t sp_flop_num_iterations;
  std::uint64_t dp_flop_num_iterations;

  std::vector<std::uint64_t> cache_and_memory_args;
  // std::tuple<double, double, double, double, double, double> input_colors_continuous_scale = {
  std::vector<double> input_colors_continuous_scale = {
    200, 200, 200,
    43, 140, 190
  };
  std::string color_scale = "RdPu";

  // for use with csv output: keeps a consistent name no matter what vector is
  // used for arguments

  // std::uint64_t num_memory_iter = 0;
  // std::uint64_t memory_size_kb = 0;

  std::string perfmon_output_filename;

  char time_str[20];
  std::time_t t = std::time(nullptr);
  std::strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H%M", 
    std::localtime(&t));
  std::string image_output_filename = "perfmon_visualization_";
  image_output_filename += time_str;
  image_output_filename += ".svg";

  output_format o = output_format::pretty;

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
              "of total iterations, number of measured iterations, and size "
              "of data transfer in kilobytes. "
              "Default: " + to_string(l2_args[0]) + " " + 
              to_string(l2_args[1])).c_str()
              )
    ("L3,3", po::value<std::vector<std::uint64_t>>(&l3_args)->
             multitoken()->zero_tokens(),
             ("benchmark L3 cache bandwidth. May be followed by number "
              "of total iterations, number of measured iterations, and size "
              "of data transfer in kilobytes. "
              "Default: " + to_string(l3_args[0]) + " " + 
              to_string(l3_args[1])).c_str()
              )
    ("mem,m", po::value<std::vector<std::uint64_t>>(&ram_args)->
              multitoken()->zero_tokens(),
              ("benchmark memory (ram) bandwidth. May be followed by number "
              "of total iterations, number of measured iterations, and size "
              "of data transfer in kilobytes. "
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
      "internally. This corresponds with fhv_perfmon::printCsvOutput "
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
    ("color-scale,c",
      po::value<std::string>(&color_scale),
      "Specify the color scale to be used in the visualization. Must be one of "
      "'Greys', 'PuBu', 'RdPu', 'YlGn', or 'YlGnBu'.  If no color scale is "
      "specified, 'RdPu' will be used.")
    ("test-discrete-color-scale",
      "Create diagram demonstrating each discrete color scale option.")
    ("test-color-lerp",
      "create band of color from least to most to test linear interpolation. "
      "respects colors specified in --interpolated-colors.")
    ("interpolated-colors",
       po::value<std::vector<double>>(&input_colors_continuous_scale)->multitoken(),
       "Six numbers must be "
       "supplied to indicate RGB values for the min color and max color, in "
       "that order. Values should be in the range [0:255].")
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
    std::cout << "Sorry, csv printing has temporarily been disabled."
      << std::endl;
    // fhv_perfmon::printCsvHeader();
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
    // num_memory_iter = cache_and_memory_args[0];
    // memory_size_kb = cache_and_memory_args[1];
    fhv_perfmon::init("copy_bw", "", "L2|L3|MEM|DATA");
    int num_groups = 4;
    bandwidth_rw("copy_bw", num_groups, cache_and_memory_args[0], 
      cache_and_memory_args[1], cache_and_memory_args[2]);
    benchmark_done = true;
  }
  else if (vm.count("L2"))
  {
    const char * region_name = "region_l2";
    fhv_perfmon::init(region_name, "", "L2");
    int num_groups = 1;
    bandwidth_rw(region_name, num_groups, l2_args[0], l2_args[1], l2_args[2]);
    benchmark_done = true;
  }
  else if (vm.count("L3"))
  {
    const char * region_name = "region_l3";
    fhv_perfmon::init(region_name, "", "L3");
    int num_groups = 1;
    bandwidth_rw(region_name, num_groups, l3_args[0], l3_args[1], l3_args[2]);
    benchmark_done = true;
  }
  else if (vm.count("mem"))
  {
    const char * region_name = "region_mem";
    fhv_perfmon::init(region_name, "", "MEM");
    int num_groups = 1;
    bandwidth_rw(region_name, num_groups, ram_args[0], ram_args[1], ram_args[2]);
    benchmark_done = true;
  }
  else if (vm.count("flops_sp"))
  {
    const char * region_name = "region_flops_sp";
    fhv_perfmon::init(region_name, "", "FLOPS_SP");
    benchmark_flops(precision::SINGLE_P, sp_flop_num_iterations);
    benchmark_done = true;
  }
  else if (vm.count("flops_dp"))
  {
    const char * region_name = "region_flops_dp";
    fhv_perfmon::init(region_name, "", "FLOPS_DP");
    benchmark_flops(precision::DOUBLE_P, dp_flop_num_iterations);
    benchmark_done = true;
  }

  if(benchmark_done){
    fhv_perfmon::close();

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

  // visualization things
  if (vm.count("test-color-lerp"))
  {
    auto min_color = rgb_color(
        input_colors_continuous_scale[0] / 255.0,
        input_colors_continuous_scale[1] / 255.0,
        input_colors_continuous_scale[2] / 255.0);
    auto max_color = rgb_color(
        input_colors_continuous_scale[3] / 255.0,
        input_colors_continuous_scale[4] / 255.0,
        input_colors_continuous_scale[5] / 255.0);
    saturation_diagram::test_color_lerp(min_color, max_color, 1000, 100, 20);
  }
  if (vm.count("test-discrete-color-scale"))
  {
    saturation_diagram::test_discrete_color_scale(1000, 100);
  }
  if (vm.count("visualize"))
  {
    visualize(perfmon_output_filename,
      image_output_filename, 
      color_scale);
  }

  return 0;
}
