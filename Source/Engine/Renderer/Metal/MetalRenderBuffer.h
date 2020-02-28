#import "Engine/Renderer/IRenderBuffer.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

class MetalRenderDevice;

class MetalRenderBuffer : public IRenderBuffer
{
public:
    MetalRenderBuffer(MetalRenderDevice* device, size_t size);
    MetalRenderBuffer(MetalRenderDevice* device, const void* data, size_t size);
    ~MetalRenderBuffer();

private:
    MetalRenderDevice* mDevice;
    id<MTLBuffer> mBuffer;
};
