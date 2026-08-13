#ifndef __MESH_HPP_INCLUDED__
#define __MESH_HPP_INCLUDED__

#include "utils/constants.hpp"
#include "hitable.hpp"
#include "geometry/hitable.hpp"

#include <vector>

struct triangle_context 
{
    const point3D& v0, v1, v2;
    const point3D& uv0, uv1, uv2;
    const vec3& n0, n1, n2;
};

class mesh{
public:
    void parse_from_json(const rapidjson::Value& info);
    bool trace(const ray& r, double t_min, double t_max, hit_record& rec) const;

    inline int32_t get_material_id() const { return material_id; }

private:

    // Basic triangle intersection algorithm
    bool triangle_hit(
        const ray& r,
        const triangle_context& ctx,
        double t_min, double t_max,
        hit_record& rec
    ) const;

private:
    std::vector<point3D>    verticies;  // all verticies for the mesh
    std::vector<point3D>    uvs;        // uv coordinates data
    std::vector<vec3>       v_normals;  // vertex normals
    std::vector<int>        tvi;        // triangle vertex indicies 
    int32_t                 material_id;
};

#endif