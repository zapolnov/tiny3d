#include "Level.h"
#include "Engine/Core/Engine.h"
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Renderer/IRenderBuffer.h"
#include "Engine/Renderer/IPipelineState.h"
#include "Engine/Renderer/ITexture.h"
#include "Engine/Renderer/IShaderProgram.h"
#include "Engine/Mesh/StaticMesh.h"
#include "Engine/ResMgr/ResourceManager.h"
#include "Compiled/Shaders.h"
#include "Compiled/Textures.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cstring>

Level::Level(Engine* engine, const LevelData* data)
    : mEngine(engine)
    , mIndexCount(data->indexCount)
{
    memcpy(mWalkable, data->walkable, LevelWidth * LevelHeight * sizeof(bool));
    mPlayerPos = glm::vec2(data->playerX, data->playerY);

    mStaticObjects.reserve(data->staticMeshCount);
    for (size_t i = 0; i < data->staticMeshCount; i++) {
        StaticObject obj;
        obj.x = data->staticMeshes[i].x;
        obj.y = data->staticMeshes[i].y;
        obj.mesh = mEngine->resourceManager()->cachedStaticMesh(data->staticMeshes[i].mesh);
        mStaticObjects.emplace_back(std::move(obj));
    }

    mShader = mEngine->renderDevice()->createShaderProgram(&Shaders::levelShader);
    mVertexBuffer = mEngine->renderDevice()->createBufferWithData(data->vertices, data->vertexCount * sizeof(LevelVertex));
    mIndexBuffer = mEngine->renderDevice()->createBufferWithData(data->indices, data->indexCount * sizeof(uint16_t));
    mTilesetTexture = mEngine->renderDevice()->createTexture(&Textures::dungeonTileset);
    mPipelineState = mEngine->renderDevice()->createPipelineState(mShader, LevelVertex::format());
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

void Level::render() const
{
    mEngine->renderDevice()->setModelMatrix(glm::mat4(1.0f));
    mEngine->renderDevice()->setPipelineState(mPipelineState);
    mEngine->renderDevice()->setVertexBuffer(mVertexBuffer);
    mEngine->renderDevice()->setTexture(0, mTilesetTexture);
    mEngine->renderDevice()->drawIndexedPrimitive(Triangles, mIndexBuffer, 0, mIndexCount);

    for (const auto& obj : mStaticObjects) {
        mEngine->renderDevice()->setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(obj.x, obj.y, 0.0f)));
        obj.mesh->render();
    }
}
