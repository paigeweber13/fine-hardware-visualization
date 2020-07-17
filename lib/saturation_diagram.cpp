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

double saturation_diagram::pango_cairo_draw_text(
  cairo_t * cr,
  double x,
  double y,
  double text_box_width,
  std::string text,
  PangoFontDescription * font_desc,
  PangoAlignment alignment,
  bool vertical)
{
  cairo_save(cr);

  const double spacing_factor = 0.3;
  const int text_size = pango_font_description_get_size(font_desc);

  int height;
  double cairo_height;

  PangoLayout *layout = pango_cairo_create_layout(cr);
  pango_layout_set_font_description(layout, font_desc);
  pango_layout_set_text(layout, text.c_str(), -1);
  pango_layout_set_width(layout, text_box_width * PANGO_SCALE);
  pango_layout_set_alignment(layout, alignment);
  pango_layout_set_spacing(layout, 
    static_cast<int>(static_cast<double>(text_size) * spacing_factor)
  );
  cairo_move_to(cr, x, y);
  cairo_set_source_rgb(cr, 0, 0, 0);

  if (vertical) {
    cairo_rel_move_to(cr, 0, text_box_width);
    cairo_rotate(cr, -90 * G_PI / 180.0);
  }

  pango_cairo_update_layout(cr, layout);
  pango_cairo_show_layout(cr, layout);

  pango_layout_get_size(layout, NULL, &height);
  cairo_height = ((double)height / PANGO_SCALE);

  cairo_restore(cr);
  g_object_unref(layout);

  return cairo_height;
}

