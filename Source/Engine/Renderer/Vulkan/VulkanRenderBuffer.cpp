#include "VulkanRenderBuffer.h"
#include "VulkanRenderDevice.h"

static const int MaxBuffersInFlight = 3;

VulkanRenderBuffer::VulkanRenderBuffer(VulkanRenderDevice* device, size_t size)
    : mDevice(device)
    , mSize(size)
    , mBufferIndex(0)
{
    create(mSize * MaxBuffersInFlight);
}

VulkanRenderBuffer::VulkanRenderBuffer(VulkanRenderDevice* device, const void* data, size_t size)
    : mDevice(device)
    , mSize(size)
    , mBufferIndex(0)
{
    create(mSize);
    copyData(data, 0, size);
}

VulkanRenderBuffer::~VulkanRenderBuffer()
{
    vkDestroyBuffer(mDevice->nativeDevice(), mBuffer, nullptr);
}

unsigned VulkanRenderBuffer::uploadData(const void* data)
{
    /* FIXME
    if (mSemaphore == nullptr)
        mSemaphore = dispatch_semaphore_create(MaxBuffersInFlight);

    dispatch_semaphore_wait(mSemaphore, DISPATCH_TIME_FOREVER);
    */

    mBufferIndex = (mBufferIndex + 1) % MaxBuffersInFlight;
    unsigned bufferOffset = mSize * mBufferIndex;

    /* FIXME
    __block dispatch_semaphore_t semaphore = mSemaphore;
    [mDevice->nativeCommandBuffer() addCompletedHandler:^(id<MTLCommandBuffer>) {
            dispatch_semaphore_signal(semaphore);
        }];

    */
    copyData(data, bufferOffset, mSize);

    return bufferOffset;
}

void VulkanRenderBuffer::create(size_t size)
{
    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
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
