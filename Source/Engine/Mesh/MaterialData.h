#pragma once

struct ShaderCode;
struct TextureData;
class VertexFormat;

struct MaterialData
{
    unsigned textureCount;
    const TextureData* const* textures;
    const ShaderCode* shader;
    VertexFormat (*vertexFormat)(void);
};
