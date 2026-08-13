#include "geometry/mesh.hpp"

bool mesh::trace(const ray& r, double t_min, double t_max, hit_record& rec) const
{
    hit_record  tmp_rec;
    bool        hit_anything = false;
    double      closest_so_far = t_max;

    for (size_t i = 0; i < tvi.size(); i+= 3)
    {
        const int32_t v0 = tvi[i];
        const int32_t v1 = tvi[i + 1];
        const int32_t v2 = tvi[i + 2];

        const triangle_context ctx =
        {
            // verticies
            verticies[v0],
            verticies[v1],
            verticies[v2],

            // uv coordinates
            uvs[v0],
            uvs[v1],
            uvs[v2],

            // normals
            v_normals[v0],
            v_normals[v1],
            v_normals[v2],
        };

        if(triangle_hit(
            r, 
            ctx,
            t_min, closest_so_far, tmp_rec
        ))
        {
            rec = tmp_rec;
            hit_anything = true;
            closest_so_far = tmp_rec.t;
        }
    }
    
    return hit_anything;
}

void mesh::parse_from_json(const rapidjson::Value& root)
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
    
    if(uvs_itt != root.MemberEnd())
    {   
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
    }
    
    this->material_id = material_index_itt->value.GetInt();
}

bool mesh::triangle_hit(
    const ray& r,
    const triangle_context& ctx,
    double t_min, double t_max,
    hit_record& rec
) const
{   
    const double eps = 1e-9;

    vec3 edge1 = ctx.v1 - ctx.v0;
    vec3 edge2 = ctx.v2 - ctx.v0;
    
    vec3 ray_corss_e2 = cross(r.direction, edge2);
    double det = dot(edge1, ray_corss_e2);

    if (std::abs(det) < eps) return false;

    double inv_det = 1.0 / det;
    vec3 s = r.origin - ctx.v0;
    double u = inv_det * dot(s,ray_corss_e2);

    if(u < -eps || u - 1 > eps) return false;       // must be in [0,1]

    vec3 s_cross_e1 = cross(s,edge1);
    double v = inv_det * dot(r.direction, s_cross_e1);

    if(v < -eps || u + v - 1 > eps) return false;   // u + v <= 1

    double t = inv_det * dot(edge2,s_cross_e1);

    if (t <= t_min || t >= t_max) return false;     // outside bounds

    rec.t = t;
    rec.point = r.at(t);
    rec.material_id = material_id;
    rec.baryU = u;
    rec.baryV = v;

    vec3 geometric_N = unit_vector(cross(edge1, edge2));
    vec3 smooth_N = unit_vector((1.0 - u - v) * ctx.n0 + u * ctx.n1 + v * ctx.n2);
    point3D interpolatedUV = u * ctx.uv1 + v * ctx.uv2 + (1 - u - v) * ctx.uv0;

    rec.geometric_normal = geometric_N;
    rec.hit_normal = smooth_N;
    rec.pUV = interpolatedUV;

    return true;
}