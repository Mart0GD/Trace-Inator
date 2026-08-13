#ifndef __LIGHT_HPP_INCLUDED__
#define __LIGHT_HPP_INCLUDED__

#include "utils/constants.hpp"

struct light {

    point3D position;
    double  intensity;
    
    void parse_from_json(const rapidjson::Value& root);
};

#endif