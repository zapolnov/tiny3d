#pragma once
#include <glm/mat4x4.hpp>
#include <memory>

struct ShaderCode;
class IRenderBuffer;
class IPipelineState;
class IShaderProgram;

enum PrimitiveType
{
    Triangles,
};

class IRenderDevice
{
public:
    virtual ~IRenderDevice() = default;

    virtual std::unique_ptr<IRenderBuffer> createBuffer(size_t size) = 0;
    virtual std::unique_ptr<IRenderBuffer> createBufferWithData(const void* data, size_t size) = 0;

    virtual std::unique_ptr<IShaderProgram> createShaderProgram(const ShaderCode* code) = 0;

    virtual std::unique_ptr<IPipelineState> createPipelineState(const std::unique_ptr<IShaderProgram>& shader) = 0;

    virtual void setProjectionMatrix(const glm::mat4& matrix) = 0;
    virtual void setViewMatrix(const glm::mat4& matrix) = 0;

    virtual void setPipelineState(const std::unique_ptr<IPipelineState>& state) = 0;
    virtual void setVertexBuffer(const std::unique_ptr<IRenderBuffer>& buffer, unsigned offset = 0) = 0;

    virtual void drawPrimitive(PrimitiveType type, unsigned start, unsigned count) = 0;
    virtual void drawIndexedPrimitive(PrimitiveType type, const std::unique_ptr<IRenderBuffer>& indexBuffer, unsigned start, unsigned count) = 0;

    virtual bool beginFrame() = 0;
    virtual void endFrame() = 0;
};
