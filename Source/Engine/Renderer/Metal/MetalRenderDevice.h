#import "Engine/Renderer/IRenderDevice.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

class MetalRenderDevice : public IRenderDevice
{
public:
    explicit MetalRenderDevice(MTKView* view);
    ~MetalRenderDevice();

    bool beginFrame() override;
    void endFrame() override;

private:
    MTKView* mView;
    id<MTLDevice> mDevice;
    id<MTLCommandQueue> mCommandQueue;
    id<MTLCommandBuffer> mCommandBuffer;
    id<MTLRenderCommandEncoder> mCommandEncoder;
};
