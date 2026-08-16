#ifndef __MESH_HPP_INCLUDED__
#define __MESH_HPP_INCLUDED__

#include "utils/constants.hpp"
#include "hitable.hpp"
#include "geometry/hitable.hpp"
#include "material.hpp"

#include <vector>

// Forward declaration
struct parse_ctx;

struct mesh
{
    void parse_from_json(const rapidjson::Value& info, const parse_ctx& ctx);
    
    inline size_t triangles_cnt() const { return tvi.size() / 3; }

    std::vector<point3D>    verticies;  // all verticies for the mesh
    std::vector<point3D>    uvs;        // uv coordinates data
    std::vector<vec3>       v_normals;  // vertex normals
    std::vector<int32_t>    tvi;        // triangle vertex indicies 
    const material         *mat = nullptr;
};

#endif