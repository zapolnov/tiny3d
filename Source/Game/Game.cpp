#include "Game.h"
#include "Resources/Compiled/Levels.h"
#include "Resources/Compiled/Animations.h"
#include "Resources/Compiled/Meshes.h"
#include "Engine/Core/Engine.h"
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Mesh/AnimatedMesh.h"
#include "Engine/ResMgr/ResourceManager.h"
#include <glm/gtc/matrix_transform.hpp>

Game::Game(Engine* engine)
    : mEngine(engine)
{
    mCamera.setFov(90.0f * 3.1415f / 180.0f);
    mCamera.setZRange(1.0f, 100.0f);

    loadLevel(&Levels::level1);

    mPlayerMesh = engine->resourceManager()->cachedAnimatedMesh(&Meshes::character);
    mPlayerMesh->setAnimation(&Animations::characterIdle);
}

Game::~Game()
{
}

void Game::update(float frameTime)
{
    mCamera.setSize(mEngine->renderDevice()->viewportSize());
    mCamera.setUpVector(glm::vec3(0.0f, -1.0f, 0.0f));
    mCamera.setPosition(mPlayerPos + glm::vec3(0.0f, 1.0f, 5.0f));
    mCamera.setTarget(mPlayerPos);

    mPlayerMesh->addTime(frameTime);
}

void Game::render()
{
    mEngine->renderDevice()->setProjectionMatrix(mCamera.projectionMatrix());
    mEngine->renderDevice()->setViewMatrix(mCamera.viewMatrix());

    mLevel->render();

    // render character

    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, mPlayerPos);
    m = glm::rotate(m, 3.1415f * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.005f, 0.0025f, 0.005f));
    mEngine->renderDevice()->setModelMatrix(m);
    mPlayerMesh->render();
}

void Game::loadLevel(const LevelData* level)
{
    mLevel = std::make_unique<Level>(mEngine, level);
    mPlayerPos = glm::vec3(level->playerX, level->playerY, 0.0f);
}
