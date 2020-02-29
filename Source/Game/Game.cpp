#include "Game.h"
#include "Resources/Compiled/Levels.h"
#include "Engine/Core/Engine.h"
#include "Engine/Renderer/IRenderDevice.h"

Game::Game(Engine* engine)
    : mEngine(engine)
{
    mCamera.setFov(90.0f * 3.1415f / 180.0f);
    mCamera.setZRange(1.0f, 100.0f);

    mLevel = std::make_unique<Level>(mEngine, &Levels::level1);
}

Game::~Game()
{
}

void Game::render()
{
    mCamera.setSize(mEngine->renderDevice()->viewportSize());
    mCamera.setPosition(glm::vec3(10, 10, 10));
    mCamera.setTarget(glm::vec3(10, 10, 0));

    mEngine->renderDevice()->setProjectionMatrix(mCamera.projectionMatrix());
    mEngine->renderDevice()->setViewMatrix(mCamera.viewMatrix());

    mLevel->render();
}
