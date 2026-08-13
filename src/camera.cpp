#include "camera.hpp"
#include "scene.hpp"

camera::camera(point3D position, double v_fov)
    : position(position)
    , fov(v_fov)
{
    init(INIT_WIDTH, INIT_HEIGHT);
}

void camera::init(double width, double height)
{
    // FOV settings
    double focal_length = 1;
    double angle = deg_to_rads(fov);
    double h = std::tan(angle / 2);
    
    // Viewport settings 
    double v_height = 2 * h * focal_length;
    double v_width  = v_height * width / height;

    vec3 viewport_u = vec3(v_width, 0, 0);
    vec3 viewport_v = vec3(0, -v_height, 0);

    delta_u = viewport_u / width;
    delta_v = viewport_v / height;

    vec3 top_left = vec3(0,0, -focal_length) - viewport_u / 2.0 - viewport_v / 2.0;
    pixel_00 = top_left + 0.5 * (delta_u + delta_v);
}

void camera::pan(double degrees)
{   
    double rad = deg_to_rads(degrees);
    matrix rotation_Y = 
    {
        cos(rad), 0.f, -sin(rad),
        0.f,       1.f,     0.f   ,
        sin(rad), 0.f, cos(rad)
    };
    rotation_matrix = rotation_matrix * rotation_Y; // accumulates rotation ...
}

void camera::tilt(double degrees)
{
    double rad = deg_to_rads(degrees);
    matrix rotation_X = 
    {
        1.f,       0      ,    0     ,
        0,      cos(rad) , sin(rad),
        0,      -sin(rad), cos(rad)
    };
    rotation_matrix = rotation_matrix * rotation_X; // accumulates rotation ...
}

void camera::roll(double degrees)
{
    double rad = deg_to_rads(degrees);
    matrix rotation_Z = 
    {
        cos(rad) , sin(rad), 0.f,
        -sin(rad), cos(rad), 0.f,
        0.f       , 0.f      , 1.f
    };
    rotation_matrix = rotation_matrix * rotation_Z; // accumulates rotation ...
}

void camera::translate(const vec3& dir)
{
    const vec3 world_space_dir = dir * rotation_matrix;
    position += world_space_dir;
}

void camera::parse_from_json(const rapidjson::Value& root) 
{   
    ASSERT_OR_THROW(root.IsObject());

    auto matrix_itt     = root.FindMember(JSON_CAMERA_MATRIX);
    auto position_itt   = root.FindMember(JSON_CAMERA_POSITION);
    auto v_fov_itt      = root.FindMember(JSON_CAMERA_FOV);

    ASSERT_OR_THROW(
        matrix_itt      != root.MemberEnd()  &&
        position_itt    != root.MemberEnd()  &&
        v_fov_itt       != root.MemberEnd()
    );

    const rapidjson::Value& matrix      = matrix_itt->value;    
    const rapidjson::Value& position    = position_itt->value;
    const rapidjson::Value& v_fov       = v_fov_itt->value;
    
    ASSERT_OR_THROW(
        matrix.IsArray()    && matrix.Size() == 9 &&
        position.IsArray()  && position.Size() == 3
    );

    this->position = parse_vector(position.GetArray());
    this->fov = v_fov.GetDouble();

    int32_t size = matrix.Size();
    for (int i = 0; i < size; i++)
    {
        this->rotation_matrix[i / 3][i % 3] = matrix[i].GetDouble();
    }
    
}

ray camera::get_ray(double u, double v) const
{
    point3D pixel       = pixel_00 + delta_u * u + delta_v * v;

    vec3    local_dir   = pixel; // pixel - (0,0,0)
    vec3    global_dir  = local_dir * rotation_matrix;

    ray out;
    out.origin = position;
    out.direction = global_dir;

    return out;        
}
