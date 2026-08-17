#ifndef __AABB_HPP_INCLUDED__
#define __AABB_HPP_INCLUDED__

#include "utils/constants.hpp"
#include "geometry/mesh.hpp"
#include "geometry/mesh_triangle.hpp"


// Class representing an Axis Aligned Bounding Box.
struct aabb {
    
    point3D p_min, p_max;

    // Empty box
    aabb();

    // aabb over a point
    explicit aabb(const point3D& p) : p_min(p), p_max(p) {};

    // aabb with two points
    aabb(const point3D& p1, const point3D& p2) : p_min(min(p1,p2)), p_max(max(p1,p2)) {};

    void grow_to_include(const point3D& point);
    void grow_to_include(const mesh_triangle& triangle);
    void grow_to_include(const mesh& m);

    bool is_empty() const; 
    fp   area()     const;
};

// Encompass a new point with the given aabb
aabb _union(const aabb& box, const point3D& point);

// Encompass two bounding boxes 
aabb _union(const aabb& box1, const aabb& box2);

// Cheks if a point is inside a bounding box
bool inside(const point3D& p, const aabb& box);

fp intersects(const ray& r, const aabb& box, fp ray_t);

#endif