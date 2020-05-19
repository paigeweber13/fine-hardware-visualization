// things CLI will need to do:
//  - benchmark machine
//  - create visualization from output data

#include <boost/program_options.hpp>
#include <cairo.h>
#include <cairo-svg.h>
#include <ctime>
#include <immintrin.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <omp.h>
#include <tuple>

#include "../lib/architecture.h"
#include "../lib/computation_measurements.h"
#include "../lib/performance_monitor.h"

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
      likwid_markerStartRegion("flops_sp");
      flops_sp(num_iter);
      likwid_markerStopRegion("flops_sp");
    } else if (p == precision::DOUBLE_P){
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
  bandwidth_rw(memory_type.c_str(), num_iterations, mem_size_kb);
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
    likwid_markerRegisterRegion("L2");
    likwid_markerRegisterRegion("L3");
    likwid_markerRegisterRegion("MEM");
    likwid_markerRegisterRegion("DATA");
    likwid_pinThread(omp_get_thread_num());
  }
  
  benchmark_memory_bw("L2", num_iterations, data_size_kb);
  likwid_markerNextGroup();
  benchmark_memory_bw("L3", num_iterations, data_size_kb);
  likwid_markerNextGroup();
  benchmark_memory_bw("MEM", num_iterations, data_size_kb);
  likwid_markerNextGroup();
  benchmark_memory_bw("DATA", num_iterations, data_size_kb);
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
  likwid_markerNextGroup();

  std::cout << "starting double precision flop benchmark" << std::endl;
  benchmark_flops(precision::DOUBLE_P, FLOAT_NUM_ITERATIONS);
  likwid_markerNextGroup();

  std::cout << "starting L2 cache rw bandwidth benchmark" << std::endl;
  benchmark_memory_bw("L2", l2_args[0], l2_args[1]);
  likwid_markerNextGroup();

  std::cout << "starting L3 cache rw bandwidth benchmark" << std::endl;
  benchmark_memory_bw("L3", l3_args[0], l3_args[1]);
  likwid_markerNextGroup();

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

// ----- simple color type ----- //
typedef std::tuple<double, double, double> rgb_color;

// ----- LINEAR INTERPOLATION (LERP) ----- //

// taken from
// https://en.wikipedia.org/wiki/Linear_interpolation#Programming_language_support

// used under Creative Commons Attribution-ShareAlike 3.0 Unported License. See
// https://en.wikipedia.org/wiki/Wikipedia:Text_of_Creative_Commons_Attribution-ShareAlike_3.0_Unported_License
// for full text

// Imprecise method, which does not guarantee result = max_color when t = 1,
// due to floating-point arithmetic error. This form may be used when the
// hardware has a native fused multiply-add instruction.
rgb_color color_lerp( rgb_color min_color, rgb_color max_color, double t) 
{
  return rgb_color(
    std::get<0>(min_color) + t * (std::get<0>(max_color) - std::get<0>(min_color)),
    std::get<1>(min_color) + t * (std::get<1>(max_color) - std::get<1>(min_color)),
    std::get<2>(min_color) + t * (std::get<2>(max_color) - std::get<2>(min_color))
  );
}

// ----- CLAMP ----- //

// taken from: https://en.cppreference.com/w/cpp/algorithm/clamp

template<class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
    assert( !(hi < lo) );
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

// ---- custom log scale ---- //

// designed to make most apparent the difference between 0.01 and 0.2, to make
// somewhat apparent the difference between 0.2 and 0.5, and to minimize
// difference in values from 0.5 to 1.0

// expects 0.0 <= value <= 1.0
double scale(double value){
  #define c 7
  return (log2(value) + c)/c;
}

// ---- calculate saturation colors ----- // 

