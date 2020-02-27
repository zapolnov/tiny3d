#pragma once

class IRenderDevice;

class Engine
{
public:
    explicit Engine(IRenderDevice* renderDevice);
    ~Engine();

    void doOneFrame();

private:
    IRenderDevice* mRenderDevice;
};
