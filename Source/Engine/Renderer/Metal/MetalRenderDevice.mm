#import "MetalRenderDevice.h"
#import "MetalRenderBuffer.h"
#import "MetalShaderProgram.h"
#import "Engine/Renderer/ShaderCode.h"

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

std::unique_ptr<IShaderProgram> MetalRenderDevice::createShaderProgram(const ShaderCode* code)
{
    dispatch_data_t data = dispatch_data_create(code->metal, code->metalSize,
        nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);

    NSError* error = nil;
    id<MTLLibrary> library = [mDevice newLibraryWithData:data error:&error];
    if (error != nil)
        NSLog(@"Unable to load shader: %@", error);

    return std::make_unique<MetalShaderProgram>(this, library);
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
