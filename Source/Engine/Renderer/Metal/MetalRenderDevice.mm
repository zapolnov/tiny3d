#import "MetalRenderDevice.h"

MetalRenderDevice::MetalRenderDevice(MTKView* view)
    : mView(view)
{
    mDevice = view.device;
    mCommandQueue = [mDevice newCommandQueue];
}

MetalRenderDevice::~MetalRenderDevice()
{
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
