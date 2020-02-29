#pragma once

struct ShaderCode;
struct TextureData;

struct MaterialData
{
    unsigned textureCount;
    const TextureData** textures;
    const ShaderCode* shader;
};
