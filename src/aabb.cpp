#include "geometry/aabb.hpp"
#include "scene.hpp"

aabb::aabb()
{
    const fp max = 1e30;
    const fp min = -1e30;
    
    p_min = point3D(max,max,max);
    p_max = point3D(min,min,min);
}

bool aabb::is_empty() const 
{
    return p_min.x > p_max.x || p_min.y > p_max.y || p_min.z > p_max.z;
}

aabb _union(const aabb& box, const point3D& point)
{
    aabb ret;
    ret.p_min = min(box.p_min, point);
    ret.p_max = max(box.p_max, point);

    return ret; // RVO
}

aabb _union(const aabb& box1, const aabb& box2)
{
    aabb ret;
    ret.p_min = min(box1.p_min, box2.p_min);
    ret.p_max = max(box1.p_max, box2.p_max);

    return ret; // RVO
}

bool inside(const point3D& p, const aabb& box)
{
    return 
    p.x >= box.p_min.x && p.x <= box.p_max.x &&
    p.y >= box.p_min.y && p.y <= box.p_max.y &&
    p.z >= box.p_min.z && p.z <= box.p_max.z;
}
fp intersects(const ray& r, const aabb& box, fp ray_t)
{   
    fp tx1 = (box.p_min.x - r.origin.x) * r.inv_dir.x;
    fp tx2 = (box.p_max.x - r.origin.x) * r.inv_dir.x;

    fp tmin = std::min(tx1, tx2);
    fp tmax = std::max(tx1, tx2);

    // Y 
    fp ty1 = (box.p_min.y - r.origin.y) * r.inv_dir.y;
    fp ty2 = (box.p_max.y - r.origin.y) * r.inv_dir.y;

    tmin = std::max(tmin, std::min(ty1, ty2));
    tmax = std::min(tmax, std::max(ty1, ty2));

    // Z 
    fp tz1 = (box.p_min.z - r.origin.z) * r.inv_dir.z;
    fp tz2 = (box.p_max.z - r.origin.z) * r.inv_dir.z;

    tmin = std::max(tmin, std::min(tz1, tz2));
    tmax = std::min(tmax, std::max(tz1, tz2));

    bool valid = tmax >= tmin && tmin < ray_t && tmax > 0.0f;
    return valid * tmin + !valid * 1e30f;
}

void aabb::grow_to_include(const point3D& point)
{
    this->p_min = min(this->p_min, point);
    this->p_max = max(this->p_max, point);
}


void aabb::grow_to_include(const mesh_triangle& triangle)
{
    grow_to_include(triangle[0]);
    grow_to_include(triangle[1]);
    grow_to_include(triangle[2]);
}

void aabb::grow_to_include(const mesh& m)
{
    const std::vector<point3D>& verticies = m.verticies;

    for(const point3D& p: verticies)
    {
        grow_to_include(p);
    }
}


void aabb::grow_to_include(const aabb& box)
{
    this->p_min = min(this->p_min, box.p_min);
    this->p_max = max(this->p_max, box.p_max);
}

fp aabb::area() const
{
    vec3 diagonal = p_max - p_min;
    return diagonal.x * diagonal.y + diagonal.y * diagonal.z + diagonal.z * diagonal.x;
}