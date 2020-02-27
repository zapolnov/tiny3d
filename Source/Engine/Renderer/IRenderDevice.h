#pragma once

class IRenderDevice
{
public:
    virtual ~IRenderDevice() = default;
    virtual bool beginFrame() = 0;
    virtual void endFrame() = 0;
};
