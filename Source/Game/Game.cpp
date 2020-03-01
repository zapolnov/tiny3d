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

    mLevel = std::make_unique<Level>(mEngine, &Levels::level1);

    mPlayerMesh = engine->resourceManager()->cachedAnimatedMesh(&Meshes::character);
    mPlayerMesh->setAnimation(&Animations::characterIdle);
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

    mEngine->renderDevice()->setModelMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(2, 10, 0)));
    mPlayerMesh->addTime(1.0f / 60.0f);
    mPlayerMesh->render();
}
