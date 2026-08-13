#include "material.hpp"

void material::parse_from_json(const rapidjson::Value& root)
{
    ASSERT_OR_THROW(root.IsObject());

    const auto type_itt             = root.FindMember(JSON_MATERIALS_TYPE);
    const auto albedo_itt           = root.FindMember(JSON_MATERIALS_ALBEDO);
    const auto smooth_shading_itt   = root.FindMember(JSON_MATERIALS_SMOOTH_SHADING);
    const auto ior_itt              = root.FindMember(JSON_MATERIALS_IOR);

    ASSERT_OR_THROW(type_itt != root.MemberEnd() && type_itt->value.IsString());

    auto itt = m_types.find(type_itt->value.GetString());
    ASSERT_OR_THROW(itt != m_types.end());

    this->type = itt->second;
    if(albedo_itt != root.MemberEnd())          this->albedo = albedo_itt->value.GetString();
    if(smooth_shading_itt != root.MemberEnd())  this->smooth_shading = smooth_shading_itt->value.GetBool();
    if(ior_itt != root.MemberEnd())             this->ior = ior_itt->value.GetDouble();
}