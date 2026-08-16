#include "renderer.hpp"
#include "geometry/mesh_triangle.hpp"

void renderer::render(std::ostream& os) {
    const camera& camera = world->cam;
    const scene_settings& settings  = world->settings;

    fp width = settings.width;
    fp height = settings.height;
    size_t bucket_size = settings.bucket_size;

//  -- PREPARE BUCKETS --

    std::vector<color> image_buffer(width * height);
    std::vector<bucket> buckets;

    for (size_t x = 0; x < width; x += bucket_size)
    {
        for (size_t y = 0; y < height; y+= bucket_size)
        {
            bucket b;
            b.min_x = x;
            b.max_x = std::min((size_t)width, x + bucket_size);

            b.min_y = y;
            b.max_y = std::min((size_t)height, y + bucket_size);

            buckets.push_back(std::move(b));
        }
        
    }
    
//  -- RENDER --

    std::mutex m;
    std::condition_variable var;
    std::atomic<size_t> bucket_index = 0;
    std::atomic<size_t> completed_buckets = 0;

    for (size_t i = 0; i < pool.workers_count(); ++i)
    {
        pool.enqueue([&]
        {
            while (true)
            {
                size_t index = bucket_index.fetch_add(1);
                if(index >= buckets.size()) break;

                const bucket& b = buckets[index];

                for (int y = b.min_y; y < b.max_y; ++y)
                {
                    for (int x = b.min_x; x < b.max_x; ++x)
                    {
                        ray r = camera.get_ray(x,y);

                        color c = shade(r);
                        image_buffer[y * width + x] = std::move(c);
                    }
                }

                // One bucket rendered
                if(++completed_buckets == buckets.size())
                {
                    std::unique_lock<std::mutex> lock(m);
                    var.notify_one();
                }
            }
            
        });
    }

    // Wait for the pool to finish
    std::unique_lock<std::mutex> lock(m);
    var.wait(lock, [&]
    {
        return completed_buckets.load() == buckets.size();
    });
    
//  -- OUTPUT IMAGE BUFFER --

    os << "P3\n" << width << ' ' << height << "\n255\n";

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            print_color(os, image_buffer[i * width + j]);
        }
    }
}

color renderer::shade(const ray& r) const 
{
    hit_record info;
    bool hit = false;
    
    const bvh& acc_tree = world->acc_tree;

    if(r.depth < MAX_DEPTH) hit = acc_tree.trace(r, 0.001, INF, info);

    if(hit == false)
    {
        // Return background
        vec3 unit_direction = unit_vector(r.direction);
        fp a = 0.5*(unit_direction.y + 1.0);
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
        const fp  sr = ld.length();
        ld = unit_vector(ld);

        const fp cos = std::max(0.f, dot(ld, normal));
        const fp sa = 4 * PI * sr * sr;
        
        ray shadow_ray;
        shadow_ray.origin = info.point + normal * SHADOW_BIAS;
        shadow_ray.direction = ld;
        shadow_ray.type = RT_SHADOW;
        shadow_ray.depth = r.depth + 1;
        shadow_ray.ior = r.ior;
        shadow_ray.inv_dir = 1 / ld;

        hit_record dummy;
        bool intersection = world->acc_tree.trace(shadow_ray, 0.001, sr - SHADOW_BIAS, dummy);
        
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
    reflection.inv_dir      = 1 / new_direction;

    return albedo * shade(reflection);
}

color renderer::shade_refractive(const ray& r, const hit_record& info) const 
{
    const material* mat = info.mat;

    vec3 I = unit_vector(r.direction);
    vec3 N = mat->smooth_shading ? info.hit_normal : info.geometric_normal;
    fp n1 = r.ior;
    fp n2 = mat->ior;

    if(dot(r.direction, N) > 0) 
    {
        std::swap(n1,n2); // leaving object
        N = -N;
    }

    fp cos_a = -dot(I, N);
    fp sin_a_pow_2 = std::max(0.f, 1 - cos_a * cos_a);
    fp sin_a = std::sqrt(sin_a_pow_2);

    ray reflected;
    reflected.origin = info.point + N * REFLECTION_BIAS;
    reflected.direction = unit_vector(I + 2 * cos_a * N);
    reflected.type = RT_REFLECTION;
    reflected.depth = r.depth + 1;
    reflected.ior = n1;
    reflected.inv_dir = 1 / reflected.direction;

    if(sin_a >= n2/n1) return shade(reflected); // Total internal reflection

    fp sin_b = sin_a * n1 / n2;
    fp cos_b = std::sqrt(1 - sin_b * sin_b);

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
    refracted.inv_dir = 1 / R;

    color refracted_color = shade(refracted);
    color reflected_color = shade(reflected);

    fp fresnel = schlick(n1,n2,cos_a);
    return fresnel * reflected_color + (1 - fresnel) * refracted_color;
}


fp renderer::chaos(fp cos_a)
{
    return 0.5 * pow(1.0 - cos_a, 5);
}

fp renderer::schlick(fp n1, fp n2, fp cos_a)
{
    fp Ro = ((n1 - n2) / (n1 + n2)) * ((n1 - n2) / (n1 + n2));
    return Ro + (1 - Ro) * pow(1 - cos_a, 5);
}