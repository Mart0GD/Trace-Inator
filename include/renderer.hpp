#ifndef __RENDERER_HPP_INCLUDED__
#define __RENDERER_HPP_INCLUDED__

#include "utils/constants.hpp"
#include "memory/thread_pool.hpp"
#include "geometry/aabb.hpp"

#include "scene.hpp"
#include "texture.hpp"

#include <mutex>
#include <condition_variable>
#include <atomic>

struct bucket
{
    int32_t min_x, max_x;
    int32_t min_y, max_y;
};

class renderer {
public:
    void render(std::ostream& os = std::cout);

    scene* world;

private:

    color shade(const ray& r) const;  
    
    color shade_diffusive(const ray& r, const hit_record& info) const; 
    color shade_reflective(const ray& r, const hit_record& info) const; 
    color shade_refractive(const ray& r, const hit_record& info) const; 

    static fp schlick(fp n1, fp n2, fp cos_a); 
    static fp chaos(fp cos_a);

private:

    thread_pool pool;

    static constexpr double  SHADOW_BIAS        = 0.01;
    static constexpr double  REFRACTION_BIAS    = 0.01;
    static constexpr double  REFLECTION_BIAS    = 0.01;

    static constexpr size_t  MAX_DEPTH          = 3;  
};

#endif