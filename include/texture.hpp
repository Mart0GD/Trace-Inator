#ifndef __TEXTURE_HPP_INCLUDED__
#define __TEXTURE_HPP_INCLUDED__

#include "utils/constants.hpp"
#include "memory/tagged_pointer.hpp"
#include "geometry/hitable.hpp"
#include "memory/allocator.hpp"

#include <unordered_map>

// Forward declaration
struct parse_ctx;

struct albedo_texture
{
    color albedo;

    void parse_from_json(const rapidjson::Value& info);
};

struct edges_texture
{
    vec3    edge_color;
    vec3    inner_color;
    double  edge_width;

    void parse_from_json(const rapidjson::Value& info);
};

struct checker_texture
{
    vec3 color_A;
    vec3 color_B;
    double square_size;

    void parse_from_json(const rapidjson::Value& info);
};

struct bitmap_texture
{
    int width, height, channels;
    unsigned char* buffer = nullptr;

    void parse_from_json(const rapidjson::Value& info);

    ~bitmap_texture() noexcept;
};

using Texture = tagged_pointer
<
    albedo_texture, 
    edges_texture, 
    checker_texture, 
    bitmap_texture
>;

enum texture_type
{
    TT_NULLPTR,

    TT_ALBEDO,
    TT_EDGES,
    TT_CHECKER,
    TT_BITMAP,

    TT_COUNT
};

inline static const std::unordered_map<std::string, texture_type> texture_types = {
    {"albedo", TT_ALBEDO},
    {"edges", TT_EDGES},
    {"checker", TT_CHECKER},
    {"bitmap", TT_BITMAP}
};

struct texture_handle
{
    Texture ptr;
    std::string name;

    color evaluate(const hit_record& info) const;
    void parse_from_json(const rapidjson::Value& info, const parse_ctx& ctx);
};

#endif