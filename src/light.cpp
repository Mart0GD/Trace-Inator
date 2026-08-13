#include "light.hpp"
#include "scene.hpp"

void light::parse_from_json(const rapidjson::Value& root, const parse_ctx& ctx)
{
    ASSERT_OR_THROW(root.IsObject());

    const auto intensity_itt = root.FindMember(JSON_LIGHTS_INTENSITY);
    const auto position_itt  = root.FindMember(JSON_LIGHTS_POSITION);

    if(intensity_itt != root.MemberEnd()) intensity = intensity_itt->value.GetDouble();
    if(position_itt != root.MemberEnd())  position = parse_vector(position_itt->value.GetArray());
}