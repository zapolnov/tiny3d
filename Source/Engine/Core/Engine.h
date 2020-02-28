#pragma once
#include <functional>
#include <memory>

class IGame;
class IRenderDevice;

class Engine
{
public:
    Engine(IRenderDevice* renderDevice, std::function<IGame*(Engine*)> gameFactory);
    ~Engine();

    IRenderDevice* renderDevice() const { return mRenderDevice; }

    void doOneFrame();

private:
    IRenderDevice* mRenderDevice;
    std::unique_ptr<IGame> mGame;
};
