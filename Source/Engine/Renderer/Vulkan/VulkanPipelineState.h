#pragma once
#include "Engine/Renderer/IPipelineState.h"

class VulkanRenderDevice;

class VulkanPipelineState : public IPipelineState
{
public:
    VulkanPipelineState(VulkanRenderDevice* device);//, id<MTLRenderPipelineState> state);
    ~VulkanPipelineState();

    //id<MTLRenderPipelineState> nativeState() const { return mState; }

private:
    VulkanRenderDevice* mDevice;
    //id<MTLRenderPipelineState> mState;
};
