#include "saturation_diagram.h"

rgb_color 
saturation_diagram::color_lerp( 
  rgb_color min_color, 
  rgb_color max_color, 
  double t) 
{
  return rgb_color(
    std::get<0>(min_color) + t * (std::get<0>(max_color) - std::get<0>(min_color)),
    std::get<1>(min_color) + t * (std::get<1>(max_color) - std::get<1>(min_color)),
    std::get<2>(min_color) + t * (std::get<2>(max_color) - std::get<2>(min_color))
  );
}

template<class T>
constexpr const T& 
saturation_diagram::clamp( const T& v, const T& lo, const T& hi )
{
    assert( !(hi < lo) );
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

double saturation_diagram::scale(double value){
  #define c 7
  return (log2(value) + c)/c;
}

// ---- calculate saturation colors ----- // 

std::map<std::string, rgb_color>
saturation_diagram::calculate_saturation_colors(
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

void saturation_diagram::cairo_draw_swatch(
  cairo_t *cr,
  rgb_color min_color, 
  rgb_color max_color,
  unsigned x,
  unsigned y,
  unsigned width,
  unsigned height,
  unsigned num_steps
  )
{
  cairo_save(cr);

  unsigned step_size = width/num_steps;

  for (unsigned i = 0; i < num_steps; i++){
    // shape
    cairo_rectangle(cr, x + i * step_size, y, step_size, height);

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

  cairo_restore(cr);
}

void saturation_diagram::test_color_lerp(
  rgb_color min_color, 
  rgb_color max_color,
  unsigned width,
  unsigned height,
  unsigned num_steps)
{
  std::string output_dir = "visualizations/swatches/";
  std::string image_output_filename = output_dir
    + std::to_string(static_cast<unsigned>(round(std::get<0>(min_color) * 255.0))) + ","
    + std::to_string(static_cast<unsigned>(round(std::get<1>(min_color) * 255.0))) + ","
    + std::to_string(static_cast<unsigned>(round(std::get<2>(min_color) * 255.0)))
    + "_to_"
    + std::to_string(static_cast<unsigned>(round(std::get<0>(max_color) * 255.0))) + ","
    + std::to_string(static_cast<unsigned>(round(std::get<1>(max_color) * 255.0))) + ","
    + std::to_string(static_cast<unsigned>(round(std::get<2>(max_color) * 255.0)))
    + ".svg";

  if(system(("mkdir -p " + output_dir).c_str()) != 0)
    std::cout << "WARN: there was a problem making the directory for "
      << "color swatches (" << output_dir << ")." << std::endl
      << "Swatch creation will likely fail.";

  // create surface and cairo object
  cairo_surface_t *surface = cairo_svg_surface_create(
    image_output_filename.c_str(),
    width,
    height
  );
  cairo_t *cr = cairo_create(surface);

  double line_thickness = 10.0;
  cairo_set_line_width(cr, line_thickness);

  cairo_draw_swatch(cr, min_color, max_color, 0, 0, width, height, num_steps);

  // --- done drawing things, clean up

  // svg file automatically gets written to disk
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}

void saturation_diagram::cairo_draw_sideways_text(
  cairo_t * cr, 
  double x,
  double y,
  double text_box_height,
  std::string text, 
  PangoFontDescription * font_desc)
{
  cairo_save(cr);

  PangoLayout *layout = pango_cairo_create_layout(cr);
  pango_layout_set_text(layout, text.c_str(), -1);
  pango_layout_set_font_description(layout, font_desc);
  
  int width, height;
  double cairo_width, cairo_height;
  pango_cairo_update_layout(cr, layout);
  pango_layout_get_size(layout, &width, &height);
  cairo_width = ((double)width / PANGO_SCALE);
  cairo_height = ((double)height / PANGO_SCALE);

  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_move_to(cr, x, y - text_box_height/2 + cairo_width/2);
  cairo_rotate(cr, -90.0 * G_PI / 180.0);

  pango_cairo_update_layout(cr, layout);
  pango_cairo_show_layout(cr, layout);

  cairo_restore(cr);
  g_object_unref(layout);

  cairo_move_to(cr, x + cairo_height, y);
}

void saturation_diagram::cairo_draw_text(
  cairo_t * cr,
  double x, 
  double y,
  std::string text,
  double text_size,
  double text_line_thickness
)
{
  cairo_save(cr);

  cairo_move_to(cr, x, y);

  cairo_text_path(cr, text.c_str());
  // cairo_show_text(cr, text);
  cairo_set_line_width(cr, text_line_thickness);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_fill_preserve(cr);
  cairo_stroke(cr); 

  cairo_restore(cr);
}

void saturation_diagram::draw_diagram(
  std::map<std::string, rgb_color> region_colors,
  json region_data,
  rgb_color min_color,
  rgb_color max_color,
  std::string region_name,
  std::string parameters,
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

  // valuse we'll pass to helper functions
  double x, y;
  double current_text_y;

  // variables for cairo
  const double line_thickness = 10.0;
  const double text_line_thickness_large = 1.0;
  const double text_line_thickness_small = 0.0;
  const double text_size_xlarge = 75.0;
  const double text_size_large = 50.0;
  const double text_size_medium = 30.0;
  const double text_size_small = 15.0;
  cairo_text_extents_t text_extents;
  std::string text;

  // initialization for cairo
  const double image_width = 800;
  const double image_height = 1730;
  const double margin_x = 50;
  const double margin_y = 50;
  const double internal_margin = 25;
  const double line_spacing = 10;
  // double title_text_height = 310;

  cairo_surface_t *surface = cairo_svg_surface_create(
    output_filename.c_str(),
    image_width,
    image_height
  );
  cairo_t *cr = cairo_create(surface);

  cairo_set_font_size(cr, text_size_large);
  
  // --- title and description text --- //
  text = "Saturation diagram for region";
  cairo_text_extents(cr, text.c_str(), &text_extents);
  current_text_y = margin_y + text_extents.height;
  x = image_width/2 - text_extents.width/2;
  cairo_draw_text(cr, x, current_text_y, text, text_size_large, 
    text_line_thickness_large);

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

  // spacing between title and small text
  current_text_y += 2 * line_spacing;

  if(parameters.compare("") != 0)
  {
    cairo_text_extents(cr, parameters.c_str(), &text_extents);
    current_text_y += line_spacing + text_extents.height;
    cairo_move_to(
      cr,
      margin_x,
      current_text_y);
    // cairo_set_text_width(cr, text_line_thickness_large);
    cairo_text_path(cr, parameters.c_str());
    cairo_fill_preserve(cr);
    cairo_stroke(cr); 
    current_text_y += line_spacing;
  }

  // computation color notes
  for(auto &line : computation_color_note){
    cairo_text_extents(cr, line.c_str(), &text_extents);
    current_text_y += line_spacing + text_extents.height;
    cairo_move_to(
      cr,
      margin_x,
      current_text_y);
    // cairo_set_line_width(cr, text_line_thickness_large);
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
    // cairo_set_line_width(cr, text_line_thickness_large);
    cairo_text_path(cr, line.c_str());
    cairo_fill_preserve(cr);
    cairo_stroke(cr); 
  }
  current_text_y += line_spacing;

  // --- draw swatch/legend --- //
  cairo_set_line_width(cr, line_thickness);
  unsigned swatch_height = 50;
  current_text_y += 2*line_spacing;
  cairo_draw_swatch(cr, min_color, max_color, margin_x, current_text_y, 
    image_width - 2 * margin_x, swatch_height, 10);

  // text settings
  current_text_y += swatch_height;
  cairo_set_line_width(cr, text_line_thickness_small);
  cairo_set_source_rgb(cr, 0, 0, 0);

  text = "Low saturation";
  cairo_text_extents(cr, text.c_str(), &text_extents);
  current_text_y += line_spacing + text_extents.height;
  cairo_move_to(
    cr,
    margin_x,
    current_text_y);
  cairo_text_path(cr, text.c_str());
  cairo_fill_preserve(cr);
  cairo_stroke(cr); 

  text = "High saturation";
  cairo_text_extents(cr, text.c_str(), &text_extents);
  cairo_move_to(
    cr,
    image_width - margin_x - text_extents.width,
    current_text_y);
  cairo_text_path(cr, text.c_str());
  cairo_fill_preserve(cr);
  cairo_stroke(cr); 

  current_text_y += line_spacing;

  // revert size and width changes
  cairo_set_font_size(cr, text_size_large);
  cairo_set_line_width(cr, text_line_thickness_large);


  // --- draw RAM --- //
  double ram_x = margin_x;
  double ram_y = internal_margin + current_text_y; 
  double ram_width = 700;
  double ram_height = 200;
  cairo_rectangle(cr, ram_x, ram_y, ram_width, ram_height);
  // - fill
  cairo_set_source_rgb(
    cr,
    std::get<0>(region_colors["Memory bandwidth [MBytes/s]"]),
    std::get<1>(region_colors["Memory bandwidth [MBytes/s]"]),
    std::get<2>(region_colors["Memory bandwidth [MBytes/s]"]));
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
  cairo_set_line_width(cr, text_line_thickness_large);
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
    std::get<0>(region_colors["L3 bandwidth [MBytes/s]"]),
    std::get<1>(region_colors["L3 bandwidth [MBytes/s]"]),
    std::get<2>(region_colors["L3 bandwidth [MBytes/s]"]));
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
  cairo_set_line_width(cr, text_line_thickness_large);
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
  cairo_set_line_width(cr, text_line_thickness_large);
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
    unsigned core_and_cache_height = core_height + (cache_height * num_attached_caches);
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
    cairo_set_font_size(cr, text_size_large);
    cairo_set_line_width(cr, text_line_thickness_large);

    text = "Core " + std::to_string(core_num);
    cairo_text_extents(cr, text.c_str(), &text_extents);
    x = socket0_x + internal_margin;
    y = core_y + core_and_cache_height;

    PangoFontDescription *desc = pango_font_description_from_string ("Sans Bold 40");
    
    cairo_draw_sideways_text(cr, x, y, core_and_cache_height, 
      text, desc);
    
    pango_font_description_free(desc);
    // TODO: use the current cursor location here instead of manually
    // calculating where I should draw next. This way if the size changes, the
    // x postion where the core/caches are drawn will automatically adjust

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
          std::get<0>(computation_color),
          std::get<1>(computation_color),
          std::get<2>(computation_color));
      cairo_fill_preserve(cr);

      // - stroke
      cairo_set_line_width(cr, line_thickness);
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_stroke(cr);

      // - text
      text = "Thread " + std::to_string(core_num * THREADS_PER_CORE + thread_num);
      cairo_text_extents(cr, text.c_str(), &text_extents);
      cairo_move_to(
        cr,
        thread_x + thread_width/2 - text_extents.width/2,
        core_y + core_height/2 + text_extents.height/2);
      cairo_text_path(cr, text.c_str());
      // cairo_show_text(cr, text);
      cairo_set_line_width(cr, text_line_thickness_large);
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
          std::get<0>(region_colors["L" + std::to_string(cache_num + 1) + 
            " bandwidth [MBytes/s]"]),
          std::get<1>(region_colors["L" + std::to_string(cache_num + 1) + 
            " bandwidth [MBytes/s]"]),
          std::get<2>(region_colors["L" + std::to_string(cache_num + 1) + 
            " bandwidth [MBytes/s]"]));
      }
      cairo_fill_preserve(cr);

      // - stroke
      cairo_set_line_width(cr, line_thickness);
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_stroke(cr);

      // - text
      text = "L" + std::to_string(cache_num + 1) + " Cache";
      cairo_set_font_size(cr, text_size_medium);
      cairo_text_extents(cr, text.c_str(), &text_extents);
      cairo_move_to(
        cr,
        core_x + core_width/2 - text_extents.width/2,
        cache_y + cache_height * cache_num + cache_height/2 + 
          text_extents.height/2);
      cairo_text_path(cr, text.c_str());
      // cairo_show_text(cr, text);
      cairo_set_line_width(cr, text_line_thickness_large);
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
