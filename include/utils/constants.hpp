#ifndef __CONSTANTS_HPP_INCLUDED__
#define __CONSTANTS_HPP_INCLUDED__

// -- INCLUDES --

#include "math/vec3.hpp"
#include "math/ray.hpp"
#include "math/color.hpp"
#include "math/matrix.hpp"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <numeric>
#include <stdexcept>

// -- CONSTANTS --

static constexpr double       PI = 3.1415926535897932385;
static constexpr double       INF = std::numeric_limits<double>::infinity();

static const std::string      SCENE_FILE_PATH                       = "scenes/HW_11";

inline static constexpr char  JSON_VERSION[]                        = "version";

inline static constexpr char  JSON_SETTINGS[]                       = "settings";
inline static constexpr char  JSON_SETTINGS_BG[]                    = "background_color";
inline static constexpr char  JSON_SETTINGS_IMAGE[]                 = "image_settings";
inline static constexpr char  JSON_SETTINGS_IMAGE_WIDTH[]           = "width";
inline static constexpr char  JSON_SETTINGS_IMAGE_HEIGHT[]          = "height";

inline static constexpr char  JSON_CAMERA[]                         = "camera";
inline static constexpr char  JSON_CAMERA_MATRIX[]                  = "matrix";
inline static constexpr char  JSON_CAMERA_POSITION[]                = "position";
inline static constexpr char  JSON_CAMERA_FOV[]                     = "fov";

inline static constexpr char  JSON_OBJECTS[]                        = "objects";
inline static constexpr char  JSON_OBJECTS_VERTICES[]               = "vertices";
inline static constexpr char  JSON_OBJECTS_TRIANGLES[]              = "triangles";
inline static constexpr char  JSON_OBJECTS_UVS[]                    = "uvs";
inline static constexpr char  JSON_OBJECTS_MATERIAL_INDEX[]         = "material_index";

inline static constexpr char  JSON_LIGHTS[]                         = "lights";
inline static constexpr char  JSON_LIGHTS_POSITION[]                = "position";
inline static constexpr char  JSON_LIGHTS_INTENSITY[]               = "intensity";

inline static constexpr char  JSON_MATERIALS[]                      = "materials";
inline static constexpr char  JSON_MATERIALS_TYPE[]                 = "type";
inline static constexpr char  JSON_MATERIALS_ALBEDO[]               = "albedo";
inline static constexpr char  JSON_MATERIALS_SMOOTH_SHADING[]       = "smooth_shading";
inline static constexpr char  JSON_MATERIALS_IOR[]                  = "ior";

inline static constexpr char  JSON_TEXTURES[]                       = "textures";
inline static constexpr char  JSON_TEXTURES_NAME[]                  = "name";
inline static constexpr char  JSON_TEXTURES_TYPE[]                  = "type";
inline static constexpr char  JSON_TEXTURES_ALBEDO[]                = "albedo";

inline static constexpr char  JSON_TEXTURES_EDGES_EDGE_COLOR[]      = "edge_color";
inline static constexpr char  JSON_TEXTURES_EDGES_INNER_COLOR[]     = "inner_color";
inline static constexpr char  JSON_TEXTURES_EDGES_EDGE_WIDTH[]      = "edge_width";

inline static constexpr char  JSON_TEXTURES_CHECKER_COLOR_A[]       = "color_A";
inline static constexpr char  JSON_TEXTURES_CHECKER_COLOR_B[]       = "color_B";
inline static constexpr char  JSON_TEXTURES_CHECKER_SQUARE_SIZE[]   = "square_size";

inline static constexpr char  JSON_TEXTURES_BITMAP_FILE_PATH[]      = "file_path";

// -- MACROS --

#define ASSERT_OR_THROW(expr)               \
if(expr) ;                                  \
else {                                      \
    assert(false);                          \
    throw std::invalid_argument("error!");  \
}               

// Note: uncomment for debug features
// #define _DEBUG

#ifndef L1_CACHE_LINE_SIZE
#define L1_CACHE_LINE_SIZE 64
#endif

// -- FUNCTIONS --

inline double deg_to_rads(double deg)
{
    return deg * (PI / 180.0);
}

inline vec3 parse_vector(const rapidjson::Value::ConstArray& arr)
{
    ASSERT_OR_THROW(arr.Size() == 3);
    
    return vec3
    {
        static_cast<double>(arr[0].GetDouble()),
        static_cast<double>(arr[1].GetDouble()),
        static_cast<double>(arr[2].GetDouble())
    };
}

inline void* aligned_alloc(size_t size, size_t allignment = L1_CACHE_LINE_SIZE)
{
    if(size == 0) return nullptr;

#if defined(_MSC_VER) || defined(_WIN32)
    return _aligned_malloc(size, allignment);
#else
    void* ptr = nullptr;
    if(posix_memalign(&ptr, allignment, size) != 0){
        ptr = nullptr;
    }
    return ptr;
#endif
}

inline void aligned_free(void* ptr)
{
    if(!ptr) return;

#if defined(_MSC_VER) || defined(_WIN32)
    _aligned_free(ptr); 
#else
    std::free(ptr);    
#endif
}

#endif