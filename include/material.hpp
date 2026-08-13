#ifndef __MATERIAL_HPP_INCLUDED__
#define __MATERIAL_HPP_INCLUDED__

#include "utils/constants.hpp"
#include "texture.hpp"
#include "geometry/hitable.hpp"

#include <unordered_map>

// Forward declaration
struct parse_ctx;

enum material_type
{
    MAT_UNKNOWN = -1,

    MAT_DIFFUSIVE,
    MAT_REFLECTIVE,
    MAT_REFRACTIVE,
    MAT_CONSTANT,

    MAT_COUNT
};

inline static const std::unordered_map<std::string, material_type> m_types = {
    {"diffuse", MAT_DIFFUSIVE},
    {"reflective", MAT_REFLECTIVE},
    {"refractive", MAT_REFRACTIVE},
    {"constant", MAT_CONSTANT}
};

struct material
{
    material_type  type      = MAT_CONSTANT;  // material type
    texture_handle albedo;
    bool smooth_shading      = false;         // shade with vertex normals
    double ior               = 1;             // index of refraction

    void parse_from_json(const rapidjson::Value& root, const parse_ctx& ctx);
};

#endif
