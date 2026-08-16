#ifndef __LIGHT_HPP_INCLUDED__
#define __LIGHT_HPP_INCLUDED__

#include "utils/constants.hpp"

// Forward declaration
struct parse_ctx;

struct light {

    point3D position  = {0,0,0};
    fp      intensity = 300;
    
    void parse_from_json(const rapidjson::Value& root, const parse_ctx& ctx);
};

#endif