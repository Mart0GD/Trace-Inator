#include "light.hpp"

void light::parse_from_json(const rapidjson::Value& root)
{
    ASSERT_OR_THROW(root.IsObject());

    const auto intensity_itt = root.FindMember(JSON_LIGHTS_INTENSITY);
    const auto position_itt  = root.FindMember(JSON_LIGHTS_POSITION);

    ASSERT_OR_THROW(intensity_itt != root.MemberEnd() && position_itt != root.MemberEnd());

    intensity = intensity_itt->value.GetDouble();
    position = parse_vector(position_itt->value.GetArray());
}