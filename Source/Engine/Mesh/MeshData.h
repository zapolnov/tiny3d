#pragma once
#include "Engine/Renderer/VertexFormat.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <cstdint>

struct MaterialData;

struct MeshVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 texCoord;

    static VertexFormat format()
    {
        VertexFormat fmt;
        fmt.addAttribute(VertexType::Float3); // position
        fmt.addAttribute(VertexType::Float3); // normal
        fmt.addAttribute(VertexType::Float3); // tangent
        fmt.addAttribute(VertexType::Float3); // bitangent
        fmt.addAttribute(VertexType::Float2); // texCoord
        return fmt;
    }
};

struct MeshMaterial
{
    unsigned firstIndex;
    unsigned indexCount;
    const MaterialData* material;
};

struct MeshData
{
    const MeshVertex* vertices;
    const uint16_t* indices;
    const MeshMaterial* materials;
    size_t vertexCount;
    size_t indexCount;
    size_t materialCount;
};
