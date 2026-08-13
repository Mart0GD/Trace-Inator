#include "material.hpp"
#include "scene.hpp"

void material::parse_from_json(const rapidjson::Value& root, const parse_ctx& ctx)
{
    ASSERT_OR_THROW(root.IsObject());

    const auto type_itt             = root.FindMember(JSON_MATERIALS_TYPE);
    const auto albedo_itt           = root.FindMember(JSON_MATERIALS_ALBEDO);
    const auto smooth_shading_itt   = root.FindMember(JSON_MATERIALS_SMOOTH_SHADING);
    const auto ior_itt              = root.FindMember(JSON_MATERIALS_IOR);

    ASSERT_OR_THROW(type_itt != root.MemberEnd() && type_itt->value.IsString());

    const auto itt = m_types.find(type_itt->value.GetString());
    ASSERT_OR_THROW(itt != m_types.end());

    this->type = itt->second;
    if(smooth_shading_itt != root.MemberEnd())  this->smooth_shading = smooth_shading_itt->value.GetBool();
    if(ior_itt != root.MemberEnd())             this->ior = ior_itt->value.GetDouble();

    switch (ctx.version)
    {
        case 1:
        {
            color c = {1,0,1}; // magenta

            // Has albedo color
            if(albedo_itt != root.MemberEnd())
            {
                ASSERT_OR_THROW(albedo_itt->value.IsArray());

                c = parse_vector(albedo_itt->value.GetArray());
            }

            albedo_texture* txt = ctx.al.alloc<albedo_texture>();
            txt->albedo = c;

            // Wrap in a handle
            texture_handle handle;
            handle.ptr = Texture(txt);

            this->albedo = std::move(handle);
        } break;

        case 2:
            
            // Has texture name
            if(albedo_itt != root.MemberEnd())
            {
                std::string name = albedo_itt->value.GetString();

                for (const texture_handle& handle: ctx.textures)
                {
                    if(handle.name == name) this->albedo = handle;
                }
                ASSERT_OR_THROW(albedo.ptr.ptr() != nullptr);
            }
        break;
    }
}