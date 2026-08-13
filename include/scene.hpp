#ifndef __SCENE_HPP_INCLUDED__
#define __SCENE_HPP_INCLUDED__

#include "utils/constants.hpp"
#include "geometry/mesh.hpp"
#include "memory/allocator.hpp"

#include "light.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "texture.hpp"

#include <vector>
#include <unordered_map>

struct parse_ctx
{
    allocator& al;
    const size_t version;

    const std::vector<texture_handle>&  textures;
    const std::vector<material>&        materials;
};

struct scene_settings {

    double width         = 800;
    double height        = 400;
    color  background    = {0,0,0};
    
    void parse_from_json(const rapidjson::Value& info, const parse_ctx& version);
};

struct scene {
public:

    scene(const std::string& scene_file_name);
    void parse_from_json(const rapidjson::Value& info);
    
    bool trace(const ray& r, double t_min, double t_max, hit_record& rec) const;

    texture_handle get_texture(const std::string& name) const;
    
// -- SCENE SPECIFIC --
    std::vector<mesh>           geometry;
    std::vector<light>          lights;
    std::vector<material>       materials;
    std::vector<texture_handle> textures;

// -- OTHERS --
    scene_settings          settings;
    allocator               arena;
    camera                  cam;
};

#endif