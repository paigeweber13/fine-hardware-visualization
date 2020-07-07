#pragma once

#include <cairo.h>
#include <cairo-svg.h>
#include <map>
#include <tuple>

// ----- simple color type ----- //
typedef std::tuple<double, double, double> rgb_color;

class saturation_diagram {
  public:

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
    rgb_color color_lerp( rgb_color min_color, rgb_color max_color, double t);

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

    /* ---- test color lerp ----
     * Draws a swatch of a gradient calculated using color_lerp to test before
     * using in a saturation diagram
     */
    static void test_color_lerp(
      rgb_color min_color, 
      rgb_color max_color,
      unsigned width,
      unsigned height,
      unsigned num_steps)
    {

    /* ---- calculate saturation colors ----- */ 
    static std::map<std::string, rgb_color>
    calculate_saturation_colors(
      json region_saturation,
      rgb_color min_color,
      rgb_color max_color);

  private:
}
