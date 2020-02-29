#pragma once

struct ShaderCode;
struct TextureData;

struct MaterialData
{
    unsigned textureCount;
    const TextureData* const* textures;
    const ShaderCode* shader;
};
