#pragma once
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <memory>

struct ShaderCode;
struct TextureData;
class VertexFormat;
class IRenderBuffer;
class ITexture;
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

    virtual glm::vec2 viewportSize() const = 0;

    virtual std::unique_ptr<IRenderBuffer> createBuffer(size_t size) = 0;
    virtual std::unique_ptr<IRenderBuffer> createBufferWithData(const void* data, size_t size) = 0;
    virtual std::unique_ptr<ITexture> createTexture(const TextureData* data) = 0;
    virtual std::unique_ptr<IShaderProgram> createShaderProgram(const ShaderCode* code) = 0;
    virtual std::unique_ptr<IPipelineState> createPipelineState(PrimitiveType primitiveType,
        const std::unique_ptr<IShaderProgram>& shader, const VertexFormat& vertexFormat) = 0;

    virtual void setProjectionMatrix(const glm::mat4& matrix) = 0;
    virtual void setViewMatrix(const glm::mat4& matrix) = 0;
    virtual void setModelMatrix(const glm::mat4& matrix) = 0;

    virtual void setTexture(int index, const std::unique_ptr<ITexture>& texture) = 0;
    virtual void setPipelineState(const std::unique_ptr<IPipelineState>& state) = 0;
    virtual void setVertexBuffer(int index, const std::unique_ptr<IRenderBuffer>& buffer, unsigned offset = 0) = 0;

    virtual void setLightPosition(const glm::vec3& position) = 0;
    virtual void setAmbientColor(const glm::vec4& color) = 0;

    virtual void drawPrimitive(unsigned start, unsigned count) = 0;
    virtual void drawIndexedPrimitive(const std::unique_ptr<IRenderBuffer>& indexBuffer, unsigned start, unsigned count) = 0;

    virtual bool beginFrame() = 0;
    virtual void endFrame() = 0;
};
