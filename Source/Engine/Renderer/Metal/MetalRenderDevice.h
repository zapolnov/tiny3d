#import "Engine/Renderer/IRenderDevice.h"
#import "Shaders/ShaderTypes.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

class MetalRenderDevice : public IRenderDevice
{
public:
    explicit MetalRenderDevice(MTKView* view);
    ~MetalRenderDevice();

    id<MTLDevice> nativeDevice() const { return mDevice; }
    id<MTLCommandBuffer> nativeCommandBuffer() const { return mCommandBuffer; }

    glm::vec2 viewportSize() const override;

    std::unique_ptr<IRenderBuffer> createBuffer(size_t size) override;
    std::unique_ptr<IRenderBuffer> createBufferWithData(const void* data, size_t size) override;
    std::unique_ptr<ITexture> createTexture(const TextureData* data) override;
    std::unique_ptr<IShaderProgram> createShaderProgram(const ShaderCode* code) override;
    std::unique_ptr<IPipelineState> createPipelineState(PrimitiveType primitiveType,
        const std::unique_ptr<IShaderProgram>& shader, const VertexFormat& vertexFormat) override;

    void setProjectionMatrix(const glm::mat4& matrix) override;
    void setViewMatrix(const glm::mat4& matrix) override;
    void setModelMatrix(const glm::mat4& matrix) override;

    void setTexture(int index, const std::unique_ptr<ITexture>& texture) override;
    void setPipelineState(const std::unique_ptr<IPipelineState>& state) override;
    void setVertexBuffer(int index, const std::unique_ptr<IRenderBuffer>& state, unsigned offset) override;

    void setLightPosition(const glm::vec3& position) override;
    void setAmbientColor(const glm::vec4& color) override;

    void drawPrimitive(unsigned start, unsigned count) override;
    void drawIndexedPrimitive(const std::unique_ptr<IRenderBuffer>& indexBuffer, unsigned start, unsigned count) override;

    void onDrawableSizeChanged(float width, float height);

    bool beginFrame() override;
    void endFrame() override;

private:
    MTKView* mView;
    id<MTLDevice> mDevice;
    id<MTLCommandQueue> mCommandQueue;
    id<MTLCommandBuffer> mCommandBuffer;
    id<MTLDepthStencilState> mDepthStencilState;
    id<MTLRenderCommandEncoder> mCommandEncoder;
    MTLPrimitiveType mPrimitiveType;
    VertexUniforms mVertexUniforms;
    FragmentUniforms mFragmentUniforms;
    MTLViewport mViewport;

    void bindUniforms();
};
