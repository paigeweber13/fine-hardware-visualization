#pragma once

#include <cairo.h>
#include <cairo-svg.h>
#include <fmt/core.h>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <map>
#include <pango/pangocairo.h>
#include <tuple>
#include <sstream>
#include <string>
#include <type_traits>

#include "likwid_defines.hpp"
// needed for "aggregationTypeToString"
#include "fhv_perfmon.hpp"
#include "utils.hpp"
#include "performance_monitor_defines.hpp"

using json = nlohmann::json;

// ----- simple color type ----- //
typedef std::tuple<double, double, double> rgb_color;

enum class label_position {
  INSIDE, BOTTOM, LEFT
};

enum class direction {
  UP, RIGHT, DOWN, LEFT
};

// magic numbers

// TODO: should anything else be here?
const double stroke_thickness_normal = 10.0;
const double stroke_thickness_thin = 5.0;
const rgb_color WHITE(1.0, 1.0, 1.0);

class saturation_diagram {
  public:
    /* ======== Primary functions ======== 
     * These are the high-level functions intended primarily for use outside 
     * this class. Intended usage is demonstrated in fhv.cpp::visualize.
     */
    /* ---- test color lerp ----
     * Draws a swatch of a gradient calculated using color_lerp to test before
     * using in a saturation diagram
     */
    static void test_color_lerp(
      rgb_color min_color, 
      rgb_color max_color,
      unsigned width,
      unsigned height,
      unsigned num_steps);
      
    static void test_discrete_color_scale(
      unsigned width,
      unsigned height);

    static rgb_color calculate_single_color(
      const double &value,
      const std::string &color_scale);

    /* ---- calculate saturation colors ----- 
     * The return value of this function is intended to be passed to
     * draw_diagram. 
     */ 
    static json
    calculate_saturation_colors(
      const json &region_data,
      const std::string &color_scale);

    /* ---- draw diagram ----
     * Draws an overview of the architecture that displays RAM, cores, and
     * caches 
     */
    static void draw_diagram_overview(
      json fhv_data,
      std::string color_scale,
      std::string region_name,
      std::string output_filename);

    /* ======== Helper functions: general ======== 
     * These may be used elsewhere but are intended for internal use. They
     * include things like clamping and scaling values that are applied before
     * calculating colors.
     */

    /* ----- LINEAR INTERPOLATION (LERP) ----- 
     * Used to create a gradient from min_color to max_color. This will be used
     * to indicate saturation.
     *
     * taken from
     * https://en.wikipedia.org/wiki/Linear_interpolation#Programming_language_support
     *
     * used under Creative Commons Attribution-ShareAlike 3.0 Unported License. See
     * https://en.wikipedia.org/wiki/Wikipedia:Text_of_Creative_Commons_Attribution-ShareAlike_3.0_Unported_License
     * for full text
     *
     * Imprecise method, which does not guarantee result = max_color when t = 1,
     * due to floating-point arithmetic error. This form may be used when the
     * hardware has a native fused multiply-add instruction.
     */
    static rgb_color color_lerp( 
      const rgb_color &min_color, 
      const rgb_color &max_color, 
      const double &t);

    static rgb_color discrete_color_scale(
      const std::string &scale_name,
      const double &t);

    /* ----- CLAMP ----- 
     * taken from: https://en.cppreference.com/w/cpp/algorithm/clamp
     */
    template<class T>
    static constexpr const T& clamp( const T& v, const T& lo, const T& hi );

    /* ---- custom log scale ---- 
     * designed to make most apparent the difference between 0.01 and 0.2, to make
     * somewhat apparent the difference between 0.2 and 0.5, and to minimize
     * difference in values from 0.5 to 1.0
     * 
     * designed to be applied to saturation values before they are interpolated
     * with color_lerp
     *
     * expects 0.0 <= value <= 1.0
     */
    static double scale(const double value);


    /* ======== Helper functions: cairo ======== 
     *
     * These are helper functions used by draw_diagram. They can be used
     * elsewhere but are not intended for use elsewhere, and thus their scopes
     * are fairly limited.
     *
     * each function begins with a call to cairo_save and ends with a call to
     * cairo_restore so that settings are preserved across calls
     */

