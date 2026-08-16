#ifndef __COLOR_HPP_INCLUDED__
#define __COLOR_HPP_INCLUDED__

#include "math/vec3.hpp"
#include <iostream>

using color = vec3;

inline void print_color(std::ostream& os, const color& pixel)
{
    color final_color;

    // Reinhard tone mapping --> saw it on youtube 
    final_color.x = pixel.x / (pixel.x + 1.0);
    final_color.y = pixel.y / (pixel.y + 1.0);
    final_color.z = pixel.z / (pixel.z + 1.0);

    int red_scaled   = (int)(final_color.x * 255.999);  // x --> red
    int green_scaled = (int)(final_color.y * 255.999);  // y --> green
    int blue_scaled  = (int)(final_color.z * 255.999);  // z --> blue

    os << red_scaled << ' ' << green_scaled << ' ' << blue_scaled << '\n';
}

#endif