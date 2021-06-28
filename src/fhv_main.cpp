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

#include "types.hpp"
#include "performance_monitor_defines.hpp"
#include "saturation_diagram.hpp"
#include "likwid.h"

using namespace std;
using json = nlohmann::json;
namespace po = boost::program_options;

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
    std::cerr << "ERROR: The json specified for visualization does not exist!"
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
  // std::tuple<double, double, double, double, double, double> input_colors_continuous_scale = {
  std::vector<double> input_colors_continuous_scale = {
    200, 200, 200,
    43, 140, 190
  };
  std::string color_scale = "RdPu";

  std::vector<std::string> perfmon_output_filenames;
  std::string image_output_filename;


  // behavior with arguments
  po::options_description desc(
    "Benchmarking machine with likwid");
  desc.add_options()
    ("help,h", "produce this help message")
    ("visualize,v", 
      po::value<std::vector<std::string>>(&perfmon_output_filenames)->
        multitoken(), 
        "create a visualization from data output to json during program "
        "instrumentation. Argument should be relative path to aforementioned "
        "json. More than one file may be supplied, in which case "
        "visualizations will be created for each. If more than one file is "
        "specified, the '--visualization-output' flag will be ignored.")
    ("visualization-output,o", 
      po::value<std::string>(&image_output_filename), 
      "Path where visualization should be output to. Region name will "
      "automatically be appended. If not supplied, the input filename will "
      "be used to generate a default filename.")
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
    std::cerr << "ERROR: " << e.what() << endl
         << endl
         << desc << endl;
    return 1;
  }

  if (vm.count("help") || argc == 1)
    std::cout << desc << std::endl;
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
    // give default visualization output name if none provided
    if (perfmon_output_filenames.size() == 0){
      std::cerr << "ERROR: no input files specified to visualize. Plese "
        << "include at least one json output by fhv_perfmon as an argument to "
        << "'--visualize'."
        << std::endl;
    }
    else if (image_output_filename != "" && perfmon_output_filenames.size() == 1) {
      visualize(perfmon_output_filenames[0], image_output_filename, 
        color_scale);
    }
    else {
      for (auto filename : perfmon_output_filenames) {
        image_output_filename = filename;
        image_output_filename.erase(image_output_filename.length() - 5);
        image_output_filename += ".svg";

        visualize(filename, image_output_filename, color_scale);
      }
    }
  }

  return 0;
}
