#ifndef __HITABLE_HPP_INCLUDED__
#define __HITABLE_HPP_INCLUDED__

#include "utils/constants.hpp"

// Forward declaration
struct material;

struct hit_record
{
    point3D point;                              // <- 3D point of where the ray hits the geometry
    vec3    hit_normal;                         // <- interpolated vertex normal
    vec3    geometric_normal;                   // <- the face normal of the geometry
    
    double  t;                                  // <- distance in time from the origin
    const material* mat;                        // <- material in the scene (no copies in mesh)
    double  baryU,baryV;                        // <- barycentric coordinates
    point3D pUV;                                // <- interpolated u,v coordinates
};

class hitable{
public:
    virtual ~hitable() noexcept = default;

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};

#endif