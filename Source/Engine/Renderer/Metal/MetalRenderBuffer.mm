#import "MetalRenderBuffer.h"
#import "MetalRenderDevice.h"

MetalRenderBuffer::MetalRenderBuffer(MetalRenderDevice* device, size_t size)
    : mDevice(device)
{
    mBuffer = [device->nativeDevice() newBufferWithLength:size options:MTLResourceStorageModeShared];
}

MetalRenderBuffer::MetalRenderBuffer(MetalRenderDevice* device, const void* data, size_t size)
{
    mBuffer = [device->nativeDevice() newBufferWithBytes:data length:size options:MTLResourceStorageModeManaged];
}

MetalRenderBuffer::~MetalRenderBuffer()
{
}
