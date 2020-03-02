#include "VulkanPipelineState.h"

VulkanPipelineState::VulkanPipelineState(VulkanRenderDevice* device)//, id<MTLRenderPipelineState> state)
    : mDevice(device)
    //, mState(state)
{
}

VulkanPipelineState::~VulkanPipelineState()
{
}
