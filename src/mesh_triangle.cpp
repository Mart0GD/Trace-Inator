#include "geometry/mesh_triangle.hpp"

mesh_triangle::mesh_triangle(const mesh& parent, const int32_t v0_i, const int32_t v1_i, const int32_t v2_i)
    : parent(&parent)
    , v0_i(v0_i)
    , v1_i(v1_i)
    , v2_i(v2_i)
{
    const point3D& v0 = parent.verticies[v0_i];
    const point3D& v1 = parent.verticies[v1_i];
    const point3D& v2 = parent.verticies[v2_i];

    center = (v0 + v1 + v2) * 0.333f;
}


const point3D& mesh_triangle::operator [] (const size_t index) const
{
    ASSERT_OR_THROW(index < 3);
    if(index == 0) return parent->verticies[v0_i];
    if(index == 1) return parent->verticies[v1_i];;
    return parent->verticies[v2_i];
}

bool mesh_triangle::intersects(const ray& r, fp t_min, fp t_max, hit_record& info) const 
{
    const double eps = 1e-9;

    const point3D& v0 = parent->verticies[v0_i];
    const point3D& v1 = parent->verticies[v1_i];
    const point3D& v2 = parent->verticies[v2_i];

    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    
    vec3 ray_corss_e2 = cross(r.direction, edge2);
    double det = dot(edge1, ray_corss_e2);

    if (std::abs(det) < eps) return false;

    double inv_det = 1.0 / det;
    vec3 s = r.origin - v0;
    double u = inv_det * dot(s,ray_corss_e2);

    if(u < -eps || u - 1 > eps) return false;       // must be in [0,1]

    vec3 s_cross_e1 = cross(s,edge1);
    double v = inv_det * dot(r.direction, s_cross_e1);

    if(v < -eps || u + v - 1 > eps) return false;   // u + v <= 1

    double t = inv_det * dot(edge2,s_cross_e1);

    if (t <= t_min || t >= t_max) return false;     // outside bounds

    info.t = t;
    info.point = r.at(t);
    info.mat = parent->mat;
    info.baryU = u;
    info.baryV = v;

    const point3D& n0 = parent->v_normals[v0_i];
    const point3D& n1 = parent->v_normals[v1_i];
    const point3D& n2 = parent->v_normals[v2_i];

    const point3D& uv0 = parent->uvs[v0_i];
    const point3D& uv1 = parent->uvs[v1_i];
    const point3D& uv2 = parent->uvs[v2_i];

    vec3 geometric_N = unit_vector(cross(edge1, edge2));
    vec3 smooth_N = unit_vector((1.0 - u - v) * n0 + u * n1 + v * n2);
    point3D interpolatedUV = u * uv1 + v * uv2 + (1 - u - v) * uv0;

    info.geometric_normal =  dot(r.direction, geometric_N) > 0 ? -geometric_N : geometric_N;
    info.hit_normal = dot(r.direction, smooth_N) > 0 ? -smooth_N: smooth_N;
    info.pUV = interpolatedUV;

    return true;
}
