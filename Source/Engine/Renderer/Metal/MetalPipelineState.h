#import "Engine/Renderer/IPipelineState.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

class MetalRenderDevice;

class MetalPipelineState : public IPipelineState
{
public:
    MetalPipelineState(MetalRenderDevice* device, PrimitiveType primitiveType, id<MTLRenderPipelineState> state);
    ~MetalPipelineState();

    id<MTLRenderPipelineState> nativeState() const { return mState; }
    PrimitiveType primitiveType() const { return mPrimitiveType; }

private:
    MetalRenderDevice* mDevice;
    id<MTLRenderPipelineState> mState;
    PrimitiveType mPrimitiveType;
};
