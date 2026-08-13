#include "renderer.hpp"

void renderer::render(std::ostream& os) {
    const camera& camera = world->cam;
    const scene_settings& settings  = world->settings;

    double width = settings.width;
    double height = settings.height;

    os << "P3\n" << width << ' ' << height << "\n255\n";

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            ray r = camera.get_ray(j, i);

            color c = shade(r);
            print_color(os, c);
        }
    }
}

color renderer::shade(const ray& r) const 
{
    hit_record info;
    bool hit = world->trace(r, 0.001, INF, info);

    if(hit == false || r.depth >= MAX_DEPTH)
    {
        // Return background
        vec3 unit_direction = unit_vector(r.direction);
        auto a = 0.5*(unit_direction.y + 1.0);
        return (1.0-a)*color(1.0, 1.0, 1.0) + a*world->settings.background;
    }

#ifndef _DEBUG

    color final_color;
    const material* mat = info.mat;

    switch (mat->type)
    {
        case MAT_CONSTANT: 
        {
            final_color = mat->albedo.evaluate(info);
        } break;

        case MAT_DIFFUSIVE:
        {
            final_color = shade_diffusive(r, info);
        } break;

        case MAT_REFLECTIVE:
        {
            final_color = shade_reflective(r, info);
        } break;
        
        case MAT_REFRACTIVE:
        {
            final_color = shade_refractive(r, info);
        } break;

        default: ASSERT_OR_THROW(true);
    }

    return final_color;

#else

    vec3 N = info.geometric_normal;
    return 0.5*color(N.x +1 , N.y + 1, N.z + 1);
#endif
}

color renderer::shade_diffusive(const ray& r, const hit_record& info) const
{
    const material* mat = info.mat;

    const size_t lights_cnt = world->lights.size();
    const color  albedo = mat->albedo.evaluate(info);
    const vec3   normal = mat->smooth_shading ? info.hit_normal : info.geometric_normal;

    color final_color;

    for (size_t i = 0; i < lights_cnt; i++)
    {
        vec3    ld = world->lights[i].position - r.at(info.t);
        const double  sr = ld.length();
        ld = unit_vector(ld);

        const double cos = std::max(0.0, dot(ld, normal));
        const double sa = 4 * PI * sr * sr;
        
        ray shadow_ray;
        shadow_ray.origin = info.point + normal * SHADOW_BIAS;
        shadow_ray.direction = ld;
        shadow_ray.type = RT_SHADOW;
        shadow_ray.depth = r.depth + 1;
        shadow_ray.ior = r.ior;

        hit_record dummy;
        bool intersection = world->trace(shadow_ray, 0.001, sr - SHADOW_BIAS, dummy);
        
        final_color += intersection 
        ? color{0,0,0} 
        : color(world->lights[i].intensity / sa * albedo * cos);
    }

    return final_color;
}

color renderer::shade_reflective(const ray& r, const hit_record& info) const
{
    const material* mat = info.mat;
    const color albedo = mat->albedo.evaluate(info);

    vec3 N = mat->smooth_shading ? info.hit_normal : info.geometric_normal;
    vec3 A = r.direction;

    vec3 new_direction = A - 2 * dot(A,N) * N;

    ray reflection;
    reflection.origin       = info.point + N * REFLECTION_BIAS;
    reflection.direction    = new_direction;
    reflection.type         = RT_REFLECTION; 
    reflection.depth        = r.depth + 1;
    reflection.ior          = r.ior;

    return albedo * shade(reflection);
}

color renderer::shade_refractive(const ray& r, const hit_record& info) const 
{
    const material* mat = info.mat;

    vec3 I = unit_vector(r.direction);
    vec3 N = mat->smooth_shading ? info.hit_normal : info.geometric_normal;
    double n1 = r.ior;
    double n2 = mat->ior;

    if(dot(r.direction, N) > 0) 
    {
        std::swap(n1,n2); // leaving object
        N = -N;
    }

    double cos_a = -dot(I, N);
    double sin_a_pow_2 = std::max(0.0, 1 - cos_a * cos_a);
    double sin_a = std::sqrt(sin_a_pow_2);

    ray reflected;
    reflected.origin = info.point + N * REFLECTION_BIAS;
    reflected.direction = unit_vector(I + 2 * cos_a * N);
    reflected.type = RT_REFLECTION;
    reflected.depth = r.depth + 1;
    reflected.ior = n1;

    if(sin_a >= n2/n1) return shade(reflected); // Total internal reflection

    double sin_b = sin_a * n1 / n2;
    double cos_b = std::sqrt(1 - sin_b * sin_b);

    vec3 A = cos_b * -N;
    vec3 C = I + cos_a * N;
    C = C.length_pow2() <= 1e-9 ? vec3(0,0,0) : unit_vector(C);

    vec3 B = C * sin_b;
    vec3 R = A + B;
    
    ray refracted;
    refracted.origin = info.point + (-N * REFRACTION_BIAS);
    refracted.direction = R;
    refracted.ior = n2;
    refracted.type = RT_REFRACTION;
    refracted.depth = r.depth + 1;

    color refracted_color = shade(refracted);
    color reflected_color = shade(reflected);

    double fresnel = schlick(n1,n2,cos_a);
    return fresnel * reflected_color + (1 - fresnel) * refracted_color;
}


double renderer::chaos(double cos_a)
{
    return 0.5 * pow(1.0 - cos_a, 5);
}

double renderer::schlick(double n1, double n2, double cos_a)
{
    double Ro = ((n1 - n2) / (n1 + n2)) * ((n1 - n2) / (n1 + n2));
    return Ro + (1 - Ro) * pow(1 - cos_a, 5);
}