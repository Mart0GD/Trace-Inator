#ifndef __COLOR_HPP_INCLUDED__
#define __COLOR_HPP_INCLUDED__

#include "math/vec3.hpp"
#include <iostream>
#include <algorithm>

using color = vec3;

inline void print_color(std::ostream& os, const color& pixel)
{
    // Gamma correction (gamma = 2.0)
    fp r = std::sqrt(pixel.x);
    fp g = std::sqrt(pixel.y);
    fp b = std::sqrt(pixel.z);

    r = std::clamp(r, 0.f, 1.f);
    g = std::clamp(g, 0.f, 1.f);
    b = std::clamp(b, 0.f, 1.f);

    int32_t red_scaled   = static_cast<int32_t>(r * 255.999f);
    int32_t green_scaled = static_cast<int32_t>(g * 255.999f);
    int32_t blue_scaled  = static_cast<int32_t>(b * 255.999f);

    os << red_scaled << ' '
       << green_scaled << ' '
       << blue_scaled << '\n';
}

inline uint32_t color_to_rgba32(const color& pixel) 
{
    color final_color;

    fp r = std::sqrt(pixel.x);
    fp g = std::sqrt(pixel.y);
    fp b = std::sqrt(pixel.z);

    r = std::clamp(r, 0.f, 1.f);
    g = std::clamp(g, 0.f, 1.f);
    b = std::clamp(b, 0.f, 1.f);

    uint8_t ir = static_cast<uint8_t>(255.999f * r);
    uint8_t ig = static_cast<uint8_t>(255.999f * g);
    uint8_t ib = static_cast<uint8_t>(255.999f * b);
    uint8_t ia = 255; // Alpha = 255

    return (ir << 24) | (ig << 16) | (ib << 8) | ia;
}

#endif