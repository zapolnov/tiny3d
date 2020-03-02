#pragma once
#include "Engine/Renderer/IRenderBuffer.h"
#include "Engine/Renderer/Vulkan/VulkanCommon.h"

class VulkanRenderDevice;

class VulkanRenderBuffer : public IRenderBuffer
{
public:
    VulkanRenderBuffer(VulkanRenderDevice* device, size_t size);
    VulkanRenderBuffer(VulkanRenderDevice* device, const void* data, size_t size);
    ~VulkanRenderBuffer();

    //id<MTLBuffer> nativeBuffer() const { return mBuffer; }

    unsigned uploadData(const void* data) override;

private:
    VulkanRenderDevice* mDevice;
    VkBuffer mHandle;
    //dispatch_semaphore_t mSemaphore;
    size_t mSize;
    size_t mAlignedSize;
    unsigned mBufferIndex;
};
