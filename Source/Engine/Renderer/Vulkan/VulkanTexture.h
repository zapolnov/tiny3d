#pragma once
#include "Engine/Renderer/ITexture.h"
#include "Engine/Renderer/Vulkan/VulkanCommon.h"

class VulkanRenderDevice;

class VulkanTexture : public ITexture
{
public:
    VulkanTexture(VulkanRenderDevice* device, VkImage texture, VkDeviceMemory textureMemory, VkImageView imageView, VkSampler sampler);
    ~VulkanTexture();

    VkImageView nativeImageView() const { return mImageView; }
    VkSampler nativeSampler() const { return mSampler; }

private:
    VulkanRenderDevice* mDevice;
    VkImage mTexture;
    VkDeviceMemory mTextureMemory;
    VkImageView mImageView;
    VkSampler mSampler;
};
