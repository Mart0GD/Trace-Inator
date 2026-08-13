#ifndef __RENDERER_HPP_INCLUDED__
#define __RENDERER_HPP_INCLUDED__

#include "utils/constants.hpp"
#include "scene.hpp"
#include "texture.hpp"

#include <random>
#include <stack>

class renderer {
public:
    void render(std::ostream& os = std::cout);

    scene* world;

private:

    color shade(const ray& r) const;  
    
    color shade_diffusive(const ray& r, const hit_record& info) const; 
    color shade_reflective(const ray& r, const hit_record& info) const; 
    color shade_refractive(const ray& r, const hit_record& info) const; 

    static double schlick(double n1, double n2, double cos_a); 
    static double chaos(double cos_a);

    static constexpr double  SHADOW_BIAS        = 0.01;
    static constexpr double  REFRACTION_BIAS    = 0.01;
    static constexpr double  REFLECTION_BIAS    = 0.01;

    static constexpr size_t  MAX_DEPTH          = 3;  
};

#endif