std::map<std::string, std::tuple<double, double, double>>
calculate_saturation_colors(
  json region_saturation,
  rgb_color min_color,
  rgb_color max_color
  )
{
  auto saturation_colors = std::map< std::string, 
    std::tuple<double, double, double> >();

  for (auto const& metric: region_saturation.items())
  {
    // clamp values to [0.0,1.0]
    metric.value() = clamp(static_cast<double>(metric.value()), 0.0, 1.0);
    // scale using custom function
    metric.value() = scale(metric.value());
    // clamp again because scale can give negative values for very small input
    metric.value() = clamp(static_cast<double>(metric.value()), 0.0, 1.0);

    saturation_colors[metric.key()] = 
      color_lerp(min_color, max_color, metric.value());
  }

  // TODO calculate rgb colors instead of returning empty tuples
  return saturation_colors;
}

// the idea here was to use hue to represent difference in best and worst
// saturation, and color saturation to represent average saturation. For more
// information, see "Thoughts on coloring of diagram" in
// `./DEVELOPMENT_LOG.md`. We have decided to replace this function with a much
// simpler strategy of coloring the diagram, implemented in the function
// `calculate_saturation_colors` above
[[deprecated("Incomplete: chose a simpler method of color calculation")]]
std::vector<std::tuple<double, double, double>>
calculate_saturation_colors_complicated(
  cairo_t *cr, 
  json region_saturation)
{
  double min_saturation = 1;
  double max_saturation = 0;
  double saturation_average = 1;

  auto saturation_colors = std::vector< std::tuple<double, double, double> >();

  for (auto const& metric: region_saturation.items())
  {
    // clamp values to [0.0,1.0]
    metric.value() = clamp(static_cast<double>(metric.value()), 0.0, 1.0);

    min_saturation = min(min_saturation, static_cast<double>(metric.value()));
    max_saturation = max(max_saturation, static_cast<double>(metric.value()));
    saturation_average *= static_cast<double>(metric.value());
  }

  // take root for geometric mean
  saturation_average = pow(saturation_average, 
    1.0/static_cast<double>(region_saturation.size()));

  double saturation_range = max_saturation - min_saturation;
  double color_value = 1.0 - 0.5 * saturation_average;
  double color_saturation = 1.0 - saturation_average;
  double color_hue = 125 - 125 * saturation_range;
  saturation_colors.push_back(
    std::tuple<double, double, double>(color_hue, color_saturation, color_value)
    );

  // TODO convert HSV to rgb

  // TODO return list of rgb colors instead of empty tuples
  return saturation_colors;
}

void test_color_lerp(
  std::tuple<double, double, double> min_color, 
  std::tuple<double, double, double> max_color,
  unsigned num_steps,
  unsigned step_size,
  unsigned height)
{
  std::string image_output_filename = "test_color_lerp.svg";

  // create surface and cairo object
  cairo_surface_t *surface = cairo_svg_surface_create(
    image_output_filename.c_str(),
    num_steps * step_size,  // image width
    height              // image height
  );
  cairo_t *cr = cairo_create(surface);

  double line_thickness = 10.0;
  cairo_set_line_width(cr, line_thickness);

  for (unsigned i = 0; i < num_steps; i++){
    // shape
    cairo_rectangle(cr, i * step_size, 0, step_size, height);

    // fill
    auto color = color_lerp(
      min_color, 
      max_color, 
      static_cast<double>(i + 1)/static_cast<double>(num_steps)
      );

    cairo_set_source_rgb(cr, 
      std::get<0>(color), 
      std::get<1>(color), 
      std::get<2>(color)
      );
    cairo_fill_preserve(cr);

    // stroke
    // cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_stroke(cr);
  }

  // --- done drawing things, clean up

  // svg file automatically gets written to disk
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}

