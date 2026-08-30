#include "camera.hpp"
#include "scene.hpp"

camera::camera(point3D position, fp v_fov)
    : position(position)
    , fov(v_fov)
{
    init(INIT_WIDTH, INIT_HEIGHT);
}

void camera::init(fp width, fp height)
{
    // FOV settings
    fp focal_length = 1;
    fp angle = deg_to_rads(fov);
    fp h = std::tan(angle / 2);
    
    // Viewport settings 
    fp v_height = 2 * h * focal_length;
    fp v_width  = v_height * width / height;

    vec3 viewport_u = vec3(v_width, 0, 0);
    vec3 viewport_v = vec3(0, -v_height, 0);

    delta_u = viewport_u / width;
    delta_v = viewport_v / height;

    vec3 top_left = vec3(0,0, -focal_length) - viewport_u / 2.0 - viewport_v / 2.0;
    pixel_00 = top_left + 0.5 * (delta_u + delta_v);
}

void camera::pan(fp degrees)
{   
    fp rad = deg_to_rads(degrees);
    matrix rotation_Y = 
    {
        std::cos(rad), 0.f, -std::sin(rad),
        0.f,       1.f,     0.f   ,
        std::sin(rad), 0.f, std::cos(rad)
    };
    rotation_matrix = rotation_matrix * rotation_Y; // accumulates rotation ...
}

void camera::tilt(fp degrees)
{
    fp rad = deg_to_rads(degrees);
    matrix rotation_X = 
    {
        1.f,       0      ,    0     ,
        0,      std::cos(rad) , std::sin(rad),
        0,      -std::sin(rad), std::cos(rad)
    };
    rotation_matrix = rotation_matrix * rotation_X; // accumulates rotation ...
}

void camera::roll(fp degrees)
{
    fp rad = deg_to_rads(degrees);
    matrix rotation_Z = 
    {
        std::cos(rad) , std::sin(rad), 0.f,
        -std::sin(rad), std::cos(rad), 0.f,
        0.f       , 0.f      , 1.f
    };
    rotation_matrix = rotation_matrix * rotation_Z; // accumulates rotation ...
}

void camera::translate(const vec3& dir)
{
    // Transform direction from local to global
    const vec3 world_space_dir = dir * rotation_matrix;
    position += world_space_dir;
}

void camera::parse_from_json(const rapidjson::Value& root, const parse_ctx& ctx) 
{   
    ASSERT_OR_THROW(root.IsObject());

    switch (ctx.version)
    {
        case 3:
        {
            const auto fov_itt = root.FindMember(JSON_CAMERA_FOV);

            // Has FOV
            if(fov_itt != root.MemberEnd()) this->fov = fov_itt->value.GetDouble();
        }
        case 1: case 2:
        {
            const auto matrix_itt     = root.FindMember(JSON_CAMERA_MATRIX);
            const auto position_itt   = root.FindMember(JSON_CAMERA_POSITION);

            // Has Matrix
            if(matrix_itt != root.MemberEnd())
            {
                const rapidjson::Value& matrix      = matrix_itt->value;    
                ASSERT_OR_THROW(matrix.IsArray() && matrix.Size() == 9);

                for (size_t i = 0; i < matrix.Size(); ++i)
                {
                    this->rotation_matrix[i / 3][i % 3] = matrix[i].GetDouble();
                }
                
            }

            // Has position 
            if(position_itt != root.MemberEnd())
            {
                ASSERT_OR_THROW(position_itt->value.IsArray());
                this->position = parse_vector(position_itt->value.GetArray());
            }
        } break;
    }
}

ray camera::get_ray(fp u, fp v) const
{
    point3D pixel       = pixel_00 + delta_u * u + delta_v * v;

    vec3    local_dir   = pixel; // pixel - (0,0,0)
    vec3    global_dir  = local_dir * rotation_matrix;

    ray out;
    out.origin = position;
    out.direction = global_dir;
    out.type = RT_CAMERA;
    out.depth = 0;
    out.ior = 1;
    out.inv_dir = 1 / out.direction;

    return out;        
}

void camera::look_at(const point3D& target)
{
    vec3 forward = unit_vector(target - position);

    vec3 world_up = {0, -1, 0};

    // Edge case when we look parallel to the Y axis, z is up
    if(std::abs(dot(forward, world_up)) > 0.99) world_up = {0,0,1};

    vec3 right = unit_vector(cross(forward, world_up));

    vec3 down = cross(forward, right);

    rotation_matrix =
    {
        right.x,  right.y,  right.z,
        down.x,   down.y,  down.z,
        -forward.x, -forward.y, -forward.z
    };
}