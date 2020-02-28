#include "Level.h"
#include "Engine/Core/Engine.h"
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Renderer/IRenderBuffer.h"
#include "Engine/Renderer/IShaderProgram.h"
#include "Compiled/Shaders.h"
#include <vector>
#include <cstring>

Level::Level(Engine* engine, const LevelData* data)
    : mEngine(engine)
{
    memcpy(mWalkable, data->walkable, LevelWidth * LevelHeight * sizeof(bool));
    mPlayerPos = glm::vec2(data->playerX, data->playerY);

    mShader = mEngine->renderDevice()->createShaderProgram(&levelShader);
    mVertexBuffer = mEngine->renderDevice()->createBufferWithData(data->vertices, data->vertexCount * sizeof(LevelVertex));
    mIndexBuffer = mEngine->renderDevice()->createBufferWithData(data->indices, data->indexCount * sizeof(uint16_t));
}

Level::~Level()
{
}

bool Level::isWalkable(int x, int y) const
{
    if (x < 0 || y < 0 || x >= LevelWidth || y >= LevelHeight)
        return false;
    return mWalkable[y * LevelWidth + x];
}
