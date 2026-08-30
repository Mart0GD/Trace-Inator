#include "renderer.hpp"
#include "geometry/mesh_triangle.hpp"
#include <fstream>

void renderer::render(int32_t debug_depth) {
    const camera& camera = world->cam;
    const scene_settings& settings  = world->settings;

    fp width = settings.width;
    fp height = settings.height;
    size_t bucket_size = settings.bucket_size;
    size_t rpp = settings.rpp;

//  -- PREPARE BUCKETS --

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

    image_buffer = std::vector<color>(width * height);

    std::mutex m;
    std::condition_variable var;
    std::atomic<size_t> bucket_index = 0;
    std::atomic<size_t> completed_buckets = 0;

    for (size_t i = 0; i < pool.workers_count(); ++i)
    {
        pool.enqueue([&]
        {
            thread_local std::random_device dev;    
            thread_local std::mt19937 gen(dev());  
            thread_local std::uniform_real_distribution<fp> uniform(0.0, 1.0);

            while (true)
            {
                size_t index = bucket_index.fetch_add(1);
                if(index >= buckets.size()) break;

                const bucket& b = buckets[index];
                
                // Render bucket
                for (int y = b.min_y; y < b.max_y; ++y)
                {
                    for (int x = b.min_x; x < b.max_x; ++x)
                    {
                        color pixel_color;
                            
                        #ifdef _DEBUG 
                            ray r = camera.get_ray(x,y);    
                            pixel_color = shade_debug(r, debug_depth);
                        #else 
                            for(size_t i = 0; i < rpp; ++i)
                            {
                                fp offset_x = uniform(gen) - 0.5;
                                fp offset_y = uniform(gen) - 0.5;
                                
                                ray r = camera.get_ray(x + offset_x,y + offset_y);
                                pixel_color += shade(r, gen);
                            }

                            pixel_color /= rpp;
                        #endif
                            
                        size_t index = y * width + x;
                        image_buffer[index] = pixel_color;
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
}

void renderer::render_sdl() {
    const camera& camera = world->cam;
    const scene_settings& settings = world->settings;

    int width = settings.width;
    int height = settings.height;
    size_t bucket_size = settings.bucket_size;
    size_t rpp = settings.rpp;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL failure: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Raytracer Real-time Preview",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(
        sdl_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        width, height
    );

    std::vector<bucket> buckets;

    for (size_t y = 0; y < height; y += bucket_size) {

        for (size_t x = 0; x < width; x += bucket_size) {

            bucket b;
            b.min_x = x;
            b.max_x = std::min((size_t)width, x + bucket_size);

            b.min_y = y;
            b.max_y = std::min((size_t)height, y + bucket_size);
            buckets.push_back(b);
        }
    }

    image_buffer = std::vector<color>(width * height);
    std::vector<uint32_t> pixel_buffer(width * height, 0);

    std::atomic<bool> is_running{true};
    std::atomic<uint32_t> frame_count{0};

    std::mutex cv_m;
    std::condition_variable cv;

    std::thread render_coordinator([&]() {
        while (is_running) {
            uint32_t current_frame = ++frame_count;

            std::atomic<size_t> bucket_index{0};
            std::atomic<size_t> completed_buckets{0};
            size_t total_buckets = buckets.size();
            size_t total_workers = pool.workers_count();

            for (size_t i = 0; i < total_workers; ++i) {
                pool.enqueue([&, current_frame, total_buckets] {
                    thread_local std::random_device dev;    
                    thread_local std::mt19937 gen(dev());  
                    thread_local std::uniform_real_distribution<fp> uniform(0.0, 1.0);

                    while (is_running) {
                        size_t index = bucket_index.fetch_add(1);
                        if (index >= total_buckets) break;

                        const auto& b = buckets[index];
                        fp weight = 1.f / frame_count.load();

                        for (size_t y = b.min_y; y < b.max_y; ++y) {
                            for (size_t x = b.min_x; x < b.max_x; ++x) {
                                if(!is_running) break;

                                color pixel_color{0, 0, 0};
                                for (size_t r = 0; r < rpp; ++r) {
                                    fp offset_x = uniform(gen) - 0.5;
                                    fp offset_y = uniform(gen) - 0.5;

                                    ray r_ray = camera.get_ray(x + offset_x, y + offset_y);
                                    pixel_color += shade(r_ray, gen);
                                }
                                
                                size_t index = y * width + x;
                                image_buffer[index] = lerp(
                                    image_buffer[index],
                                    pixel_color / static_cast<fp>(rpp),
                                    weight
                                );
                            }
                        }

                        if (++completed_buckets == total_buckets) {
                            std::lock_guard<std::mutex> lock(cv_m);
                            cv.notify_one();
                        }
                    }
                });
            }

            std::unique_lock<std::mutex> lock(cv_m);
            cv.wait(lock, [&] {
                return completed_buckets.load() >= total_buckets || !is_running;
            });
        }
    });

    bool quit = false;
    SDL_Event event;

    while (!quit) {

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        if (frame_count.load() > 0) {
            for (int i = 0; i < width * height; ++i) {
                pixel_buffer[i] = color_to_rgba32(image_buffer[i]);
            }
            
            SDL_UpdateTexture(texture, nullptr, pixel_buffer.data(), width * sizeof(uint32_t));
        }

        SDL_RenderClear(sdl_renderer);
        SDL_RenderCopy(sdl_renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(sdl_renderer);

        SDL_Delay(33);
    }

    is_running = false;

    {
        std::unique_lock<std::mutex> lock(cv_m);
        cv.notify_all();
    }

    if (render_coordinator.joinable()) {
        render_coordinator.join();
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

color renderer::shade(const ray& r, std::mt19937& rng) const 
{
    hit_record info;
    bool hit = false;
    const bvh& acc_tree = world->acc_tree;

    if(r.depth >= MAX_DEPTH) return get_background(r);
    
    hit = acc_tree.trace(r, 0.001, INF, info);
    if(hit == false) return get_background(r);

    const material* mat = info.mat;
    color final_color;


    switch (mat->type)
    {
        case MAT_CONSTANT: 
        {
            final_color = mat->texture.evaluate(info);
        } break;

        case MAT_DIFFUSIVE:
        {
            final_color = shade_diffusive(r, info, rng);
        } break;
        
        case MAT_REFLECTIVE:
        {
            final_color = shade_reflective(r, info, rng);
        } break;
        
        case MAT_REFRACTIVE:
        {
            final_color = shade_refractive(r, info, rng);
        } break;

        default: ASSERT_OR_THROW(true);
    }

    return final_color;
}

color renderer::shade_debug(const ray& r, int debug_depth) const {
    hit_record info;
    int32_t hit = world->acc_tree.trace_debug(r, 0.001, INF, info, debug_depth);;

    if (!hit) {
        vec3 unit_direction = unit_vector(r.direction);
        fp a = 0.5 * (unit_direction.y + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * world->settings.background;
    }

    
    vec3 N = info.geometric_normal;
    return 0.5 * color(N.x + 1.0, N.y + 1.0, N.z + 1.0);
}

color renderer::shade_diffusive(const ray& r, const hit_record& info, std::mt19937& rng) const
{
    const material* mat = info.mat;
    const size_t lights_cnt = world->lights.size();
    const color  albedo = mat->texture.evaluate(info);
    const vec3   normal = mat->smooth_shading ? info.hit_normal : info.geometric_normal;

    // Compute random ray
    vec3 direction = normal + random_unit_vector(rng);
    direction = unit_vector(direction);

    ray diffusive_ray;
    diffusive_ray.origin = info.point + normal * BIAS;
    diffusive_ray.direction = direction;
    diffusive_ray.depth = r.depth + 1;
    diffusive_ray.type = RT_DIFFUSE;
    diffusive_ray.ior = r.ior;
    diffusive_ray.inv_dir = 1 / direction;

    // Indirect light
    color diffuse_light = albedo * shade(diffusive_ray, rng); 

    // Get the direct illumination factor
    color direct_light;
    for (size_t i = 0; i < lights_cnt; i++)
    {
        vec3    ld = world->lights[i].position - info.point;
        const fp  sr = ld.length();
        ld = unit_vector(ld);

        const fp cos = std::max(0.f, dot(ld, normal));
        const fp sa = 4 * PI * sr * sr;
        
        ray shadow_ray;
        shadow_ray.origin = info.point + normal * BIAS;
        shadow_ray.direction = ld;
        shadow_ray.type = RT_SHADOW;
        shadow_ray.depth = r.depth + 1;
        shadow_ray.ior = r.ior;
        shadow_ray.inv_dir = 1 / ld;

        hit_record dummy;
        bool intersection = world->acc_tree.trace(shadow_ray, 0.001, sr, dummy);
        
        // Check for illumination
        if(!intersection || dummy.mat->type == MAT_REFRACTIVE) 
        direct_light += color(world->lights[i].intensity / sa * albedo * cos);
    }
    // combine
    return diffuse_light + direct_light;
}

color renderer::shade_reflective(const ray& r, const hit_record& info, std::mt19937& rng) const
{
    const material* mat = info.mat;
    const color albedo = mat->texture.evaluate(info);

    vec3 N = mat->smooth_shading ? info.hit_normal : info.geometric_normal;
    vec3 A = r.direction;

    vec3 new_direction = reflect(A,N);

    ray reflection;
    reflection.origin       = info.point + N * BIAS;
    reflection.direction    = new_direction;
    reflection.type         = RT_REFLECTION; 
    reflection.depth        = r.depth + 1;
    reflection.ior          = r.ior;
    reflection.inv_dir      = 1 / new_direction;

    return albedo * shade(reflection, rng);
}

color renderer::shade_refractive(const ray& r, const hit_record& info, std::mt19937& rng) const
{
    thread_local std::uniform_real_distribution<fp> uniform;

    const material* mat = info.mat;
    const color albedo = mat->texture.evaluate(info);
    vec3 normal = mat->smooth_shading ? info.hit_normal : info.geometric_normal;
    vec3 dir = unit_vector(r.direction);

    fp n1 = r.ior;
    fp n2 = mat->ior;

    // going out
    if(dot(dir, normal) > 0) 
    {
        n1 = mat->ior;
        n2 = 1.f;
        normal = -normal;
    }

    light_response lr = calculate_fresnel_equasion(dir, normal, n1, n2);
    ray result;
    result.depth = r.depth + 1;

    bool has_reflection = (lr.reflection_weight >= 1.0f) || (uniform(rng) < lr.reflection_weight);
    if(has_reflection) 
    {
        result.direction = reflect(dir, normal);
        result.type = RT_REFLECTION;
        result.ior = n1;
        result.origin = info.point + normal * BIAS;
    }
    else
    {
        result.direction = n1 == n2 ? dir : refract(dir, normal, n1, n2);
        result.type = RT_REFRACTION;
        result.ior = n2;
        result.origin = info.point - normal * BIAS;
    }

    result.inv_dir = 1.0f / result.direction;
    return albedo * shade(result, rng);
}

fp renderer::schlick_appx(fp n1, fp n2, fp cos_a) const
{
    if(abs(n1 - n2) < 1e-9) return 0.f;
    
    fp Ro = ((n1 - n2) / (n1 + n2)) * ((n1 - n2) / (n1 + n2));
    return Ro + (1 - Ro) * pow(1 - cos_a, 5.0f);
}

color renderer::get_background(const ray& r) const
{
    vec3 unit_direction = unit_vector(r.direction);
    fp a = 0.5*(unit_direction.y + 1.0);
    return (1.0-a)*color(1.0, 1.0, 1.0) + a*world->settings.background;
}

vec3 renderer::reflect(const vec3 direction, const vec3& normal) const
{
    return direction - 2 * dot(normal, direction) * normal;
}

vec3  renderer::refract(const vec3& dir, const vec3& normal, fp n1, fp n2) const
{
    if(abs(n1 - n2) < 1e-9) return dir;

    fp ratio = n1 / n2;
    fp cos_a = std::fmin(-dot(dir,normal), 1.0f);

    // n1*sinA = n2*sinB  --> sinB = n1/n2*sinA | ^2
    fp sin_b_2 = ratio * ratio * (1 - cos_a * cos_a); 
    if(sin_b_2 >= 1.0f) return {0,0,0}; // fully reflected

    vec3 refracted_dir = ratio * dir + (ratio * cos_a - std::sqrt(1 - sin_b_2)) * normal;
    return refracted_dir;
}

light_response renderer::calculate_fresnel_equasion(const vec3& dir, const vec3& normal, fp n1, fp n2) const
{
    light_response res;
    
    fp ratio = n1 / n2;
    fp cos_a = std::fmin(-dot(dir, normal), 1.f); // rounding errors
    fp sin_b_2 = ratio * ratio * (1 - cos_a * cos_a);

    if (sin_b_2 > 1.0f)
    {
        // TIR
        res.reflection_weight = 1;
    }
    else
    {
        res.reflection_weight = schlick_appx(n1, n2, cos_a);
    }
    

    res.refraction_weight = 1 - res.reflection_weight;
    return res;
}

void renderer::save(const std::string& name)
{
    const fp width = world->settings.width;
    const fp height = world->settings.height;

//  -- OUTPUT IMAGE BUFFER --

    std::ofstream os(name);
    os << "P3\n" << width << ' ' << height << "\n255\n";

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            color output = image_buffer[i * width + j];

            print_color(os, output);
        }
    }
}

void renderer::animate(const animation_info& info)
{
    if(info.path.size() <= 1) return;

    camera& cam = world->cam;
    cam.set_position(info.path[0]);

    for (int32_t frame = 0; frame < info.frames; ++frame)
    {
        fp t = fp(frame) / (info.frames - 1);

        point3D pos = position_at(info, t);

        cam.set_position(pos);
        cam.look_at(info.target);
        
        render();
        save("frame" + std::to_string(frame) + ".ppm");
    }
    
}

point3D renderer::position_at(const animation_info& info, fp t)
{
    const std::vector<point3D>& path = info.path;
    ASSERT_OR_THROW(path.size() >= 1);

    if(path.size() == 1)
    {
        return path[0];
    }

    if(path.size() == 2)
    {
        return lerp(path[0], path[1], t);
    }

    const size_t points_cnt = path.size();
    const size_t intervals_cnt = points_cnt - 1;

    fp point_scaled = t * intervals_cnt;
    int32_t interval = static_cast<int32_t>(std::floor(point_scaled));
    
    if(interval >= intervals_cnt) interval = intervals_cnt - 1;

    // time in between the points
    const fp local_t = point_scaled - static_cast<fp>(interval);

    const int32_t point1 = interval;
    const int32_t point2 = point1 + 1;

    const int32_t point0 = point1 == 0 ? point1 : point1 - 1;
    const int32_t point3 = point2 + 1 >= points_cnt ? points_cnt - 1 : point2 + 1;

    return catmull_rom(
        path[point0], 
        path[point1], 
        path[point2], 
        path[point3], 
        local_t);
}

// thanks to wikipedia --> https://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline
point3D renderer::catmull_rom(
    const point3D& p0, 
    const point3D& p1, 
    const point3D& p2, 
    const point3D& p3,
    fp t
)
{
    const fp t2 = t * t;
    const fp t3 = t2 * t;

    return 0.5 * (
        (2.0 * p1) +
        (-p0 + p2) * t +
        (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * t2 +
        (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * t3
    );
}