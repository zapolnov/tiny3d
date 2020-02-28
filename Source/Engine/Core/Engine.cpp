#include "Engine.h"
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Core/IGame.h"

Engine::Engine(IRenderDevice* renderDevice, std::function<IGame*(Engine*)> gameFactory)
    : mRenderDevice(renderDevice)
{
    mGame.reset(gameFactory(this));
}

Engine::~Engine()
{
}

void Engine::doOneFrame()
{
    if (mRenderDevice->beginFrame()) {
        mRenderDevice->endFrame();
    }
}