double saturation_diagram::cairo_draw_component(
  cairo_t *cr,
  double x,
  double y,
  double width,
  double height,
  rgb_color fill_color,
  std::string label,
  PangoFontDescription * font_desc,
  label_position position)
{
  cairo_save(cr);

  double text_height = 0; // height in vertical dimension relative to text

  int old_font_size = pango_font_description_get_size(font_desc);
  std::cout << "old font size: " << old_font_size << std::endl;
  std::cout << "new font size: " <<  (height/4) * PANGO_SCALE << std::endl;
  pango_font_description_set_size(font_desc, (height/4) * PANGO_SCALE);

  if(position == label_position::INSIDE)
  {
    cairo_rectangle(cr, x, y, width, height);
    text_height = 0;
  }
  else if(position == label_position::LEFT)
  {
    text_height = pango_cairo_draw_text(cr, x, y, height, label,
      font_desc, PANGO_ALIGN_CENTER, true);
    cairo_rectangle(cr, x + text_height, y,
      width - text_height, height);
  }
  else if(position == label_position::BOTTOM)
  {
    // really need to know height before drawing, because y position should be
    // y + height - text_height. For now, not resizing rectangle
    text_height = pango_cairo_draw_text(cr, x,
      y + height, width, label, font_desc, 
      PANGO_ALIGN_CENTER);
    // cairo_rectangle(cr, x, y, width, height - text_height);
    cairo_rectangle(cr, x, y, width, height);
  }

  cairo_set_source_rgb(
    cr,
    std::get<0>(fill_color),
    std::get<1>(fill_color),
    std::get<2>(fill_color));
  cairo_fill_preserve(cr);

  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_stroke(cr);

  if(position == label_position::INSIDE)
  {
    // because text is on top of rectangle, have to draw it AFTER fill and
    // stroke happen
    pango_cairo_draw_text(cr, x, y + height/4, width, label,
      font_desc, PANGO_ALIGN_CENTER);
    text_height = 0;
  }

  cairo_restore(cr);
  pango_font_description_set_size(font_desc, old_font_size);

  return text_height;
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

  // variables for cairo
  PangoFontDescription *title_font = pango_font_description_from_string ("Sans 40");
  PangoFontDescription *description_font = pango_font_description_from_string ("Sans 12");
  PangoFontDescription *big_label_font = pango_font_description_from_string ("Sans 40");
  PangoFontDescription *small_label_font = pango_font_description_from_string ("Sans 25");

  // general cairo constants
  const double image_width = 800;
  const double image_height = 1730;
  const double margin_x = 50;
  const double margin_y = 50;
  const double content_width = image_width - 2*margin_x;
  const double internal_margin = 25;
  const double line_thickness = 10.0;

  // should probably be in architecture, perhaps derived from likwid's
  // L*_CACHE_GROUPS 
  const unsigned num_attached_caches = 2;

  // size of different diagram components
  const double swatch_height = 50;
  const double ram_width = 700;
  const double ram_height = 200;
  const double l3_width = 400;
  const double l3_height = 100;
  const double socket0_width = 700;
  const double socket0_height = 700;
  const double core_width = 550;
  const double core_height = 175;
  const double core_cache_height = 50;
  const double core_and_cache_height = core_height + (core_cache_height * num_attached_caches);
  
  double text_height;

  cairo_surface_t *surface = cairo_svg_surface_create(
    output_filename.c_str(),
    image_width,
    image_height
  );
  cairo_t *cr = cairo_create(surface);
  cairo_set_line_width(cr, line_thickness);

  // --- title and description text --- //
  double title_x = margin_x;
  double title_y = margin_y;

  text_height = pango_cairo_draw_text(cr, title_x, title_y, content_width,
    "Saturation diagram for region\n\"" + region_name + "\"", 
    title_font, PANGO_ALIGN_CENTER);
  // add some spacing after big text
  
  double description_x = title_x;
  double description_y = title_y + text_height + internal_margin;

  std::string description;
  if(parameters.compare("") != 0)
    description += parameters + "\n\n";

  // computation color notes
  description += 
    "Note: both single- and double-precision floating point operations were "
    "measured. Here the cores are visualized using " + chosen_precision + " "
    "saturation, because it was higher.\n\n";

  // l1 cache note
  description += 
    "Note: L1 cache is currently not measured and therefore will appear "
    "white. This is not an indication of L1 cache saturation.";

  text_height = pango_cairo_draw_text(cr, description_x, description_y, 
    content_width, description, description_font);

  // --- draw swatch/legend --- //
  double swatch_x = description_x;
  double swatch_y = description_y + text_height + internal_margin;
  cairo_draw_swatch(cr, min_color, max_color, swatch_x, swatch_y, 
    content_width, swatch_height, 10);

  double swatch_label_x = swatch_x;
  double swatch_label_y = swatch_y + swatch_height + internal_margin/2;
  text_height = pango_cairo_draw_text(cr, swatch_label_x, swatch_label_y, content_width, 
    "Low saturation", description_font, PANGO_ALIGN_LEFT);
  pango_cairo_draw_text(cr, swatch_label_x, swatch_label_y, content_width, 
    "High saturation", description_font, PANGO_ALIGN_RIGHT);

  // --- draw RAM --- //
  // TODO: replace this and other components with a custom "draw rectangle"
  // command 
  double ram_x = margin_x;
  double ram_y = swatch_label_y + text_height + internal_margin;
  cairo_draw_component(cr, ram_x, ram_y, ram_width, ram_height, 
    region_colors[ram_bandwidth_metric_name], "RAM", big_label_font,
    label_position::INSIDE);

  // --- line from RAM to L3 cache --- //
  double line_start_x = ram_x + ram_width/2;
  double line_start_y = ram_y + ram_height;
  double line_end_x = line_start_x;
  double line_end_y = line_start_y + 150;
  cairo_move_to(cr, line_start_x, line_start_y);
  cairo_line_to(cr, line_end_x, line_end_y);

  // --- draw L3 cache --- //
  cairo_save(cr);

  double l3_x = 200;
  double l3_y = line_end_y;
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

  cairo_restore(cr);

  // - L3 cache text
  pango_cairo_draw_text(cr, l3_x, l3_y + l3_height/4, l3_width, 
    "L3 Cache", big_label_font, PANGO_ALIGN_CENTER);

  // --- draw socket 0 --- //
  double socket0_x = margin_x;
  double socket0_y = l3_y + l3_height;
  cairo_rectangle(cr, socket0_x, socket0_y, socket0_width, socket0_height);
  // - fill
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_fill_preserve(cr);

  // - stroke
  cairo_set_line_width(cr, line_thickness);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_stroke(cr);

  // - text
  pango_cairo_draw_text(cr, socket0_x, 
    socket0_y + socket0_height + internal_margin, content_width, 
    "Socket 0", big_label_font, PANGO_ALIGN_CENTER);

  // --- draw cores --- //
  for (unsigned core_num = 0; core_num < CORES_PER_SOCKET; core_num++)
  {
    // cache numbers
    // unsigned between_core_buffer = 50;
    unsigned core_x = 150;
    unsigned core_y = socket0_y + margin_y
      + core_num * (
        internal_margin*2 + core_height +
        (num_attached_caches * core_cache_height)
      );

    // cache numbers that depend on core numbers
    unsigned cache_y = core_y + core_height;

    // - core text
    double x = socket0_x + internal_margin;
    double y = core_y;
    
    pango_cairo_draw_text(cr, x, y, core_and_cache_height, 
      "Core " + std::to_string(core_num), 
      big_label_font, PANGO_ALIGN_CENTER, true);    

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
      pango_cairo_draw_text(cr, thread_x, core_y + core_height/3, thread_width, 
        "Thread " + std::to_string(core_num * THREADS_PER_CORE + thread_num),
        big_label_font, PANGO_ALIGN_CENTER);
    }

    // --- caches attached to core:

    for (unsigned cache_num = 0; cache_num < num_attached_caches; cache_num++)
    {
      cairo_rectangle(cr, core_x, cache_y + core_cache_height * cache_num,
        core_width, core_cache_height);

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
      pango_cairo_draw_text(cr, core_x, 
        cache_y + core_cache_height * cache_num + core_cache_height/8,
        core_width, 
        "L" + std::to_string(cache_num + 1) + " Cache",
        small_label_font, PANGO_ALIGN_CENTER);
    }
  }

  // --- done drawing things, clean up
  pango_font_description_free(title_font);
  pango_font_description_free(description_font);
  pango_font_description_free(big_label_font);
  pango_font_description_free(small_label_font);

  // svg file automatically gets written to disk
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}
