#include "texture.hpp"
#include "scene.hpp"

#include <algorithm>

// STB 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bitmap_texture::~bitmap_texture() noexcept
{
    if(buffer != nullptr)
    {
        stbi_image_free(buffer); // will be called from arena
        buffer = nullptr;
    }
}

void albedo_texture::parse_from_json(const rapidjson::Value& info)
{
    const auto albedo_itt = info.FindMember(JSON_TEXTURES_ALBEDO); 
    ASSERT_OR_THROW(albedo_itt != info.MemberEnd());

    this->albedo = parse_vector(albedo_itt->value.GetArray());
}

void edges_texture::parse_from_json(const rapidjson::Value& info)
{
    const auto edgde_color_itt = info.FindMember(JSON_TEXTURES_EDGES_EDGE_COLOR);
    const auto inner_color_itt = info.FindMember(JSON_TEXTURES_EDGES_INNER_COLOR);
    const auto edge_width_itt  = info.FindMember(JSON_TEXTURES_EDGES_EDGE_WIDTH);

    ASSERT_OR_THROW(
        edgde_color_itt != info.MemberEnd() &&
        inner_color_itt != info.MemberEnd() &&
        edge_width_itt  != info.MemberEnd() &&
        edgde_color_itt->value.IsArray()    &&
        inner_color_itt->value.IsArray()  
    );

    this->edge_color = parse_vector(edgde_color_itt->value.GetArray());
    this->inner_color = parse_vector(inner_color_itt->value.GetArray());
    this->edge_width = edge_width_itt->value.GetDouble();
}

void checker_texture::parse_from_json(const rapidjson::Value& info)
{
    const auto color_A_itt = info.FindMember(JSON_TEXTURES_CHECKER_COLOR_A);
    const auto color_B_itt = info.FindMember(JSON_TEXTURES_CHECKER_COLOR_B);
    const auto square_size_itt  = info.FindMember(JSON_TEXTURES_CHECKER_SQUARE_SIZE);

    ASSERT_OR_THROW(
        color_A_itt     != info.MemberEnd() &&
        color_B_itt     != info.MemberEnd() &&
        square_size_itt != info.MemberEnd() &&
        color_A_itt->value.IsArray()    &&
        color_B_itt->value.IsArray()  
    );

    this->color_A = parse_vector(color_A_itt->value.GetArray());
    this->color_B = parse_vector(color_B_itt->value.GetArray());
    this->square_size = square_size_itt->value.GetDouble();
}

void bitmap_texture::parse_from_json(const rapidjson::Value& info)
{
    const auto file_path_itt = info.FindMember(JSON_TEXTURES_BITMAP_FILE_PATH);
    ASSERT_OR_THROW(file_path_itt != info.MemberEnd());

    std::string file_path = SCENE_FILE_PATH + file_path_itt->value.GetString();

    this->buffer = stbi_load(file_path.c_str(), &width, &height, &channels, 0);
}

void texture_handle::parse_from_json(const rapidjson::Value& info, const parse_ctx& ctx)
{
    ASSERT_OR_THROW(info.IsObject());

    const auto name_itt = info.FindMember(JSON_TEXTURES_NAME);
    const auto type_itt = info.FindMember(JSON_TEXTURES_TYPE);

    ASSERT_OR_THROW(
        name_itt != info.MemberEnd() && type_itt != info.MemberEnd() &&
        name_itt->value.IsString()   && type_itt->value.IsString()
    );

    auto itt = texture_types.find(type_itt->value.GetString());
    ASSERT_OR_THROW(itt != texture_types.end());

    texture_type type = itt->second;
    switch (type)
    {
        case TT_ALBEDO:
        {
            albedo_texture* at = ctx.al.alloc<albedo_texture>();
            at->parse_from_json(info);

            this->ptr = Texture(at); 
        } break;
        case TT_CHECKER:    
        {
            checker_texture* ct = ctx.al.alloc<checker_texture>();
            ct->parse_from_json(info);

            this->ptr = Texture(ct); 
        } break;
        case TT_EDGES:      
        {
            edges_texture* et = ctx.al.alloc<edges_texture>();
            et->parse_from_json(info);

            this->ptr = Texture(et); 
        } break;
        case TT_BITMAP:    
        {
            bitmap_texture* bt = ctx.al.alloc<bitmap_texture>();
            bt->parse_from_json(info);

            this->ptr = Texture(bt); 
        } break;
    }

    this->name = name_itt->value.GetString();
}

color texture_handle::evaluate(const hit_record& info) const
{
    switch (ptr.tag())
    {
        case TT_ALBEDO:  return ptr.get<albedo_texture>()->albedo;

        case TT_EDGES: 
        {
            edges_texture* txt = ptr.get<edges_texture>();

            double width = txt->edge_width;
            bool on_edge = info.baryU < width || info.baryV < width || 1 - (info.baryU + info.baryV) < width;

            return on_edge ? txt->edge_color : txt->inner_color;
        } break;

        case TT_CHECKER:
        {
            checker_texture* txt = ptr.get<checker_texture>();

            int32_t x = static_cast<int32_t>(std::floor(info.pUV.x / txt->square_size));
            int32_t y = static_cast<int32_t>(std::floor(info.pUV.y / txt->square_size));

            bool even = ((x + y) % 2) == 0;
            return even ? txt->color_A : txt->color_B;
        } break;

        case TT_BITMAP: // TODO: implement bilinear interpolation
        {
            bitmap_texture* txt = ptr.get<bitmap_texture>();
            if(!txt->buffer) return {1,0,1}; // famous magenta 

            double u = info.pUV.x - std::floor(info.pUV.x);
            double v = 1 - (info.pUV.y - std::floor(info.pUV.y)); // invert v

            int32_t x = std::clamp(static_cast<int32_t>(u * txt->width), 0, txt->width - 1);
            int32_t y = std::clamp(static_cast<int32_t>(v * txt->height), 0, txt->height - 1);

            size_t index = (y * txt->width + x) * txt->channels; // can have an alpha channel

            return color {
                txt->buffer[index]      / 255.0, 
                txt->buffer[index + 1]  / 255.0, 
                txt->buffer[index + 2]  / 255.0
            };
        } break;

        default: ASSERT_OR_THROW(false); break;
    }
}