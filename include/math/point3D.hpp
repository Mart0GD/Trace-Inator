#ifndef __POINT3D_HPP_INCLUDED__
#define __POINT3D_HPP_INCLUDED__

#include "math/vec3.hpp"

using point3D = vec3;

inline fp dist(const point3D& p1, const point3D& p2)
{
    return sqrt
    (
        (p2.x - p1.x) * (p2.x - p1.x) + 
        (p2.y - p1.y) * (p2.y - p1.y) +
        (p2.z - p1.z) * (p2.z - p1.z) 
    );
}

inline point3D min(const point3D& p1, const point3D& p2)
{
    return point3D(std::min(p1.x,p2.x), std::min(p1.y,p2.y), std::min(p1.z,p2.z));
}

inline point3D max(const point3D& p1, const point3D& p2)
{
    return point3D(std::max(p1.x,p2.x), std::max(p1.y,p2.y), std::max(p1.z,p2.z));
}


#endif