#import "Engine/Renderer/IRenderDevice.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

class MetalRenderDevice : public IRenderDevice
{
public:
    explicit MetalRenderDevice(MTKView* view);
    ~MetalRenderDevice();

    id<MTLDevice> nativeDevice() const { return mDevice; }

    std::unique_ptr<IRenderBuffer> createBuffer(size_t size) override;
    std::unique_ptr<IRenderBuffer> createBufferWithData(const void* data, size_t size) override;

    bool beginFrame() override;
    void endFrame() override;

private:
    MTKView* mView;
    id<MTLDevice> mDevice;
    id<MTLCommandQueue> mCommandQueue;
    id<MTLCommandBuffer> mCommandBuffer;
    id<MTLRenderCommandEncoder> mCommandEncoder;
};
