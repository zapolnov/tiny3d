#include "Engine.h"
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Core/IGame.h"
#include "Engine/ResMgr/ResourceManager.h"

Engine::Engine(IRenderDevice* renderDevice, std::function<IGame*(Engine*)> gameFactory)
    : mRenderDevice(renderDevice)
{
    mResourceManager.reset(new ResourceManager(this));
    mGame.reset(gameFactory(this));
}

Engine::~Engine()
{
}

void Engine::doOneFrame()
{
    if (mRenderDevice->beginFrame()) {
        mGame->render();
        mRenderDevice->endFrame();
    }
}
