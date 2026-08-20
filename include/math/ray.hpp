#ifndef __RAY_INCLUDED_HPP__
#define __RAY_INCLUDED_HPP__

#include "math/vec3.hpp"
#include "math/point3D.hpp"
#include "math/color.hpp"

enum r_type
{
    RT_INVALID,

    RT_CAMERA,
    RT_SHADOW,
    RT_REFLECTION,
    RT_REFRACTION,
    RT_DIFFUSE,

    RT_COUNT
};

struct ray {

    point3D origin       = point3D(0,0,0);   // <- origin point in 3D world
    vec3    direction    = vec3(0,0,-1);     // <- direction vector
    int32_t depth        = 0;                // <- how many rays came before 
    r_type  type         = RT_CAMERA;        // <- type of the ray for logic checks
    fp  ior              = 1;                // <- ior of the material where the ray comes from
    vec3    inv_dir      = 1 / direction;    // <- for ray AABB intersections

    inline point3D at(double length) const   { return origin + direction * length; }   
};

#endif