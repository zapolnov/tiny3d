#pragma once
#include "Engine/Renderer/IPipelineState.h"
#include "Engine/Renderer/Vulkan/VulkanCommon.h"

class VulkanRenderDevice;

class VulkanPipelineState : public IPipelineState
{
public:
    VulkanPipelineState(VulkanRenderDevice* device, VkPipelineLayout layout, VkPipeline pipeline);
    ~VulkanPipelineState();

    VkPipelineLayout nativeLayout() const { return mLayout; }
    VkPipeline nativePipeline() const { return mPipeline; }

private:
    VulkanRenderDevice* mDevice;
    VkPipelineLayout mLayout;
    VkPipeline mPipeline;
};