void draw_diagram(
  std::map<std::string, rgb_color> region_colors,
  json region_data,
  std::string region_name,
  std::string output_filename
)
{
  rgb_color computation_color;
  std::string chosen_precision;
  
  if(region_data["DP [MFLOP/s]"] >
    region_data["SP [MFLOP/s]"])
  {
    computation_color = region_colors["DP [MFLOP/s]"];
    chosen_precision = "double-precision";
  }
  else 
  {
    computation_color = region_colors["SP [MFLOP/s]"];
    chosen_precision = "single-precision";
  }

  std::vector<std::string> computation_color_note = {
      "Note: both single- and double-precision floating point operations "
        "were measured. Here the cores are",
      "visualized using " + chosen_precision + " saturation, because "
        "it was higher."
    };
  std::vector<std::string> l1_cache_color_note = {
      "Note: L1 cache is currently not measured and therefore will appear "
        "white. This is not an indication of",
      "L1 cache saturation."
    };

  // variables for cairo
  double line_thickness = 10.0;
  double text_line_thickness = 1.0;
  double text_line_thickness_small = 0.0;
  double text_size_xlarge = 75.0;
  double text_size_large = 50.0;
  double text_size_medium = 30.0;
  double text_size_small = 15.0;
  cairo_text_extents_t text_extents;
  std::string text;

  // initialization for cairo
  double image_width = 800;
  double image_height = 1500;
  double margin_x = 50;
  double margin_y = 50;
  double title_text_height = 210;
  cairo_surface_t *surface = cairo_svg_surface_create(
      output_filename.c_str(),
      image_width,
      image_height
  );
  cairo_t *cr =
      cairo_create(surface);
  cairo_matrix_t default_matrix, rotated_matrix;
  cairo_matrix_init_identity(&default_matrix);
  cairo_matrix_init_identity(&rotated_matrix);
  cairo_set_font_size(cr, text_size_large);
  cairo_get_font_matrix(cr, &default_matrix);
  cairo_get_font_matrix(cr, &rotated_matrix);
  cairo_matrix_rotate(&rotated_matrix, -90.0 * M_PI / 180.0);

  // --- title and description text --- //
  double current_text_y = margin_y + text_extents.height;
  double line_spacing = 10;

  text = "Saturation diagram for region";
  cairo_text_extents(cr, text.c_str(), &text_extents);
  cairo_move_to(
    cr,
    image_width/2 - text_extents.width/2,
    current_text_y);
  cairo_text_path(cr, text.c_str());
  // cairo_show_text(cr, text);
  cairo_set_line_width(cr, text_line_thickness);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_fill_preserve(cr);
  cairo_stroke(cr); 

  text = "\"" + region_name + "\"";
  cairo_text_extents(cr, text.c_str(), &text_extents);
  current_text_y += line_spacing + text_extents.height;
  cairo_move_to(
    cr,
    image_width/2 - text_extents.width/2,
    current_text_y);
  cairo_text_path(cr, text.c_str());
  cairo_fill_preserve(cr);
  
  // start small text
  cairo_set_font_size(cr, text_size_small);
  cairo_set_line_width(cr, text_line_thickness_small);

  // computation color notes
  current_text_y += 2 * line_spacing;
  for(auto &line : computation_color_note){
    cairo_text_extents(cr, line.c_str(), &text_extents);
    current_text_y += line_spacing + text_extents.height;
    cairo_move_to(
      cr,
      margin_x,
      current_text_y);
    cairo_set_line_width(cr, text_line_thickness);
    cairo_text_path(cr, line.c_str());
    cairo_fill_preserve(cr);
    cairo_stroke(cr); 
  }
  
  // l1 cache note
  current_text_y += line_spacing;
  for(auto &line : l1_cache_color_note){
    cairo_text_extents(cr, line.c_str(), &text_extents);
    current_text_y += line_spacing + text_extents.height;
    cairo_move_to(
      cr,
      margin_x,
      current_text_y);
    cairo_set_line_width(cr, text_line_thickness);
    cairo_text_path(cr, line.c_str());
    cairo_fill_preserve(cr);
    cairo_stroke(cr); 
  }

  // revert size and width changes
  cairo_set_font_size(cr, text_size_large);
  cairo_set_line_width(cr, text_line_thickness);

  // --- draw RAM --- //
  double ram_x = margin_x;
  double ram_y = margin_y + title_text_height;
  double ram_width = 700;
  double ram_height = 200;
  cairo_rectangle(cr, ram_x, ram_y, ram_width, ram_height);
  // - fill
  cairo_set_source_rgb(
    cr,
    get<0>(region_colors["Memory bandwidth [MBytes/s]"]),
    get<1>(region_colors["Memory bandwidth [MBytes/s]"]),
    get<2>(region_colors["Memory bandwidth [MBytes/s]"]));
  cairo_fill_preserve(cr);

  // - stroke
  cairo_set_line_width(cr, line_thickness);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_stroke(cr);

  // - text
  text = "RAM";
  cairo_set_font_size(cr, text_size_xlarge);
  cairo_text_extents(cr, text.c_str(), &text_extents);
  cairo_move_to(
    cr,
    ram_x + (ram_width/2 - text_extents.width/2),
    ram_y + (ram_height/2 + text_extents.height/2));
  cairo_text_path(cr, text.c_str());
  // cairo_show_text(cr, text);
  cairo_set_line_width(cr, text_line_thickness);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_fill_preserve(cr);
  cairo_stroke(cr); 
  cairo_set_font_size(cr, text_size_large);


  // --- line from RAM to L3 cache --- //
  double line_start_x = ram_x + ram_width/2;
  double line_start_y = ram_y + ram_height;
  double line_end_x = line_start_x;
  double line_end_y = line_start_y + 150;
  cairo_move_to(cr, line_start_x, line_start_y);
  cairo_line_to(cr, line_end_x, line_end_y);


  // --- draw L3 cache --- //
  double l3_x = 200;
  double l3_y = line_end_y;
  double l3_width = 400;
  double l3_height = 100;
  cairo_rectangle(cr, l3_x, l3_y, l3_width, l3_height);
  // - fill
  cairo_set_source_rgb(
    cr,
    get<0>(region_colors["L3 bandwidth [MBytes/s]"]),
    get<1>(region_colors["L3 bandwidth [MBytes/s]"]),
    get<2>(region_colors["L3 bandwidth [MBytes/s]"]));
  cairo_fill_preserve(cr);

  // - stroke
  cairo_set_line_width(cr, line_thickness);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_stroke(cr);

  // - text
  text = "L3 Cache";
  cairo_text_extents(cr, text.c_str(), &text_extents);
  cairo_move_to(
    cr,
    l3_x + (l3_width/2 - text_extents.width/2),
    l3_y + (l3_height/2 + text_extents.height/2));
  cairo_text_path(cr, text.c_str());
  // cairo_show_text(cr, text);
  cairo_set_line_width(cr, text_line_thickness);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_fill_preserve(cr);
  cairo_stroke(cr); 


  // --- draw socket 0 --- //
  double socket0_x = 50;
  double socket0_y = l3_y + l3_height;
  double socket0_width = 700;
  double socket0_height = 700;
  cairo_rectangle(cr, socket0_x, socket0_y, socket0_width, socket0_height);
  // - fill
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_fill_preserve(cr);

  // - stroke
  cairo_set_line_width(cr, line_thickness);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_stroke(cr);

  // - text
  text = "Socket 0";
  cairo_text_extents(cr, text.c_str(), &text_extents);
  cairo_move_to(
    cr,
    socket0_x + (socket0_width/2 - text_extents.width/2),
    socket0_y + socket0_height + text_extents.height + 25);
  cairo_text_path(cr, text.c_str());
  // cairo_show_text(cr, text);
  cairo_set_line_width(cr, text_line_thickness);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_fill_preserve(cr);
  cairo_stroke(cr); 


  // --- draw cores --- //
  for (unsigned core_num = 0; core_num < CORES_PER_SOCKET; core_num++)
  {
    // cache numbers
    unsigned num_attached_caches = 2;
    unsigned cache_height = 50;

    // core numbers
    unsigned core_width = 550;
    unsigned core_height = 175;
    unsigned between_core_buffer = 50;
    unsigned core_x = 150;
    unsigned core_y = socket0_y + margin_y
      + core_num * (
        between_core_buffer + core_height +
        (num_attached_caches * cache_height)
      );

    // cache numbers that depend on core numbers
    unsigned cache_y = core_y + core_height;

    // - core text
    text = "Core " + to_string(core_num);
    cairo_text_extents(cr, text.c_str(), &text_extents);
    cairo_move_to(
      cr,
      core_x - text_extents.height,
      core_y + (core_height + cache_height * num_attached_caches)/2 
        + text_extents.width/2
    );
    
	  // cairo_rotate (cr, -90 * M_PI / 180.0);
    cairo_set_font_matrix(cr, &rotated_matrix);
    // cairo_set_font_size(cr, text_size_large);
    cairo_text_path(cr, text.c_str());
    // cairo_show_text(cr, text);
    cairo_set_line_width(cr, text_line_thickness);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_fill_preserve(cr);
    cairo_stroke(cr); 
    cairo_set_font_matrix(cr, &default_matrix);
	  // cairo_rotate (cr, 0 * M_PI / 180.0);

    // --- threads within core:

    double thread_x, thread_width;
    for (unsigned thread_num = 0; thread_num < THREADS_PER_CORE; thread_num++)
    {
      thread_x = core_x + thread_num * core_width / THREADS_PER_CORE;
      thread_width = core_width / THREADS_PER_CORE;

      cairo_rectangle(
          cr,
          thread_x,
          core_y,
          thread_width,
          core_height);
      // - fill
      cairo_set_source_rgb(
          cr,
          get<0>(computation_color),
          get<1>(computation_color),
          get<2>(computation_color));
      cairo_fill_preserve(cr);

      // - stroke
      cairo_set_line_width(cr, line_thickness);
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_stroke(cr);

      // - text
      text = "Thread " + to_string(core_num * THREADS_PER_CORE + thread_num);
      cairo_text_extents(cr, text.c_str(), &text_extents);
      cairo_move_to(
        cr,
        thread_x + thread_width/2 - text_extents.width/2,
        core_y + core_height/2 + text_extents.height/2);
      cairo_text_path(cr, text.c_str());
      // cairo_show_text(cr, text);
      cairo_set_line_width(cr, text_line_thickness);
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_fill_preserve(cr);
      cairo_stroke(cr); 
    }

    // --- caches attached to core:

    for (unsigned cache_num = 0; cache_num < num_attached_caches; cache_num++)
    {
      cairo_rectangle(cr,
                      core_x,
                      cache_y + cache_height * cache_num,
                      core_width,
                      cache_height);

      // - fill
      if (cache_num == 0)
      {
        cairo_set_source_rgb(cr, 1, 1, 1);
      }
      else
      {
        cairo_set_source_rgb(
          cr,
          get<0>(region_colors["L" + to_string(cache_num + 1) + 
            " bandwidth [MBytes/s]"]),
          get<1>(region_colors["L" + to_string(cache_num + 1) + 
            " bandwidth [MBytes/s]"]),
          get<2>(region_colors["L" + to_string(cache_num + 1) + 
            " bandwidth [MBytes/s]"]));
      }
      cairo_fill_preserve(cr);

      // - stroke
      cairo_set_line_width(cr, line_thickness);
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_stroke(cr);

      // - text
      text = "L" + to_string(cache_num + 1) + " Cache";
      cairo_set_font_size(cr, text_size_medium);
      cairo_text_extents(cr, text.c_str(), &text_extents);
      cairo_move_to(
        cr,
        core_x + core_width/2 - text_extents.width/2,
        cache_y + cache_height * cache_num + cache_height/2 + 
          text_extents.height/2);
      cairo_text_path(cr, text.c_str());
      // cairo_show_text(cr, text);
      cairo_set_line_width(cr, text_line_thickness);
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_fill_preserve(cr);
      cairo_stroke(cr); 
    }
  }

  // --- done drawing things, clean up

  // svg file automatically gets written to disk
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}

