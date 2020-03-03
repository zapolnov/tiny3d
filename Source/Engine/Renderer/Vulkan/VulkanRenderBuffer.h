#pragma once
#include "Engine/Renderer/IRenderBuffer.h"
#include "Engine/Renderer/Vulkan/VulkanCommon.h"

class VulkanRenderDevice;

class VulkanRenderBuffer : public IRenderBuffer
{
public:
    VulkanRenderBuffer(VulkanRenderDevice* device, size_t size, uint32_t maxBuffersInFlight);
    VulkanRenderBuffer(VulkanRenderDevice* device, const void* data, size_t size);
    ~VulkanRenderBuffer();

    const VkBuffer& nativeBuffer() const { return mBuffer; }

    unsigned uploadData(const void* data) override;

private:
    VulkanRenderDevice* mDevice;
    VkBuffer mBuffer;
    VkDeviceMemory mDeviceMemory;
    size_t mSize;
    size_t mAlignedSize;
    uint32_t mMaxBuffersInFlight;

    void create(size_t size);
    void copyData(const void* data, unsigned offset, size_t size);
};
