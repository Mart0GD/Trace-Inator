#include "geometry/aabb.hpp"
#include "scene.hpp"

aabb::aabb()
{
    double max = std::numeric_limits<double>::max();
    double min = std::numeric_limits<double>::min();
    
    p_min = point3D(max,max,max);
    p_max = point3D(min,min,min);
}

bool aabb::is_empty() const 
{
    return p_min.x <= p_max.x || p_min.y <= p_max.y || p_min.z <= p_max.z;
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

bool intersects(const ray& r, const aabb& box)
{
    // Formula --> t = (point - r.origin) / r.dir
    struct packet
    {
        double box_side;
        double origin_side;
        double direction_side;
    };

    // Packets for all calculations
    const packet sides[6]
    {
        {box.p_min.x, r.origin.x, r.direction.x},
        {box.p_min.y, r.origin.y, r.direction.y},
        {box.p_min.z, r.origin.z, r.direction.z},

        {box.p_max.x, r.origin.x, r.direction.x},
        {box.p_max.y, r.origin.y, r.direction.y},
        {box.p_max.z, r.origin.z, r.direction.z},
    };

    bool intersection = false;
    double min_t = std::numeric_limits<double>::max();

    for(const packet& p: sides)
    {
        if(p.direction_side > -1e-9 && p.direction_side < 1e-9) continue;

        const double t = (p.box_side - p.origin_side) / p.direction_side;
        if(t < 0) continue; // behind

        const point3D point = r.at(t);
        if(inside(point, box) && t < min_t)
        {
            intersection = true;
            min_t = t;
        }
    }
    
    return intersection;
}

void aabb::grow_to_include(const point3D& point)
{
    this->p_min = min(this->p_min, point);
    this->p_max = max(this->p_max, point);
}

void aabb::grow_to_include(const mesh& m)
{
    const std::vector<point3D>& verticies = m.get_verticies();

    for(const point3D& p: verticies)
    {
        grow_to_include(p);
    }
}