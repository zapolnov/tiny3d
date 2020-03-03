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

    unsigned uploadData(const void* data) override;

private:
    VulkanRenderDevice* mDevice;
    VkBuffer mBuffer;
    VkDeviceMemory mDeviceMemory;
    //dispatch_semaphore_t mSemaphore;
    size_t mSize;
    unsigned mBufferIndex;

    void create(size_t size);
    void copyData(const void* data, unsigned offset, size_t size);
};
