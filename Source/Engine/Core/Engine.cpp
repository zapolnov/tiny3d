#include "Engine.h"
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Core/IGame.h"
#include "Engine/ResMgr/ResourceManager.h"

Engine::Engine(IRenderDevice* renderDevice, std::function<IGame*(Engine*)> gameFactory)
    : mRenderDevice(renderDevice)
{
    mResourceManager.reset(new ResourceManager(this));
    mGame.reset(gameFactory(this));
    mPrevTime = std::chrono::high_resolution_clock::now();
}

Engine::~Engine()
{
}

void Engine::doOneFrame()
{
    auto time = std::chrono::high_resolution_clock::now();
    auto deltaTime = time - mPrevTime;
    mPrevTime = time;
    float frameTime = std::chrono::duration<float>(deltaTime).count();

    mGame->update(frameTime);

    if (mRenderDevice->beginFrame()) {
        mGame->render();
        mRenderDevice->endFrame();
    }
}
