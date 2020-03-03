#pragma once
#include "VulkanTexture.h"
#include "VulkanRenderDevice.h"

VulkanTexture::VulkanTexture(VulkanRenderDevice* device, VkImage texture, VkDeviceMemory textureMemory, VkImageView imageView, VkSampler sampler)
    : mDevice(device)
    , mTexture(texture)
    , mTextureMemory(textureMemory)
    , mImageView(imageView)
    , mSampler(sampler)
{
}

VulkanTexture::~VulkanTexture()
{
    vkDestroySampler(mDevice->nativeDevice(), mSampler, nullptr);
    vkDestroyImageView(mDevice->nativeDevice(), mImageView, nullptr);
    vkDestroyImage(mDevice->nativeDevice(), mTexture, nullptr);
    vkFreeMemory(mDevice->nativeDevice(), mTextureMemory, nullptr);
}
