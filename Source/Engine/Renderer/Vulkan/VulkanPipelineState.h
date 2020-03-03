#pragma once
#include "Engine/Renderer/IPipelineState.h"
#include "Engine/Renderer/Vulkan/VulkanCommon.h"

class VulkanRenderDevice;

class VulkanPipelineState : public IPipelineState
{
public:
    VulkanPipelineState(VulkanRenderDevice* device, VkPipelineLayout layout, VkPipeline pipeline);
    ~VulkanPipelineState();

private:
    VulkanRenderDevice* mDevice;
    VkPipelineLayout mLayout;
    VkPipeline mPipeline;
};
