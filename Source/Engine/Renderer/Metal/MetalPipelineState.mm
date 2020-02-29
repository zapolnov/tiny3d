#import "MetalPipelineState.h"

MetalPipelineState::MetalPipelineState(MetalRenderDevice* device, id<MTLRenderPipelineState> state)
    : mDevice(device)
    , mState(state)
{
}

MetalPipelineState::~MetalPipelineState()
{
}
