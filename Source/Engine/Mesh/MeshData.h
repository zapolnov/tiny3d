#pragma once
#include "Engine/Renderer/VertexFormat.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
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

struct MeshSkinningVertex
{
    float boneWeights[4] = {0};
    uint8_t boneIndices[4] = {0};

    static VertexFormat format()
    {
        VertexFormat fmt = MeshVertex::format();
        fmt.addAttribute(VertexType::Float4, 1); // boneWeights
        fmt.addAttribute(VertexType::UByte4, 1); // normal
        return fmt;
    }
};

struct MeshBone
{
    static const uint8_t InvalidIndex = -1;

    const char* name = nullptr;
    uint8_t parentIndex = InvalidIndex;
    glm::mat4 matrix{1.0f};
};

struct MeshSkeleton
{
    glm::mat4 globalInverseTransform;
};

struct MeshPositionKey
{
    float time;
    glm::vec3 position;
};

struct MeshRotationKey
{
    float time;
    glm::quat rotation;
};

struct MeshScaleKey
{
    float time;
    glm::vec3 scale;
};

struct MeshBoneAnimation
{
    const MeshPositionKey* positionKeys;
    const MeshRotationKey* rotationKeys;
    const MeshScaleKey* scaleKeys;
    size_t positionKeyCount;
    size_t rotationKeyCount;
    size_t scaleKeyCount;
};

struct MeshAnimation
{
    float durationInTicks;
    float ticksPerSecond;
    const MeshBoneAnimation* boneAnimations;
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
    const MeshBone* bones;
    const glm::mat4* globalInverseTransform;
    const MeshSkinningVertex* skinningVertices;
    const uint16_t* indices;
    const MeshMaterial* materials;
    size_t vertexCount;
    size_t skinningVertexCount;
    size_t indexCount;
    size_t materialCount;
    size_t boneCount;
};
