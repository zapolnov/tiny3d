#pragma once
#include "Engine/Renderer/ITexture.h"

class VulkanRenderDevice;

class VulkanTexture : public ITexture
{
public:
    VulkanTexture(VulkanRenderDevice* device/*, id<MTLTexture> texture*/);
    ~VulkanTexture();

    //id<MTLTexture> nativeTexture() const { return mTexture; }

private:
    VulkanRenderDevice* mDevice;
    //id<MTLTexture> mTexture;
};
