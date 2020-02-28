#import "MetalRenderDevice.h"
#import "MetalRenderBuffer.h"

MetalRenderDevice::MetalRenderDevice(MTKView* view)
    : mView(view)
{
    mDevice = view.device;
    mCommandQueue = [mDevice newCommandQueue];
}

MetalRenderDevice::~MetalRenderDevice()
{
}

std::unique_ptr<IRenderBuffer> MetalRenderDevice::createBuffer(size_t size)
{
    return std::make_unique<MetalRenderBuffer>(this, size);
}

std::unique_ptr<IRenderBuffer> MetalRenderDevice::createBufferWithData(const void* data, size_t size)
{
    return std::make_unique<MetalRenderBuffer>(this, data, size);
}

bool MetalRenderDevice::beginFrame()
{
    MTLRenderPassDescriptor* renderPassDescriptor = mView.currentRenderPassDescriptor;
    if (!renderPassDescriptor)
        return false;

    mCommandBuffer = [mCommandQueue commandBuffer];
    mCommandEncoder = [mCommandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

    return true;
}

void MetalRenderDevice::endFrame()
{
    [mCommandEncoder endEncoding];
    [mCommandBuffer presentDrawable:mView.currentDrawable];
    [mCommandBuffer commit];

    mCommandBuffer = nil;
    mCommandEncoder = nil;
}
