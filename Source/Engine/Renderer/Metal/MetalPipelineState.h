#import "Engine/Renderer/IPipelineState.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

class MetalRenderDevice;

class MetalPipelineState : public IPipelineState
{
public:
    MetalPipelineState(MetalRenderDevice* device, id<MTLRenderPipelineState> state);
    ~MetalPipelineState();

    id<MTLRenderPipelineState> nativeState() const { return mState; }

private:
    MetalRenderDevice* mDevice;
    id<MTLRenderPipelineState> mState;
};
