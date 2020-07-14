#pragma once

#include <cairo.h>
#include <cairo-svg.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <map>
#include <pango/pangocairo.h>
#include <tuple>
#include <string>

#include "architecture.h"

using json = nlohmann::json;

// ----- simple color type ----- //
typedef std::tuple<double, double, double> rgb_color;

enum text_alignment { left, center, right };

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

    /* ---- calculate saturation colors ----- 
     * The return value of this function is intended to be passed to
     * draw_diagram. 
     */ 
    static std::map<std::string, rgb_color>
    calculate_saturation_colors(
      json region_saturation,
      rgb_color min_color,
      rgb_color max_color);

    /* ---- draw diagram ----
     * the function that actually makes the diagram 
     */
    static void draw_diagram(
      std::map<std::string, rgb_color> region_colors,
      json region_data,
      rgb_color min_color,
      rgb_color max_color,
      std::string region_name,
      std::string parameters,
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
      rgb_color min_color, 
      rgb_color max_color, 
      double t);

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
    static double scale(double value);


    /* ======== Helper functions: cairo ======== 
     * These are helper functions used by draw_diagram. They can be used
     * elsewhere but are not intended for use elsewhere, and thus their scopes
     * are fairly limited.
     *
     * each function begins with a call to cairo_save and ends with a call to
     * cairo_restore so that settings are preserved across calls
     */
    
    /* ---- draw text ----
     * x, y coordinates should be to top-left corner of text box. 
     * 
     * TODO: rename this and sideways to pango_cairo...
     */
    static void cairo_draw_text(
      cairo_t * cr,
      double x, 
      double y,
      double text_box_width,
      std::string text,
      PangoFontDescription * font_desc,
      text_alignment alignment);

    /* ---- draw sideways text ----
     * x, y coordinates should indicate bottom left corner of textbox that will
     * contain the text. Text will be vertically aligned to the center and the
     * width of the box will be determined by the size of the text. This
     * function moves the cursor to the bottom-right corner of the text box
     * before returning.
     *
     * TODO: handle wrapping text, support more than one alignment
     */
    static void cairo_draw_sideways_text(
      cairo_t * cr, 
      double x,
      double y,
      double text_box_height,
      std::string text,
      PangoFontDescription * font_desc);

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

  private:
};