    /* used by pango_cairo_draw_text */
    static void pango_cairo_make_text_layout(
      PangoLayout *layout,
      PangoFontDescription *font_desc,
      std::string text,
      int width,
      PangoAlignment alignment = PangoAlignment::PANGO_ALIGN_LEFT,
      int height = -1);

    /* used by pango_cairo_draw_text */
    static void pango_cairo_draw_layout(
      cairo_t * cr,
      double x,
      double y,
      PangoLayout *layout,
      bool vertical = false);
    
    /* ---- draw text ----
     *
     * Draws text at x, y. x and y should indicate the top-left corner of the
     * text box. Will automatically take as much vertical space as needed, but
     * will limit to the horizonatal space specified, adding new lines as
     * needed to maintain the specified text box width.
     *
     * This function returns the height taken by the text
     * 
     * if "vertical" is true, "text_box_width" will be applied in the vertical
     * cairo dimension. Therefore, vertical distance is fixed and horizontal
     * distance is not. As a result, this function will instead return the
     * horizontal distance taken by the text
     */
    static double pango_cairo_draw_text(
      cairo_t * cr,
      double x,
      double y,
      int text_box_width,
      std::string text,
      PangoFontDescription * font_desc,
      PangoAlignment alignment = PangoAlignment::PANGO_ALIGN_LEFT,
      bool vertical = false);

    /* ---- draw swatch ----
     * Used to create legend on saturation diagram. Also used to test gradients
     * that may be used to indicate saturation in diagrams 
     */
    static void cairo_draw_swatch(
      cairo_t *cr,
      rgb_color min_color, 
      rgb_color max_color,
      unsigned x,
      unsigned y,
      unsigned width,
      unsigned height,
      unsigned num_steps); 

    static void cairo_draw_discrete_swatch(
      cairo_t *cr,
      std::string color_scale_name,
      unsigned x,
      unsigned y,
      unsigned width,
      unsigned height);

    /* ---- draw component ----
     *
     * Font size will be adjusted to be approximately 1/3 the height of the
     * box. Text will be centered horizontally (and vertically in all positions
     * except BOTTOM)
     *
     * Returns the distance needed for the text. This distance is in the
     * vertical dimension for position BOTTOM and horizontal for LEFT. The
     * rectangle will take the rest of <width>
     *
     * In other words, for position LEFT, this function returns the distance
     * from x to the rectangle. For position BOTTOM, this function returns the
     * distance from the bottom of the rectangle to (y+height). For position
     * CENTER, this returns 0.
     */
    static double cairo_draw_component(
      cairo_t *cr,
      double x,
      double y,
      double width,
      double height,
      rgb_color fill_color,
      std::string label,
      PangoFontDescription * font_desc,
      label_position position = label_position::INSIDE,
      double stroke_width = stroke_thickness_normal);

    static void cairo_draw_arrow(
      cairo_t *cr,
      double x,
      double y,
      double width,
      double height,
      rgb_color fill_color,
      direction arrow_direction,
      std::string label,
      PangoFontDescription * font_desc,
      double stroke_width = stroke_thickness_normal);

  private:
};

// COLOR SCALES
const std::string colorScaleName_RdPu = "RdPu";
const std::vector<rgb_color> colorScale_RdPu = {
  rgb_color(255.0/255.0  ,  247.0/255.0  ,  243.0/255.0),
  rgb_color(253.0/255.0  ,  224.0/255.0  ,  221.0/255.0),
  rgb_color(252.0/255.0  ,  197.0/255.0  ,  192.0/255.0),
  rgb_color(250.0/255.0  ,  159.0/255.0  ,  181.0/255.0),
  rgb_color(247.0/255.0  ,  104.0/255.0  ,  161.0/255.0),
  rgb_color(221.0/255.0  ,  52.0/255.0  ,  151.0/255.0),
  rgb_color(174.0/255.0  ,  1.0/255.0  ,  126.0/255.0),
  rgb_color(122.0/255.0  ,  1.0/255.0  ,  119.0/255.0),
  rgb_color(73.0/255.0  ,  0.0/255.0  ,  106.0/255.0),
};

