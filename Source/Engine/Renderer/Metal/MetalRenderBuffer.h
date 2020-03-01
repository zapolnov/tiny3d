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

    id<MTLBuffer> nativeBuffer() const { return mBuffer; }

    unsigned uploadData(const void* data) override;

private:
    MetalRenderDevice* mDevice;
    id<MTLBuffer> mBuffer;
    dispatch_semaphore_t mSemaphore;
    size_t mSize;
    size_t mAlignedSize;
    unsigned mBufferIndex;
};
