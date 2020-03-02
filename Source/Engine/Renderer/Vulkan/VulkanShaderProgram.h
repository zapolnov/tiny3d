#pragma once
#include "Engine/Renderer/IShaderProgram.h"

class VulkanRenderDevice;

class VulkanShaderProgram : public IShaderProgram
{
public:
    VulkanShaderProgram(VulkanRenderDevice* device/*, id<MTLLibrary> library*/);
    ~VulkanShaderProgram();

    /*
    id<MTLFunction> vertexFunction() const { return mVertexFunction; }
    id<MTLFunction> fragmentFunction() const { return mFragmentFunction; }
    */

private:
    VulkanRenderDevice* mDevice;
    /*
    id<MTLLibrary> mLibrary;
    id<MTLFunction> mVertexFunction;
    id<MTLFunction> mFragmentFunction;
    */
};
