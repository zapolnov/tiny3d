#pragma once
#include "VulkanTexture.h"
#include "VulkanRenderDevice.h"

VulkanTexture::VulkanTexture(VulkanRenderDevice* device/*, id<MTLTexture> texture*/)
    : mDevice(device)
    //, mTexture(texture)
{
}

VulkanTexture::~VulkanTexture()
{
}
