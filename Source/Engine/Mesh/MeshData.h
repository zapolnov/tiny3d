#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <cstdint>

struct MeshVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 texCoord;
};

struct MeshMaterial
{
    unsigned firstIndex;
    unsigned indexCount;
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
