#pragma once
#include "Engine/Renderer/VertexFormat.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <vector>
#include <memory>

enum
{
    LevelWidth = 20,
    LevelHeight = 20,
};

struct MeshData;
class Engine;
class StaticMesh;
class Material;
class IRenderBuffer;

struct LevelVertex
{
    glm::vec3 position;
    glm::vec2 texCoord;

    static VertexFormat format()
    {
        VertexFormat fmt;
        fmt.addAttribute(VertexType::Float3);
        fmt.addAttribute(VertexType::Float2);
        return fmt;
    }
};

struct LevelStaticMesh
{
    glm::mat4 matrix;
    const MeshData* mesh;
};

struct LevelData
{
    bool walkable[LevelWidth * LevelHeight];
    int playerX;
    int playerY;
    const LevelVertex* vertices;
    const uint16_t* indices;
    const LevelStaticMesh* staticMeshes;
    size_t vertexCount;
    size_t indexCount;
    size_t staticMeshCount;
};

class Level
{
public:
    Level(Engine* engine, const LevelData* data);
    ~Level();

    bool isWalkable(int x, int y) const;

    void render() const;

private:
    struct StaticObject
    {
        glm::mat4 matrix;
        std::shared_ptr<StaticMesh> mesh;
    };

    Engine* mEngine;
    bool mWalkable[LevelWidth * LevelHeight];
    std::unique_ptr<IRenderBuffer> mVertexBuffer;
    std::unique_ptr<IRenderBuffer> mIndexBuffer;
    std::shared_ptr<Material> mMaterial;
    std::vector<StaticObject> mStaticObjects;
    size_t mIndexCount;
};
