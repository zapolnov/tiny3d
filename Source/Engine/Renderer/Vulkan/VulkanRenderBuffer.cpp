#include "VulkanRenderBuffer.h"
#include "VulkanRenderDevice.h"

static const int MaxBuffersInFlight = 3;

VulkanRenderBuffer::VulkanRenderBuffer(VulkanRenderDevice* device, size_t size)
    : mDevice(device)
    , mSize(size)
    , mAlignedSize((size + 255) & ~255)
    , mBufferIndex(0)
{
    //mBuffer = [device->nativeDevice() newBufferWithLength:(mAlignedSize * MaxBuffersInFlight) options:MTLResourceStorageModeShared];
}

VulkanRenderBuffer::VulkanRenderBuffer(VulkanRenderDevice* device, const void* data, size_t size)
    : mDevice(device)
    , mSize(size)
    , mAlignedSize((size + 255) & ~255)
    , mBufferIndex(0)
{
    //mBuffer = [device->nativeDevice() newBufferWithBytes:data length:size options:MTLResourceStorageModeManaged];

    VkBufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkResult result = vkCreateBuffer(mDevice->nativeDevice(), &info, nullptr, &mHandle);
    assert(result == VK_SUCCESS); // FIXME: better error handling

    VkMemoryRequirements memoryRequirements = {};
    vkGetBufferMemoryRequirements(mDevice->nativeDevice(), mHandle, &memoryRequirements);
    auto memory = mDevice->allocDeviceMemory(memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
}

VulkanRenderBuffer::~VulkanRenderBuffer()
{
    vkDestroyBuffer(mDevice->nativeDevice(), mHandle, nullptr);
}

unsigned VulkanRenderBuffer::uploadData(const void* data)
{
    /*
    if (mSemaphore == nullptr)
        mSemaphore = dispatch_semaphore_create(MaxBuffersInFlight);

    dispatch_semaphore_wait(mSemaphore, DISPATCH_TIME_FOREVER);
    */

    mBufferIndex = (mBufferIndex + 1) % MaxBuffersInFlight;
    unsigned bufferOffset = mAlignedSize * mBufferIndex;

    /*
    __block dispatch_semaphore_t semaphore = mSemaphore;
    [mDevice->nativeCommandBuffer() addCompletedHandler:^(id<MTLCommandBuffer>) {
            dispatch_semaphore_signal(semaphore);
        }];

    auto bufferPtr = reinterpret_cast<uint8_t*>(mBuffer.contents);
    memcpy(bufferPtr + bufferOffset, data, mSize);
    */

    return bufferOffset;
}
