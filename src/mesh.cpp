#include "geometry/mesh.hpp"
#include "scene.hpp"

void mesh::parse_from_json(const rapidjson::Value& root, const parse_ctx& ctx)
{
    ASSERT_OR_THROW(root.IsObject());

    const auto verticies_itt        = root.FindMember(JSON_OBJECTS_VERTICES);
    const auto triangles_itt        = root.FindMember(JSON_OBJECTS_TRIANGLES);
    const auto material_index_itt   = root.FindMember(JSON_OBJECTS_MATERIAL_INDEX);
    const auto uvs_itt              = root.FindMember(JSON_OBJECTS_UVS);

    ASSERT_OR_THROW(
        triangles_itt != root.MemberEnd()       && triangles_itt->value.IsArray()   &&
        verticies_itt != root.MemberEnd()       && verticies_itt->value.IsArray()   &&
        material_index_itt != root.MemberEnd()  && material_index_itt->value.IsInt()
    );

    const rapidjson::Value& verticies = verticies_itt->value;
    const rapidjson::Value& triangles = triangles_itt->value;

    const size_t v_size     = verticies.Size();
    const size_t t_size     = triangles.Size();

    ASSERT_OR_THROW(
        v_size > 0   && v_size % 3 == 0  &&
        t_size > 0   && t_size % 3 == 0 
    );

    this->verticies.reserve(v_size / 3);
    for (size_t i = 0; i < v_size; i += 3)
    {
        this->verticies.push_back(point3D(
            verticies[i]    .GetDouble(),
            verticies[i + 1].GetDouble(),
            verticies[i + 2].GetDouble()
        ));
    }
    
    tvi.reserve(t_size);
    v_normals.resize(v_size / 3, vec3(0,0,0));

    for (size_t i = 0; i < t_size; i += 3) 
    {
        const int32_t p1 = triangles[i]    .GetInt();
        const int32_t p2 = triangles[i + 1].GetInt();
        const int32_t p3 = triangles[i + 2].GetInt();

        const vec3 AB = this->verticies[p2] - this->verticies[p1];
        const vec3 AC = this->verticies[p3] - this->verticies[p1];

        const vec3 norm = unit_vector(cross(AB,AC));

        this->v_normals[p1] += norm;
        this->v_normals[p2] += norm;
        this->v_normals[p3] += norm;

        this->tvi.push_back(p1);
        this->tvi.push_back(p2);
        this->tvi.push_back(p3);
    }

    for(vec3& vtx: this->v_normals) vtx = unit_vector(vtx);
    this->mat = &ctx.materials[material_index_itt->value.GetInt()];
    
    switch (ctx.version)
    {
        case 2:
        {
            ASSERT_OR_THROW(uvs_itt != root.MemberEnd());

            const rapidjson::Value& uvs = uvs_itt->value;
            const size_t uvs_size = uvs.Size();   

            this->uvs.reserve(uvs_size / 3);
            for (size_t i = 0; i < uvs_size; i+=3)
            {
                this->uvs.push_back(point3D(
                    uvs[i].GetDouble(),
                    uvs[i + 1].GetDouble(),
                    uvs[i + 2].GetDouble()
                ));
            }
        } break;
        case 1:
        {
            // one pixel for the albedo effect
            this->uvs = std::vector<point3D>(v_size / 3, {0,0,0});
        } break;
    }
}