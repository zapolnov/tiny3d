#pragma once
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Renderer/Vulkan/VulkanCommon.h"
#include <memory>

class VulkanRenderDevice : public IRenderDevice
{
public:
    VulkanRenderDevice();
    ~VulkanRenderDevice();

    bool initialized() const { return mInitialized; }

    VkDevice nativeDevice() const { return mDevice; }

    glm::vec2 viewportSize() const override;

    uint32_t findDeviceMemory(const VkMemoryRequirements& memory, VkMemoryPropertyFlags desiredFlags) const;
    VkDeviceMemory allocDeviceMemory(const VkMemoryRequirements& memory, VkMemoryPropertyFlags desiredFlags);

    VkSemaphore createSemaphore();
    void destroySemaphore(VkSemaphore semaphore);

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

    bool beginFrame() override;
    void endFrame() override;

private:
    bool mInitialized;
    VkDevice mDevice;
    VkSwapchainKHR mSwapChain;
    VkQueue mPresentQueue;
    VkCommandBuffer mSetupCommandBuffer;
    VkCommandBuffer mDrawCommandBuffer;
    VkImage mDepthImage;
    VkImageView mDepthImageView;
    VkRenderPass mRenderPass;
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
    std::unique_ptr<VkImage[]> mPresentImages;
    std::unique_ptr<VkFramebuffer[]> mFramebuffers;
    int mSurfaceWidth;
    int mSurfaceHeight;
    /*
    MTKView* mView;
    id<MTLDevice> mDevice;
    id<MTLCommandQueue> mCommandQueue;
    id<MTLCommandBuffer> mCommandBuffer;
    id<MTLDepthStencilState> mDepthStencilState;
    id<MTLRenderCommandEncoder> mCommandEncoder;
    VertexUniforms mVertexUniforms;
    FragmentUniforms mFragmentUniforms;
    MTLViewport mViewport;
    */

    void bindUniforms();
};
