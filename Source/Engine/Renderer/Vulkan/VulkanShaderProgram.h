#pragma once
#include "Engine/Renderer/IShaderProgram.h"
#include "Engine/Renderer/Vulkan/VulkanCommon.h"

class VulkanRenderDevice;

class VulkanShaderProgram : public IShaderProgram
{
public:
    VulkanShaderProgram(VulkanRenderDevice* device, VkShaderModule vertex, VkShaderModule fragment);
    ~VulkanShaderProgram();

    VkShaderModule vertex() const { return mVertex; }
    VkShaderModule fragment() const { return mFragment; }

private:
    VulkanRenderDevice* mDevice;
    VkShaderModule mVertex;
    VkShaderModule mFragment;
};
