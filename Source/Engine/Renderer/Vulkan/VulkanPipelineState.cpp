#include "VulkanPipelineState.h"
#include "Engine/Renderer/Vulkan/VulkanRenderDevice.h"

VulkanPipelineState::VulkanPipelineState(VulkanRenderDevice* device, VkPipelineLayout layout, VkPipeline pipeline)
    : mDevice(device)
    , mLayout(layout)
    , mPipeline(pipeline)
{
}

VulkanPipelineState::~VulkanPipelineState()
{
    vkDestroyPipelineLayout(mDevice->nativeDevice(), mLayout, nullptr);
    vkDestroyPipeline(mDevice->nativeDevice(), mPipeline, nullptr);
}
