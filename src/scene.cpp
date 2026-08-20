#include "scene.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
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
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    acc_tree.build(*this);

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    

    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    const double seconds = duration.count() / 1'000'000.0;
    std::cerr << "time for creating bvh: " << seconds << " seconds." << std::endl;
}

void scene::parse_from_json(const rapidjson::Value& root)
{
    const auto version_itt = root.FindMember(JSON_VERSION);
    ASSERT_OR_THROW(version_itt != root.MemberEnd());

    size_t version = version_itt->value.GetInt(); 

    parse_ctx ctx = 
    {
        this->arena,
        version,
        this->textures,
        this->materials,
    };

    const auto camera_itt   = root.FindMember(JSON_CAMERA);
    const auto settings_itt = root.FindMember(JSON_SETTINGS);

    ASSERT_OR_THROW(
        camera_itt   != root.MemberEnd() && 
        settings_itt != root.MemberEnd()
    );

    cam.parse_from_json(camera_itt->value, ctx);        // Camera initialization
    settings.parse_from_json(settings_itt->value, ctx); // Settings initialization

// -- SCENE MEMBERS --

    const auto objects_itt      = root.FindMember(JSON_OBJECTS);
    const auto lights_itt       = root.FindMember(JSON_LIGHTS);
    const auto materials_itt    = root.FindMember(JSON_MATERIALS); 
    const auto textures_itt     = root.FindMember(JSON_TEXTURES);

    switch (version)
    {
        case 2:
        {
            size_t textures_cnt = textures_itt->value.Size();
            ASSERT_OR_THROW(textures_cnt != 0);

            textures.reserve(textures_cnt);
            for (size_t i = 0; i < textures_cnt; i++)
            {
                texture_handle handle;
                handle.parse_from_json(textures_itt->value[i], ctx);
                
                this->textures.push_back(std::move(handle));
            }

        };
        case 1:
        {
            if(materials_itt != root.MemberEnd())
            {
                const size_t materials_cnt = materials_itt->value.Size();

                this->materials.reserve(materials_cnt);
                for (size_t i = 0; i < materials_cnt; i++)
                {
                    material mat;
                    mat.parse_from_json(materials_itt->value[i], ctx);
                    
                    this->materials.push_back(std::move(mat));
                }
            }

            if(objects_itt != root.MemberEnd())
            {
                const size_t meshes_cnt = objects_itt->value.Size();

                geometry.reserve(meshes_cnt);
                for (size_t i = 0; i < meshes_cnt; i++)
                {
                    mesh current_mesh;
                    current_mesh.parse_from_json(objects_itt->value[i], ctx);

                    geometry.push_back(std::move(current_mesh));
                }
            }

            if (lights_itt != root.MemberEnd())
            {
                const size_t lights_cnt = lights_itt->value.Size();

                this->lights.reserve(lights_cnt);
                for (size_t i = 0; i < lights_cnt; i++)
                {
                    light current_light;
                    current_light.parse_from_json(lights_itt->value[i], ctx);
                    
                    this->lights.push_back(std::move(current_light));
                }
            }

        } break;
    }
}

bool scene::trace(const ray& r, fp t_min, fp t_max, hit_record& rec) const 
{
    hit_record tmp_rec;
    bool hit_anything = false;
    fp closest_so_far = t_max;

    for(const mesh& m : geometry)
    {
        // if(m.trace(r, t_min, closest_so_far, tmp_rec))
        // {
        //     // temporary logic --> refractive objects don't stop light
        //     // TODO: add refractive influence on shadowing
        //     if(r.type == RT_SHADOW && m.mat->type == MAT_REFRACTIVE) continue;


        //     hit_anything = true;
        //     closest_so_far = tmp_rec.t;
        //     rec = tmp_rec;
        // }
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

size_t scene::triangles_cnt() const
{
    size_t triangles = 0;
    for(const mesh& m: geometry)
    {
        triangles += m.triangles_cnt();
    }

    return triangles;
}   

void scene_settings::parse_from_json(const rapidjson::Value& root, const parse_ctx& ctx) 
{
    ASSERT_OR_THROW(root.IsObject());
    
    switch (ctx.version)
    {
        case 3:
        {
            const auto bucket_size_itt = root.FindMember(JSON_SETTINGS_BUCKET_SIZE);

            if(bucket_size_itt != root.MemberEnd()) this->bucket_size = bucket_size_itt->value.GetInt();
        }
        case 1: case 2:
        {
            const auto bg_itt = root.FindMember(JSON_SETTINGS_BG);
            const auto image_settings_itt = root.FindMember(JSON_SETTINGS_IMAGE);

            // Has BG
            if(bg_itt != root.MemberEnd()) 
            {
                ASSERT_OR_THROW(bg_itt->value.IsArray());
                this->background = parse_vector(bg_itt->value.GetArray());
            }

            // Has settings (may be empty)
            if(image_settings_itt != root.MemberEnd())
            {
                const rapidjson::Value& image_settings = image_settings_itt->value;
                ASSERT_OR_THROW(image_settings.IsObject());

                const auto width_itt = image_settings.FindMember(JSON_SETTINGS_IMAGE_WIDTH);
                const auto height_itt = image_settings.FindMember(JSON_SETTINGS_IMAGE_HEIGHT);

                if(width_itt != image_settings.MemberEnd())  this->width = width_itt->value.GetDouble();
                if(height_itt != image_settings.MemberEnd()) this->height = height_itt->value.GetDouble();
            }
        } break;
    }

}
