#include "VulkanRenderBuffer.h"
#include "VulkanRenderDevice.h"

VulkanRenderBuffer::VulkanRenderBuffer(VulkanRenderDevice* device, size_t size, uint32_t maxBuffersInFlight)
    : mDevice(device)
    , mSize(size)
    , mAlignedSize((size + 255) & ~255)
    , mMaxBuffersInFlight(maxBuffersInFlight)
{
    create(mAlignedSize * mMaxBuffersInFlight);
}

VulkanRenderBuffer::VulkanRenderBuffer(VulkanRenderDevice* device, const void* data, size_t size)
    : mDevice(device)
    , mSize(size)
    , mAlignedSize((size + 255) & ~255)
    , mMaxBuffersInFlight(1)
{
    create(mSize);
    copyData(data, 0, size);
}

VulkanRenderBuffer::~VulkanRenderBuffer()
{
    vkDestroyBuffer(mDevice->nativeDevice(), mBuffer, nullptr);
    vkFreeMemory(mDevice->nativeDevice(), mDeviceMemory, nullptr);
}

unsigned VulkanRenderBuffer::uploadData(const void* data)
{
    unsigned bufferOffset = mAlignedSize * mDevice->currentBufferInFlight();
    copyData(data, bufferOffset, mSize);
    return bufferOffset;
}

void VulkanRenderBuffer::create(size_t size)
{
    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
               | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
               | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
               | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult result = vkCreateBuffer(mDevice->nativeDevice(), &info, nullptr, &mBuffer);
    assert(result == VK_SUCCESS); // FIXME: better error handling

    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(mDevice->nativeDevice(), mBuffer, &memoryRequirements);
    mDeviceMemory = mDevice->allocDeviceMemory(memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    result = vkBindBufferMemory(mDevice->nativeDevice(), mBuffer, mDeviceMemory, 0);
    assert(result == VK_SUCCESS); // FIXME: better error handling
}

void VulkanRenderBuffer::copyData(const void* data, unsigned offset, size_t size)
{
    void* mapped;
    VkResult result = vkMapMemory(mDevice->nativeDevice(), mDeviceMemory, offset, size, 0, &mapped);
    assert(result == VK_SUCCESS); // FIXME: better error handling

    memcpy(mapped, data, size);

    vkUnmapMemory(mDevice->nativeDevice(), mDeviceMemory);
}
