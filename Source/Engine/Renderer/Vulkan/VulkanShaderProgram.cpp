#pragma once
#include "VulkanShaderProgram.h"
#include "VulkanRenderDevice.h"

VulkanShaderProgram::VulkanShaderProgram(VulkanRenderDevice* device, VkShaderModule vertex, VkShaderModule fragment)
    : mDevice(device)
    , mVertex(vertex)
    , mFragment(fragment)
{
}

VulkanShaderProgram::~VulkanShaderProgram()
{
    vkDestroyShaderModule(mDevice->nativeDevice(), mVertex, nullptr);
    vkDestroyShaderModule(mDevice->nativeDevice(), mFragment, nullptr);
}