void visualize(
  std::string perfmon_output_filename,
  std::string image_output_filename,
  rgb_color min_color,
  rgb_color max_color)
{
  // std::string image_output_filename = "perfmon_output.svg";

  // read a JSON file
  std::ifstream i(perfmon_output_filename);
  json j;
  i >> j;

  std::string region_name = "copy";
  json region_data = j["saturation"][region_name];

  auto colors = calculate_saturation_colors(
    region_data,
    min_color,
    max_color);

  draw_diagram(colors, region_data, region_name, image_output_filename);
}

int main(int argc, char *argv[])
{
  // places where argument values will be stored
  std::uint64_t sp_flop_num_iterations;
  std::uint64_t dp_flop_num_iterations;

  std::vector<std::uint64_t> cache_and_memory_args;

  // for use with csv output: keeps a consistent name no matter what vector is
  // used for arguments
  std::uint64_t num_memory_iter = 0;
  std::uint64_t memory_size_kb = 0;

  std::string perfmon_output_filename;

  char time_str[20];
  std::time_t t;
  std::strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H%M", 
    std::localtime(&t));
  std::string image_output_filename = "perfmon_output_";
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
    ("csv-style-output,c", "output benchmark results in a csv-style format:\n")
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
    ("--image-output,o", po::value<std::string>(&image_output_filename), 
      "Path where visualization should be output to. Region name will "
      "automatically be appended. If not supplied, a default name with "
      "dateTime information will be generated.")
    ("test-color-lerp", "create band of color from least to most to test "
                        "linear interpolation")
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
                << m["DATA"]["DATA"][load_to_store_ratio_metric_name] << ","
                << e["DATA"]["DATA"]["MEM_INST_RETIRED_ALL_LOADS"] 
                   * CACHE_LINE_SIZE_BYTES * BYTES_TO_GBYTES << ","
                << e["DATA"]["DATA"]["MEM_INST_RETIRED_ALL_STORES"] 
                   * CACHE_LINE_SIZE_BYTES * BYTES_TO_GBYTES << ","
                << (e["DATA"]["DATA"]["MEM_INST_RETIRED_ALL_LOADS"] 
                   + e["DATA"]["DATA"]["MEM_INST_RETIRED_ALL_STORES"])
                   * CACHE_LINE_SIZE_BYTES * BYTES_TO_GBYTES << ","
                << m["L2"]["L2"][l2_bandwidth_metric_name] << ","
                << m["L2"]["L2"][l2_data_volume_name] << ","
                << m["L2"]["L2"][l2_evict_bandwidth_name] << ","
                << m["L2"]["L2"][l2_evict_data_volume_name] << ","
                << m["L2"]["L2"][l2_load_bandwidth_name] << ","
                << m["L2"]["L2"][l2_load_data_volume_name] << ","
                << m["L3"]["L3"][l3_bandwidth_metric_name] << ","
                << m["L3"]["L3"][l3_data_volume_name] << ","
                << m["L3"]["L3"][l3_evict_bandwidth_name] << ","
                << m["L3"]["L3"][l3_evict_data_volume_name] << ","
                << m["L3"]["L3"][l3_load_bandwidth_name] << ","
                << m["L3"]["L3"][l3_load_data_volume_name] << ","
                << m["MEM"]["MEM"][ram_bandwidth_metric_name] << ","
                << m["MEM"]["MEM"][ram_data_volume_metric_name] << ","
                << m["MEM"]["MEM"][ram_evict_bandwidth_name] << ","
                << m["MEM"]["MEM"][ram_evict_data_volume_name] << ","
                << m["MEM"]["MEM"][ram_load_bandwidth_name] << ","
                << m["MEM"]["MEM"][ram_load_data_volume_name] << "\n"
                ;
    }
  }

  // visualization things
  if (vm.count("test-color-lerp"))
  {
    auto min_color = std::tuple<double, double, double>(
      255.0/255.0, 247/255.0, 251.0/255.0);
    auto max_color = std::tuple<double, double, double>(
      2.0/255.0, 56.0/255.0, 88.0/255.0);
    test_color_lerp(min_color, max_color, 20, 50, 100);
  }
  if (vm.count("visualize"))
  {
    auto min_color = std::tuple<double, double, double>(
      255.0/255.0, 247/255.0, 251.0/255.0);
    auto max_color = std::tuple<double, double, double>(
      2.0/255.0, 56.0/255.0, 88.0/255.0);
    visualize(perfmon_output_filename, image_output_filename, min_color, 
      max_color);
  }

  return 0;
}
