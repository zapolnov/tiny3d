#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <memory>

enum
{
    LevelWidth = 20,
    LevelHeight = 20,
};

class Engine;
class IShaderProgram;
class IRenderBuffer;

struct LevelVertex
{
    glm::vec3 position;
};

struct LevelData
{
    bool walkable[LevelWidth * LevelHeight];
    int playerX;
    int playerY;
    const LevelVertex* vertices;
    const uint16_t* indices;
    size_t vertexCount;
    size_t indexCount;
};

class Level
{
public:
    Level(Engine* engine, const LevelData* data);
    ~Level();

    bool isWalkable(int x, int y) const;

private:
    Engine* mEngine;
    bool mWalkable[LevelWidth * LevelHeight];
    glm::vec2 mPlayerPos;
    std::unique_ptr<IRenderBuffer> mVertexBuffer;
    std::unique_ptr<IRenderBuffer> mIndexBuffer;
    std::unique_ptr<IShaderProgram> mShader;
};
