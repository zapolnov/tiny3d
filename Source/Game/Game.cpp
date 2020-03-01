#include "Game.h"
#include "Resources/Compiled/Levels.h"
#include "Resources/Compiled/Animations.h"
#include "Resources/Compiled/Meshes.h"
#include "Engine/Core/Engine.h"
#include "Engine/Input/InputManager.h"
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Mesh/AnimatedMesh.h"
#include "Engine/ResMgr/ResourceManager.h"
#include <glm/gtc/matrix_transform.hpp>

static const float PlayerSpeed = 10.0f;

Game::Game(Engine* engine)
    : mEngine(engine)
    , mPlayerMoving(false)
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
    if (mPlayerMoving) {
        float step = PlayerSpeed * frameTime;

        glm::vec3 vector = mPlayerTarget - mPlayerPos;
        float dist = glm::length(vector);
        glm::vec3 dir = vector / dist;

        if (step < dist)
            mPlayerPos += dir * step;
        else {
            mPlayerPos = mPlayerTarget;
            mPlayerMoving = false;
        }
    }

    if (!mPlayerMoving) {
        glm::ivec2 pos = glm::ivec2(int(mPlayerPos.x), LevelHeight - 1 - int(mPlayerPos.y));
        bool horz = false;
        if (mEngine->inputManager()->isKeyPressed(KeyLeft) && mLevel->isWalkable(pos.x + 1, pos.y)) {
            mPlayerTarget += glm::vec3(1.0f, 0.0f, 0.0f);
            mPlayerMoving = true;
            mPlayerRotation = 90.0f;
            horz = true;
        }
        if (mEngine->inputManager()->isKeyPressed(KeyRight) && mLevel->isWalkable(pos.x - 1, pos.y)) {
            mPlayerTarget += glm::vec3(-1.0f, 0.0f, 0.0f);
            mPlayerMoving = true;
            mPlayerRotation = -90.0f;
            horz = true;
        }

        if (!horz) {
            if (mEngine->inputManager()->isKeyPressed(KeyUp) && mLevel->isWalkable(pos.x, pos.y + 1)) {
                mPlayerTarget += glm::vec3(0.0f, -1.0f, 0.0f);
                mPlayerRotation = 0.0f;
                mPlayerMoving = true;
            }
            if (mEngine->inputManager()->isKeyPressed(KeyDown) && mLevel->isWalkable(pos.x, pos.y - 1)) {
                mPlayerTarget += glm::vec3(0.0f, 1.0f, 0.0f);
                mPlayerRotation = 180.0f;
                mPlayerMoving = true;
            }
        }
    }

    if (mPlayerMoving)
        mPlayerMesh->setAnimation(&Animations::characterRun);
    else
        mPlayerMesh->setAnimation(&Animations::characterIdle);

    mPlayerMesh->addTime(frameTime);

    mCamera.setSize(mEngine->renderDevice()->viewportSize());
    mCamera.setUpVector(glm::vec3(0.0f, -1.0f, 0.0f));
    mCamera.setPosition(mPlayerPos + glm::vec3(0.0f, 4.0f, 6.0f));
    mCamera.setTarget(mPlayerPos);
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
    m = glm::rotate(m, mPlayerRotation * 3.1415f / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    m = glm::scale(m, glm::vec3(0.005f, 0.0025f, 0.005f));
    mEngine->renderDevice()->setModelMatrix(m);
    mPlayerMesh->render();
}

void Game::loadLevel(const LevelData* level)
{
    mLevel = std::make_unique<Level>(mEngine, level);
    mPlayerPos = glm::vec3(level->playerX, level->playerY, 0.0f);
    mPlayerTarget = mPlayerPos;
    mPlayerRotation = 0.0f;
    mPlayerMoving = false;
}
