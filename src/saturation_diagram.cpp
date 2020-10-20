#include "saturation_diagram.hpp"

rgb_color 
saturation_diagram::color_lerp( 
  const rgb_color &min_color, 
  const rgb_color &max_color, 
  const double &t) 
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

double saturation_diagram::scale(const double value){
  #define c 7
  return (log2(value) + c)/c;
}

// ---- calculate saturation colors ----- // 
rgb_color saturation_diagram::calculate_single_color(
  const double &value,
  const rgb_color &min_color,
  const rgb_color &max_color)
{
  double this_saturation = value;
  // clamp values to [0.0,1.0]
  this_saturation = clamp(static_cast<double>(this_saturation), 0.0, 1.0);
  // scale using custom function
  this_saturation = scale(this_saturation);
  // clamp again because scale can give negative values for very small input
  this_saturation = clamp(static_cast<double>(this_saturation), 0.0, 1.0);

  return color_lerp(min_color, max_color, this_saturation);
}

json
saturation_diagram::calculate_saturation_colors(
  const json &region_data,
  const rgb_color &min_color,
  const rgb_color &max_color)
{
  json saturation_colors;

  for (const auto &region_section: region_data.items())
  {
    // TODO: combine the two for loops (and if statements) below to make a
    // single loop. This loop would iterate over a data structure that
    // incorporates everything we need a color for. Something like
    // "color_metrics" instead of "saturation_metrics" and
    // "port_usage_metrics". Doing both per-thread and geometric_mean cases for
    // everything will make combining these loops simpler 

    // for now, we're just doing an overview to keep things simple

    // if we're in the "geometric_mean" part of this region's results, then
    // we'll search for each saturation metric
    if(region_section.key() == fhv::types::aggregationTypeToString(
            fhv::types::aggregation_t::saturation) )
    {
      for (const auto &metric: region_section.value().items())
      {
        for (const auto &saturation_metric : fhv_saturation_metric_names)
        {
          if (metric.key() == saturation_metric)
          {
            saturation_colors[region_section.key()][metric.key()] 
              = saturation_diagram::calculate_single_color(metric.value(), 
              min_color, max_color);
          }
        }
      }
    }

    if(region_section.key() == fhv::types::aggregationTypeToString(
            fhv::types::aggregation_t::geometric_mean) )
    {
      for (const auto &metric: region_section.value().items())
      {
        for (const auto &other_diagram_metric : fhv_other_diagram_metrics)
        {
          if (metric.key() == other_diagram_metric)
          {
            saturation_colors[region_section.key()][metric.key()] 
              = saturation_diagram::calculate_single_color(metric.value(), 
              min_color, max_color);
          }
        }
      }
    }
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

  cairo_draw_swatch(cr, min_color, max_color, 0, 0, width, height, num_steps);

  // --- done drawing things, clean up

  // svg file automatically gets written to disk
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
}

void saturation_diagram::pango_cairo_make_text_layout(
  PangoLayout *layout,
  PangoFontDescription *font_desc,
  std::string text,
  int cairo_width,
  PangoAlignment alignment,
  int cairo_height)
{
  const double spacing_factor = 0.3;
  const int text_size = pango_font_description_get_size(font_desc);
  pango_layout_set_font_description(layout, font_desc);
  pango_layout_set_text(layout, text.c_str(), -1);
  pango_layout_set_width(layout, cairo_width * PANGO_SCALE);

  if (cairo_height < -1)
    pango_layout_set_height(layout, cairo_height);
  else if (cairo_height > -1)
    pango_layout_set_height(layout, cairo_height * PANGO_SCALE);
  // if cairo_height is equal to -1, we do nothing because that is the default
  // value of cairo_height.

  pango_layout_set_alignment(layout, alignment);
  pango_layout_set_spacing(layout, 
    static_cast<int>(static_cast<double>(text_size) * spacing_factor)
  );
}

void saturation_diagram::pango_cairo_draw_layout(
  cairo_t * cr,
  double x,
  double y,
  PangoLayout *layout,
  bool vertical)
{
  cairo_save(cr);

  cairo_move_to(cr, x, y);
  cairo_set_source_rgb(cr, 0, 0, 0);

  int text_box_width = pango_layout_get_width(layout) / PANGO_SCALE;

  if (vertical) {
    cairo_rel_move_to(cr, 0, text_box_width);
    // cairo_rotate(cr, -90 * G_PI / 180.0);
    cairo_rotate(cr, -G_PI/2);
  }

  pango_cairo_update_layout(cr, layout);
  pango_cairo_show_layout(cr, layout);

  cairo_restore(cr);
}

double saturation_diagram::pango_cairo_draw_text(
  cairo_t * cr,
  double x,
  double y,
  int text_box_width,
  std::string text,
  PangoFontDescription * font_desc,
  PangoAlignment alignment,
  bool vertical)
{
  int height;
  double cairo_height;

  PangoLayout *layout = pango_cairo_create_layout(cr);

  // create layout
  pango_cairo_make_text_layout(layout, font_desc, text, text_box_width, 
    alignment);

  pango_cairo_draw_layout(cr, x, y, layout, vertical);

  pango_layout_get_size(layout, NULL, &height);
  cairo_height = ((double)height / PANGO_SCALE);

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
  label_position position,
  double stroke_width)
{
  cairo_save(cr);

  // sets what fraction of the text size should be applied as padding between
  // text and box in the case of positions BOTTOM and LEFT
  const double PADDING_RATIO_LEFT = 1.0/6.0;
  const double PADDING_RATIO_BOTTOM = 1.0/3.0;

  // height in vertical dimension relative to text
  int text_height; // pango units
  double cairo_text_height = 0; // cairo units

  cairo_set_line_width(cr, stroke_width);

  PangoLayout *layout = pango_cairo_create_layout(cr);

  if(position == label_position::INSIDE)
  {
    pango_cairo_make_text_layout(layout, font_desc, label, width,
      PANGO_ALIGN_CENTER, height);
    pango_layout_get_size(layout, NULL, &text_height);
    cairo_text_height = static_cast<double>(text_height) / PANGO_SCALE;

    cairo_rectangle(cr, x, y, width, height);
  }
  else if(position == label_position::LEFT)
  {
    pango_cairo_make_text_layout(layout, font_desc, label, height, 
      PANGO_ALIGN_CENTER);
    pango_layout_get_size(layout, NULL, &text_height);
    cairo_text_height = static_cast<double>(text_height) / PANGO_SCALE;
    pango_cairo_draw_layout(cr, x, y, layout, true);

    // there's not a lot of distance between the text and box with larger
    // fonts, so let's add a small margin:
    cairo_text_height += cairo_text_height * PADDING_RATIO_LEFT;
    
    cairo_rectangle(cr, x + cairo_text_height, y, width - cairo_text_height,
      height);
    // cairo_rectangle(cr, x, y, width, height);
  }
  else if(position == label_position::BOTTOM)
  {
    pango_cairo_make_text_layout(layout, font_desc, label, width, 
      PANGO_ALIGN_CENTER);
    pango_layout_get_size(layout, NULL, &text_height);
    cairo_text_height = static_cast<double>(text_height) / PANGO_SCALE;
    pango_cairo_draw_layout(cr, x, y + height - cairo_text_height, layout);

    // there's not a lot of distance between the text and box with larger
    // fonts, so let's add a small margin:
    cairo_text_height += cairo_text_height * PADDING_RATIO_BOTTOM;

    cairo_rectangle(cr, x, y, width, height - cairo_text_height);
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
    pango_cairo_draw_layout(cr, x, y + height/2 - cairo_text_height/2, layout); 
    
    // set to zero because 0 offset is needed to reach rectangle
    cairo_text_height = 0;
  }

  cairo_restore(cr);
  g_object_unref(layout);

  return cairo_text_height;
}

void saturation_diagram::cairo_draw_arrow(
  cairo_t *cr,
  double x,
  double y,
  double width,
  double height,
  rgb_color fill_color,
  direction arrow_direction,
  std::string label,
  PangoFontDescription * font_desc,
  double stroke_width
)
{
  cairo_save(cr);

  // ratios of arrowhead size to box size
  
  // Must be >= 1.0. Arrowhead will extend past width
  double arrowhead_width_ratio = 1.5; 
  // Must be <= 1.0. Arrowhead will take part of total height 
  double arrowhead_height_ratio = 0.35;

  cairo_move_to(cr, x, y);
  cairo_set_line_width(cr, stroke_width);

  cairo_save(cr); // start rotated things
  {
    double arrow_width = width;
    double arrow_height = height;

    // default direction is down. No changes needed if down.
    if (arrow_direction == direction::UP) {
      cairo_rel_move_to(cr, width, height);
      cairo_rotate(cr, G_PI);
    }
    else if (arrow_direction == direction::LEFT) {
      cairo_rel_move_to(cr, width, 0);
      cairo_rotate(cr, G_PI / 2.0);
      arrow_width = height;
      arrow_height = width;
    }
    else if (arrow_direction == direction::RIGHT) {
      cairo_rel_move_to(cr, 0, height);
      cairo_rotate(cr, -G_PI / 2.0);
      arrow_width = height;
      arrow_height = width;
    }

    // working down the right side of the arrow
    
    // top horizontal line
    cairo_rel_line_to(cr, arrow_width, 0); 

    // right vertical line
    cairo_rel_line_to(cr, 0, arrow_height * (1.0 - arrowhead_height_ratio)); 

    double arrowhead_width = arrow_width * arrowhead_width_ratio;
    double arrowhead_ledge_size = (arrowhead_width - arrow_width) / 2;

    // right arrowhead ledge
    cairo_rel_line_to(cr, arrowhead_ledge_size, 0); 

    // right arrowhead slant
    cairo_rel_line_to(cr, -(arrowhead_width / 2.0), 
      arrow_height * arrowhead_height_ratio); 

    // working our way back up

    // left arrowhead slant
    cairo_rel_line_to(cr, -(arrowhead_width / 2.0), 
      -(arrow_height * arrowhead_height_ratio)); 
    
    // left arrowhead ledge
    cairo_rel_line_to(cr, arrowhead_ledge_size, 0); 

    // right vertical line
    cairo_rel_line_to(cr, 0, 
      -(arrow_height * (1.0 - arrowhead_height_ratio))); 
  }
  cairo_restore(cr); // done with rotated things

  cairo_set_source_rgb(
    cr,
    std::get<0>(fill_color),
    std::get<1>(fill_color),
    std::get<2>(fill_color));
  cairo_fill_preserve(cr);

  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_stroke(cr);

  // because text is on top of rectangle, have to draw it AFTER fill and
  // stroke happen

  // height in vertical dimension relative to text
  int text_height; // pango units
  double cairo_text_height; // cairo units

  PangoLayout *layout = pango_cairo_create_layout(cr);

  pango_cairo_make_text_layout(layout, font_desc, label, width,
    PANGO_ALIGN_CENTER, height);
  pango_layout_get_size(layout, NULL, &text_height);
  cairo_text_height = static_cast<double>(text_height) / PANGO_SCALE;

  pango_cairo_draw_layout(cr, x, y + height/2 - cairo_text_height/2, layout); 

  // cleanup
  cairo_restore(cr);
  g_object_unref(layout);
}

void saturation_diagram::draw_diagram_overview(
  json region_colors,
  rgb_color min_color,
  rgb_color max_color,
  std::string region_name,
  std::string parameters,
  std::string output_filename
)
{
  // variables for cairo
  PangoFontDescription *title_font = pango_font_description_from_string ("Sans 40");
  PangoFontDescription *description_font = pango_font_description_from_string ("Sans 12");
  PangoFontDescription *big_label_font = pango_font_description_from_string ("Sans 40");
  PangoFontDescription *small_label_font = pango_font_description_from_string ("Sans 25");

  // TODO: NO LONGER USED. Will this be used eventually? Remove??
  
  // TODO: should be in architecture, perhaps derived from likwid's
  // L*_CACHE_GROUPS 

  // const unsigned num_attached_caches = 2;

  /* ----- drawing constants ----- 
   *
   * these may be changed to configure the drawing. 
   * 
   * TODO: someday, move these to a config file
   */
  const double image_width = 1200;
  const double image_height = 2400;

  const double margin_x = 50;
  const double margin_y = 50;

  const double small_internal_margin = 12;
  const double internal_margin = 25;
  const double large_internal_margin = 50;

  // size of different diagram components
  const double content_width = image_width - 2 * margin_x;

  const double swatch_width = content_width;
  const double swatch_height = 50;

  // memory/cache things
  const double ram_width = content_width;
  const double ram_height = 200;
  const double transfer_arrow_height = 150;
  const double l3_width = ram_width;
  const double l3_height = 100;
  const double l2_width = l3_width;
  const double l2_height = l3_height;
  const double l1_width = l2_width;
  const double l1_height = l2_height;
  // TODO: NO LONGER USED. Will this be used eventually? Remove??

  // const double core_cache_height = 50;

  const double core_width = content_width ;
  const double core_height = 500;

  double text_height;

  fhv::utils::create_directories_for_file(output_filename);

  cairo_surface_t *surface = cairo_svg_surface_create(
    output_filename.c_str(),
    image_width,
    image_height
  );
  cairo_t *cr = cairo_create(surface);

  // --- title and description text --- //
  double title_x = margin_x;
  double title_y = margin_y;

  text_height = pango_cairo_draw_text(cr, title_x, title_y, content_width,
    "Saturation diagram for region\n\"" + region_name + "\"", 
    title_font, PANGO_ALIGN_CENTER);
  
  double description_x = title_x;
  double description_y = title_y + text_height + large_internal_margin;

  std::string description;
  if(parameters.compare("") != 0)
    description += parameters + "\n\n";

  // l1 cache note
  description += 
    "Note: L1 cache is currently not measured and therefore will appear "
    "white. This is not an indication of L1 cache saturation.";

  text_height = pango_cairo_draw_text(cr, description_x, description_y, 
    content_width, description, description_font);

  // --- draw swatch/legend --- //
  // TODO : make drawing swatch/legend own function. Add numbers for saturation
  // as key
  double swatch_x = description_x;
  double swatch_y = description_y + text_height + internal_margin;
  double num_steps = 10;
  cairo_draw_swatch(cr, min_color, max_color, swatch_x, swatch_y, 
    swatch_width, swatch_height, num_steps);

  double swatch_legend_x = swatch_x;
  double swatch_legend_y = swatch_y + swatch_height + small_internal_margin;

  // not sure how to do this without hard-coding. However, since we are
  // aligning right, it can be very large
  double single_legend_item_width = 100; 

  double legend_offset = -10;
  double scaled_value;
  for (unsigned i = 0; i < static_cast<unsigned>(num_steps) + 1; i++)
  {
    scaled_value = clamp(scale(static_cast<double>(i)/num_steps), 0.0, 1.0);
    std::stringstream value_text;
    value_text << std::setprecision(1) << std::fixed
      << static_cast<double>(i)/num_steps;
    text_height = pango_cairo_draw_text(cr, 
      swatch_legend_x + legend_offset + scaled_value * content_width, 
      swatch_legend_y, single_legend_item_width, value_text.str(),
      description_font, PANGO_ALIGN_RIGHT, true);
  }

  double swatch_label_x = swatch_x;
  double swatch_label_y = swatch_legend_y + text_height + small_internal_margin; 
  text_height = pango_cairo_draw_text(cr, swatch_label_x, swatch_label_y, 
    content_width, "Saturation level (higher is usually better)", 
    description_font, PANGO_ALIGN_CENTER);

  // --- draw RAM --- //
  double ram_x = margin_x;
  double ram_y = swatch_label_y + text_height + internal_margin;
  cairo_draw_component(cr, ram_x, ram_y, ram_width, ram_height, 
    region_colors[fhv::types::aggregationTypeToString(
            fhv::types::aggregation_t::saturation)]
      [fhv_mem_rw_saturation_metric_name], "RAM", big_label_font,
    label_position::INSIDE);

  // --- Load/store arrows from RAM to L3 cache --- //
  double ram_l3_load_x = ram_x + ram_width/5.0;
  double ram_l3_store_x = ram_x + 3.0*ram_width/5.0;
  double ram_l3_arrow_y = ram_y + ram_height;
  double ram_l3_arrow_width = ram_width / 5.0;

  // TODO: replace with saturation color
  cairo_draw_arrow(cr, ram_l3_load_x, ram_l3_arrow_y, ram_l3_arrow_width,
    transfer_arrow_height, 
    region_colors[fhv::types::aggregationTypeToString(
            fhv::types::aggregation_t::saturation)]
      [fhv_mem_r_saturation_metric_name],
    direction::DOWN, "load", small_label_font, stroke_thickness_thin);

  cairo_draw_arrow(cr, ram_l3_store_x, ram_l3_arrow_y, ram_l3_arrow_width,
    transfer_arrow_height, 
    region_colors[fhv::types::aggregationTypeToString(
            fhv::types::aggregation_t::saturation)]
      [fhv_mem_w_saturation_metric_name],
    direction::UP, "store", small_label_font, stroke_thickness_thin);

  // --- draw L3 cache --- //
  double l3_x = ram_x;
  double l3_y = ram_l3_arrow_y + transfer_arrow_height;
  cairo_draw_component(cr, l3_x, l3_y, l3_width, l3_height, 
    region_colors
      [fhv::types::aggregationTypeToString(
            fhv::types::aggregation_t::saturation)]
      [fhv_l3_rw_saturation_metric_name], "L3 Cache", big_label_font);

  // --- Load/store arrows from L3 to L2 cache --- //
  double l3_l2_load_x = ram_l3_load_x;
  double l3_l2_store_x = ram_l3_store_x;
  double l3_l2_arrow_y = l3_y + l3_height;
  double l3_l2_arrow_width = l3_width / 5.0;

  // TODO: replace with saturation color
  cairo_draw_arrow(cr, l3_l2_load_x, l3_l2_arrow_y, l3_l2_arrow_width,
    transfer_arrow_height, 
    region_colors[fhv::types::aggregationTypeToString(
            fhv::types::aggregation_t::saturation)]
      [fhv_l3_r_saturation_metric_name],
    direction::DOWN, "load", small_label_font, stroke_thickness_thin);

  cairo_draw_arrow(cr, l3_l2_store_x, l3_l2_arrow_y, l3_l2_arrow_width,
    transfer_arrow_height, 
    region_colors[fhv::types::aggregationTypeToString(
            fhv::types::aggregation_t::saturation)]
      [fhv_l3_w_saturation_metric_name],
    direction::UP, "store", small_label_font, stroke_thickness_thin);

  // --- draw L2 cache --- //
  double l2_x = l3_x;
  double l2_y = l3_l2_arrow_y + transfer_arrow_height;
  cairo_draw_component(cr, l2_x, l2_y, l2_width, l2_height, 
    region_colors
      [fhv::types::aggregationTypeToString(
            fhv::types::aggregation_t::saturation)]
      [fhv_l2_rw_saturation_metric_name], "L2 Cache", big_label_font);

  // --- Load/store arrows from L2 to L1 cache --- //
  double l2_l1_load_x = l3_l2_load_x;
  double l2_l1_store_x = l3_l2_store_x;
  double l2_l1_arrow_y = l2_y + l2_height;
  double l2_l1_arrow_width = l2_width / 5.0;

  // TODO: replace with saturation color
  cairo_draw_arrow(cr, l2_l1_load_x, l2_l1_arrow_y, l2_l1_arrow_width,
    transfer_arrow_height, 
    region_colors[fhv::types::aggregationTypeToString(
            fhv::types::aggregation_t::saturation)]
      [fhv_l2_r_saturation_metric_name],
    direction::DOWN, "load", small_label_font, stroke_thickness_thin);

  cairo_draw_arrow(cr, l2_l1_store_x, l2_l1_arrow_y, l2_l1_arrow_width,
    transfer_arrow_height, 
    region_colors[fhv::types::aggregationTypeToString(
            fhv::types::aggregation_t::saturation)]
      [fhv_l2_w_saturation_metric_name],
    direction::UP, "store", small_label_font, stroke_thickness_thin);

  // --- draw L1 cache --- //
  double l1_x = l2_x; 
  double l1_y = l2_l1_arrow_y + transfer_arrow_height;
  cairo_draw_component(cr, l1_x, l1_y, l1_width, l1_height, 
    rgb_color(1,1,1), "L1 Cache", big_label_font);

  // --- draw core container block --- //
  double core_x = margin_x;
  double core_y = l1_y + l1_height;
  cairo_draw_component(cr, core_x, core_y, core_width, 
    core_height, rgb_color(1, 1, 1), "", big_label_font, 
    label_position::INSIDE);
  
  // --- draw block with "in-core performance" label --- //
  double in_core_x = core_x + internal_margin;
  double in_core_y = core_y + internal_margin;
  double in_core_height = core_height - 2 * internal_margin;
  double in_core_width = core_width - 2 * internal_margin;
  text_height = cairo_draw_component(cr, in_core_x, in_core_y, in_core_width, 
    in_core_height, rgb_color(1, 1, 1), "In-core performance", big_label_font, 
    label_position::LEFT);

  // --- draw FLOPs saturations

  // for both single and double precision
  double flops_width = (in_core_width - text_height) / 2.0;
  double flops_height = in_core_height * 2.0/3.0;

  // single precision
  double single_p_x = in_core_x + text_height;
  double single_p_y = in_core_y;
  cairo_draw_component(cr, single_p_x, single_p_y, flops_width, flops_height, 
    region_colors[fhv::types::aggregationTypeToString(
            fhv::types::aggregation_t::saturation)]
      [fhv_flops_dp_saturation_metric_name], 
    "Single-precision FLOP/s", big_label_font, label_position::INSIDE);

  // double precision
  double double_p_x = single_p_x + flops_width;
  double double_p_y = single_p_y;
  cairo_draw_component(cr, double_p_x, double_p_y, flops_width, flops_height, 
    region_colors[
      fhv::types::aggregationTypeToString(
              fhv::types::aggregation_t::saturation)]
      [fhv_flops_sp_saturation_metric_name],
    "Double-precision FLOP/s", big_label_font, label_position::INSIDE);
  
  // --- draw ports in core
  double port_width = (in_core_width - text_height) * 
    (1.0/static_cast<double>(NUM_PORTS_IN_CORE));
  double port_height = in_core_height - flops_height;
  double port_y = in_core_y + flops_height;
  double port_x;

  for (unsigned port_num = 0; port_num < NUM_PORTS_IN_CORE; port_num++)
  {
    port_x = in_core_x + text_height + port_num * port_width;
    cairo_draw_component(cr, port_x, port_y, port_width, port_height,
      region_colors
        [fhv::types::aggregationTypeToString(
              fhv::types::aggregation_t::geometric_mean)]
        [fhv_port_usage_ratio_start + std::to_string(port_num) 
          + fhv_port_usage_ratio_end],
      "Port " + std::to_string(port_num), 
      small_label_font, label_position::INSIDE, stroke_thickness_thin);
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

void saturation_diagram::draw_diagram_detail(
  json region_colors,
  rgb_color min_color,
  rgb_color max_color,
  std::string region_name,
  std::string parameters,
  std::string output_filename
)
{
  ;
}
