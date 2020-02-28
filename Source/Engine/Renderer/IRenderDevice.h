#pragma once
#include <memory>

struct ShaderCode;
class IRenderBuffer;
class IShaderProgram;

class IRenderDevice
{
public:
    virtual ~IRenderDevice() = default;

    virtual std::unique_ptr<IRenderBuffer> createBuffer(size_t size) = 0;
    virtual std::unique_ptr<IRenderBuffer> createBufferWithData(const void* data, size_t size) = 0;

    virtual std::unique_ptr<IShaderProgram> createShaderProgram(const ShaderCode* code) = 0;

    virtual bool beginFrame() = 0;
    virtual void endFrame() = 0;
};
