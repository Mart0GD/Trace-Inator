#include "scene.hpp"
#include <fstream>
#include <iostream>

#include <rapidjson/istreamwrapper.h>

scene::scene(const std::string& scene_file_name)  
{
    std::ifstream json_stream(scene_file_name);
    ASSERT_OR_THROW(json_stream.is_open());

    rapidjson::IStreamWrapper isw(json_stream);
    rapidjson::Document doc;

    doc.ParseStream(isw);
    ASSERT_OR_THROW(!doc.HasParseError());
    
    parse_from_json(doc);          
    cam.init(settings.width, settings.height); 
}

void scene::parse_from_json(const rapidjson::Value& root)
{
    const auto camera_itt   = root.FindMember(JSON_CAMERA);
    const auto settings_itt = root.FindMember(JSON_SETTINGS);

    ASSERT_OR_THROW(
        camera_itt   != root.MemberEnd() && 
        settings_itt != root.MemberEnd()
    );

    cam.parse_from_json(camera_itt->value);        // Camera initialization
    settings.parse_from_json(settings_itt->value); // Settings initialization

// -- SCENE MEMBERS --

    const auto objects_itt      = root.FindMember(JSON_OBJECTS);
    const auto lights_itt       = root.FindMember(JSON_LIGHTS);
    const auto materials_itt    = root.FindMember(JSON_MATERIALS); 
    const auto textures_itt     = root.FindMember(JSON_TEXTURES);

    ASSERT_OR_THROW(
        objects_itt     != root.MemberEnd() && 
        lights_itt      != root.MemberEnd() &&
        materials_itt   != root.MemberEnd() &&
        textures_itt    != root.MemberEnd() &&
        objects_itt->value.IsArray()        &&
        lights_itt->value.IsArray()         &&
        materials_itt->value.IsArray()
    );

    const size_t materials_cnt = materials_itt->value.Size();
    ASSERT_OR_THROW(materials_cnt != 0);

    this->materials.reserve(materials_cnt);
    for (size_t i = 0; i < materials_cnt; i++)
    {
        material mat;
        mat.parse_from_json(materials_itt->value[i]);
        
        this->materials.push_back(std::move(mat));
    }

    const size_t lights_cnt = lights_itt->value.Size();
    ASSERT_OR_THROW(lights_cnt != 0);   

    this->lights.reserve(lights_cnt);
    for (size_t i = 0; i < lights_cnt; i++)
    {
        light current_light;
        current_light.parse_from_json(lights_itt->value[i]);
        
        this->lights.push_back(std::move(current_light));
    }

    const size_t meshes_cnt = objects_itt->value.Size();
    ASSERT_OR_THROW(meshes_cnt != 0);   

    geometry.reserve(meshes_cnt);
    for (size_t i = 0; i < meshes_cnt; i++)
    {
        mesh current_mesh;
        current_mesh.parse_from_json(objects_itt->value[i]);
        ASSERT_OR_THROW(current_mesh.get_material_id() >= 0 && current_mesh.get_material_id() < materials_cnt);

        geometry.push_back(std::move(current_mesh));
    }
    
    size_t textures_cnt = textures_itt->value.Size();
    ASSERT_OR_THROW(textures_cnt != 0);

    textures.reserve(textures_cnt);
    for (size_t i = 0; i < textures_cnt; i++)
    {
        texture_handle handle;
        handle.parse_from_json(textures_itt->value[i], arena);
        
        this->textures.push_back(std::move(handle));
    }
}

bool scene::trace(const ray& r, double t_min, double t_max, hit_record& rec) const 
{
    hit_record tmp_rec;
    bool hit_anything = false;
    double closest_so_far = t_max;

    for(const mesh& m : geometry)
    {
        if(m.trace(r, t_min, closest_so_far, tmp_rec))
        {
            // temporary logic --> refractive objects don't stop light
            // TODO: add refractive influence on shadowing
            if(r.type == RT_SHADOW && materials[m.get_material_id()].type == MAT_REFRACTIVE) continue;


            hit_anything = true;
            closest_so_far = tmp_rec.t;
            rec = tmp_rec;
        }
    }

    return hit_anything;
}

texture_handle scene::get_texture(const std::string& name) const
{
    for(const texture_handle& txt: textures)
    {
        if(txt.name == name) return txt;
    }
    ASSERT_OR_THROW(false);
}

void scene_settings::parse_from_json(const rapidjson::Value& root) 
{
    ASSERT_OR_THROW(root.IsObject());
    
    auto bg_itt = root.FindMember(JSON_SETTINGS_BG);
    auto image_settings_itt = root.FindMember(JSON_SETTINGS_IMAGE);

    ASSERT_OR_THROW(
        bg_itt != root.MemberEnd()              && bg_itt->value.IsArray()              &&
        image_settings_itt != root.MemberEnd()  && image_settings_itt->value.IsObject()
    );

    const rapidjson::Value& image_settings = image_settings_itt->value;
    
    auto width_itt = image_settings.FindMember(JSON_SETTINGS_IMAGE_WIDTH);
    auto height_itt = image_settings.FindMember(JSON_SETTINGS_IMAGE_HEIGHT);
    
    ASSERT_OR_THROW(
        width_itt  != image_settings.MemberEnd() && 
        height_itt != image_settings.MemberEnd()
    );
    
    this->width      = width_itt->value.GetDouble();
    this->height     = height_itt->value.GetDouble();
    this->background = parse_vector(bg_itt->value.GetArray());
}
