#ifndef __MESH_TRIANGLE_HPP_INCLUDED__
#define __MESH_TRIANGLE_HPP_INCLUDED__

#include "geometry/mesh.hpp"
#include "math/ray.hpp"
#include "geometry/hitable.hpp"

class mesh_triangle {

//  ## Non copyable ##

    mesh_triangle(const mesh_triangle&) = delete;
    mesh_triangle& operator=(const mesh_triangle&) = delete; 

    mesh_triangle(mesh_triangle&&) = delete;
    mesh_triangle& operator=(mesh_triangle&&) = delete;

public:

    mesh_triangle() = default;
    mesh_triangle(const mesh& parent, const int32_t v0, const int32_t v1, const int32_t v2);
    bool intersects(const ray& r, fp t_min, fp t_max, hit_record& info) const;

    const point3D& operator [] (const size_t index) const;

    inline const mesh* get_parent() const { return parent; }
    inline const point3D get_center() const { return center; }

private:
    const mesh*   parent = nullptr;
    int32_t       v0_i, v1_i, v2_i;

    point3D center;
};

#endif