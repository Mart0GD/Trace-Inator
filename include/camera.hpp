#ifndef __CAMERA_HPP_INCLUDED__
#define __CAMERA_HPP_INCLUDED__

#include "utils/constants.hpp"

// Forward declaration
struct parse_ctx;

class camera {
public:

// -- CORE FUNCTIONALITY --

    camera(point3D position = {0,0,0}, fp v_fov = 90);
    void parse_from_json(const rapidjson::Value& root, const parse_ctx& ctx);

    void init(fp width, fp height);
    ray get_ray(const fp u, const fp v) const;
    
//  -- CAMERA MOVEMENT --

    void translate(const vec3& dir);
    void pan(fp degrees);  // <- rotate around y
    void tilt(fp degrees); // <- rotate around x
    void roll(fp degrees); // <- rotate around z
    void look_at(const point3D& target);

    inline void set_position(const point3D& pos) { this->position = pos;}

private:

    static constexpr int32_t INIT_WIDTH  = 800;
    static constexpr int32_t INIT_HEIGHT = 400;

private:

    point3D position;           // start position
    matrix  rotation_matrix;    // the direction angle

    point3D pixel_00;           // position of the top and left-most pixel to render
    vec3    delta_u;            // direction for the next pixel vertically      (normalised)
    vec3    delta_v;            // direction for the next pixel horizontally    (normalised)

    fp  fov;                    // vertical field of view
};

#endif