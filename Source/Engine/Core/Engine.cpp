#include "Engine.h"
#include "Engine/Renderer/IRenderDevice.h"

Engine::Engine(IRenderDevice* renderDevice)
    : mRenderDevice(renderDevice)
{
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