const std::string colorScaleName_YlGnBu = "YlGnBu";
const std::vector<rgb_color> colorScale_YlGnBu = {
  rgb_color(255.0/255.0  ,  255.0/255.0  ,  217.0/255.0),
  rgb_color(237.0/255.0  ,  248.0/255.0  ,  177.0/255.0),
  rgb_color(199.0/255.0  ,  233.0/255.0  ,  180.0/255.0),
  rgb_color(127.0/255.0  ,  205.0/255.0  ,  187.0/255.0),
  rgb_color(65.0/255.0  ,  182.0/255.0  ,  196.0/255.0),
  rgb_color(29.0/255.0  ,  145.0/255.0  ,  192.0/255.0),
  rgb_color(34.0/255.0  ,  94.0/255.0  ,  168.0/255.0),
  rgb_color(37.0/255.0  ,  52.0/255.0  ,  148.0/255.0),
  rgb_color(8.0/255.0  ,  29.0/255.0  ,  88.0/255.0),
};

const std::string colorScaleName_PuBu = "PuBu";
const std::vector<rgb_color> colorScale_PuBu = {
  rgb_color(255.0/255.0  ,  247.0/255.0  ,  251.0/255.0),
  rgb_color(236.0/255.0  ,  231.0/255.0  ,  242.0/255.0),
  rgb_color(208.0/255.0  ,  209.0/255.0  ,  230.0/255.0),
  rgb_color(166.0/255.0  ,  189.0/255.0  ,  219.0/255.0),
  rgb_color(116.0/255.0  ,  169.0/255.0  ,  207.0/255.0),
  rgb_color(54.0/255.0  ,  144.0/255.0  ,  192.0/255.0),
  rgb_color(5.0/255.0  ,  112.0/255.0  ,  176.0/255.0),
  rgb_color(4.0/255.0  ,  90.0/255.0  ,  141.0/255.0),
  rgb_color(2.0/255.0  ,  56.0/255.0  ,  88.0/255.0),
};

const std::string colorScaleName_YlGn = "YlGn";
const std::vector<rgb_color> colorScale_YlGn = {
  rgb_color(255.0/255.0  ,  255.0/255.0  ,  229.0/255.0),
  rgb_color(247.0/255.0  ,  252.0/255.0  ,  185.0/255.0),
  rgb_color(217.0/255.0  ,  240.0/255.0  ,  163.0/255.0),
  rgb_color(173.0/255.0  ,  221.0/255.0  ,  142.0/255.0),
  rgb_color(120.0/255.0  ,  198.0/255.0  ,  121.0/255.0),
  rgb_color(65.0/255.0  ,  171.0/255.0  ,  93.0/255.0),
  rgb_color(35.0/255.0  ,  132.0/255.0  ,  67.0/255.0),
  rgb_color(0.0/255.0  ,  104.0/255.0  ,  55.0/255.0),
  rgb_color(0.0/255.0  ,  69.0/255.0  ,  41.0/255.0),
};

const std::string colorScaleName_Greys = "Greys";
const std::vector<rgb_color> colorScale_Greys = {
  rgb_color(255.0/255.0  ,  255.0/255.0  ,  255.0/255.0),
  rgb_color(240.0/255.0  ,  240.0/255.0  ,  240.0/255.0),
  rgb_color(217.0/255.0  ,  217.0/255.0  ,  217.0/255.0),
  rgb_color(189.0/255.0  ,  189.0/255.0  ,  189.0/255.0),
  rgb_color(150.0/255.0  ,  150.0/255.0  ,  150.0/255.0),
  rgb_color(115.0/255.0  ,  115.0/255.0  ,  115.0/255.0),
  rgb_color(82.0/255.0  ,  82.0/255.0  ,  82.0/255.0),
  rgb_color(37.0/255.0  ,  37.0/255.0  ,  37.0/255.0),
  rgb_color(0.0/255.0  ,  0.0/255.0  ,  0.0/255.0),
};
