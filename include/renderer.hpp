#ifndef __RENDERER_HPP_INCLUDED__
#define __RENDERER_HPP_INCLUDED__

#include "utils/constants.hpp"
#include "memory/thread_pool.hpp"
#include "geometry/aabb.hpp"

#include "scene.hpp"
#include "texture.hpp"

#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "SDL2/SDL.h"

struct bucket
{
    int32_t min_x, max_x;
    int32_t min_y, max_y;

    inline int32_t width() const { return max_x - min_x; }
    inline int32_t height() const { return max_y - min_y; }
};

struct light_response
{
    fp reflection_weight;
    fp refraction_weight;
};

struct animation_info
{
    std::vector<point3D> path;  // points to interpolate between
    point3D target;             // point to look at
    int32_t frames;             // interpolation time
};

class renderer {
public:
    void render(int32_t debug_depth = -1);
    void render_sdl();

    void save(const std::string& name = "image.ppm");
    void animate(const animation_info& path);

    scene* world;

private:

    color shade(const ray& r, std::mt19937& rng) const;  
    color shade_debug(const ray& r, int debug_depth) const;
    
    color shade_diffusive(const ray& r, const hit_record& info, std::mt19937& rng) const; 
    color shade_reflective(const ray& r, const hit_record& info, std::mt19937& rng) const; 
    color shade_refractive(const ray& r, const hit_record& info, std::mt19937& rng) const; 

    vec3  reflect(const vec3 direction, const vec3& normal) const;
    vec3  refract(const vec3& dir, const vec3& normal, fp n1, fp n2) const;
    light_response calculate_fresnel_equasion(const vec3& dir, const vec3& normal, fp n1, fp n2) const;

    fp schlick_appx(fp n1, fp n2, fp cos_a) const; 
    color get_background(const ray& r) const;

    point3D position_at(const animation_info& info, fp t);

    // Generate an interpolated point between two points based on a spline between 4 
    static point3D catmull_rom(
        const point3D& p0, 
        const point3D& p1, 
        const point3D& p2, 
        const point3D& p3,
        fp t
    );

private:

    thread_pool pool;
    std::vector<color> image_buffer;

    static constexpr double  BIAS        = 0.01;
    static constexpr size_t  MAX_DEPTH   = 15;  
};

#endif