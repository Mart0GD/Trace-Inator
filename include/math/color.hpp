#ifndef __COLOR_HPP_INCLUDED__
#define __COLOR_HPP_INCLUDED__

#include "math/vec3.hpp"
#include <iostream>
#include <algorithm>
using color = vec3;

inline void print_color(std::ostream& os, const color& pixel)
{
    // Reinhard tone mapping
    float r = pixel.x / (1.f + pixel.x);
    float g = pixel.y / (1.f + pixel.y);
    float b = pixel.z / (1.f + pixel.z);

    // Gamma correction (gamma = 2.0)
    r = std::sqrt(r);
    g = std::sqrt(g);
    b = std::sqrt(b);

    r = std::clamp(r, 0.f, 1.f);
    g = std::clamp(g, 0.f, 1.f);
    b = std::clamp(b, 0.f, 1.f);

    int red_scaled   = static_cast<int>(r * 255.999f);
    int green_scaled = static_cast<int>(g * 255.999f);
    int blue_scaled  = static_cast<int>(b * 255.999f);

    os << red_scaled << ' '
       << green_scaled << ' '
       << blue_scaled << '\n';
}

inline uint32_t color_to_rgba32(color pixel) 
{
    color final_color;

    final_color.x = pixel.x / (pixel.x + 1.0);
    final_color.y = pixel.y / (pixel.y + 1.0);
    final_color.z = pixel.z / (pixel.z + 1.0);

    uint8_t ir = static_cast<uint8_t>(255.999f * final_color.x);
    uint8_t ig = static_cast<uint8_t>(255.999f * final_color.y);
    uint8_t ib = static_cast<uint8_t>(255.999f * final_color.z);
    uint8_t ia = 255; // Alpha = 255

    return (ir << 24) | (ig << 16) | (ib << 8) | ia;
}

#endif