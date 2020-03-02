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
}

VulkanRenderBuffer::~VulkanRenderBuffer()
{
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
