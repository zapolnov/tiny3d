#pragma once
#include <functional>
#include <memory>
#include <chrono>

class IGame;
class IRenderDevice;
class InputManager;
class ResourceManager;

class Engine
{
public:
    Engine(IRenderDevice* renderDevice, std::function<IGame*(Engine*)> gameFactory);
    ~Engine();

    IRenderDevice* renderDevice() const { return mRenderDevice; }
    ResourceManager* resourceManager() const { return mResourceManager.get(); }
    InputManager* inputManager() const { return mInputManager.get(); }

    void doOneFrame();

private:
    IRenderDevice* mRenderDevice;
    std::unique_ptr<InputManager> mInputManager;
    std::unique_ptr<ResourceManager> mResourceManager;
    std::unique_ptr<IGame> mGame;
    std::chrono::time_point<std::chrono::high_resolution_clock> mPrevTime;
};
