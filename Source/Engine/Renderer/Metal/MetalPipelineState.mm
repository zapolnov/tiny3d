#import "MetalPipelineState.h"

MetalPipelineState::MetalPipelineState(MetalRenderDevice* device, PrimitiveType primitiveType, id<MTLRenderPipelineState> state)
    : mDevice(device)
    , mState(state)
    , mPrimitiveType(primitiveType)
{
}

MetalPipelineState::~MetalPipelineState()
{
}
