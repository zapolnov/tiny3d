#pragma once
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Renderer/Vulkan/VulkanCommon.h"
#include <memory>
#include <glm/mat3x4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

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
    struct VertexUniforms
    {
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::mat3x4 normalMatrix;
        glm::vec4 lightPosition;
    };

    struct FragmentUniforms
    {
        glm::vec4 ambientColor;
    };

    bool mInitialized;
    VkDevice mDevice;
    VkSwapchainKHR mSwapChain;
    VkQueue mPresentQueue;
    VkCommandPool mCommandPool;
    VkCommandBuffer mSetupCommandBuffer;
    VkCommandBuffer mDrawCommandBuffer;
    VkSemaphore mPresentCompleteSemaphore;
    VkSemaphore mRenderingCompleteSemaphore;
    VkFence mSubmitFence;
    VkImage mDepthImage;
    VkImageView mDepthImageView;
    VkRenderPass mRenderPass;
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
    VertexUniforms mVertexUniforms;
    FragmentUniforms mFragmentUniforms;
    std::unique_ptr<VkImage[]> mPresentImages;
    std::unique_ptr<VkFramebuffer[]> mFramebuffers;
    uint32_t mNextImageIndex;
    int mSurfaceWidth;
    int mSurfaceHeight;

    void bindUniforms();
};
