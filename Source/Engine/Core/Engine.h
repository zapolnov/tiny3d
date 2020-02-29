#pragma once
#include <functional>
#include <memory>

class IGame;
class IRenderDevice;
class ResourceManager;

class Engine
{
public:
    Engine(IRenderDevice* renderDevice, std::function<IGame*(Engine*)> gameFactory);
    ~Engine();

    IRenderDevice* renderDevice() const { return mRenderDevice; }
    ResourceManager* resourceManager() const { return mResourceManager.get(); }

    void doOneFrame();

private:
    IRenderDevice* mRenderDevice;
    std::unique_ptr<ResourceManager> mResourceManager;
    std::unique_ptr<IGame> mGame;
};
