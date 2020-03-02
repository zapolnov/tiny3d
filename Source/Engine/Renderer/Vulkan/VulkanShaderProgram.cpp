#pragma once
#include "VulkanShaderProgram.h"
#include "VulkanRenderDevice.h"

VulkanShaderProgram::VulkanShaderProgram(VulkanRenderDevice* device/*, id<MTLLibrary> library*/)
    : mDevice(device)
    //, mLibrary(library)
{
    /*
    mVertexFunction = [mLibrary newFunctionWithName:@"vertexShader"];
    if (!mVertexFunction)
        NSLog(@"Vertex function was not found in shader library.");

    mFragmentFunction = [mLibrary newFunctionWithName:@"fragmentShader"];
    if (!mFragmentFunction)
        NSLog(@"Fragment function was not found in shader library.");
    */
}

VulkanShaderProgram::~VulkanShaderProgram()
{
}
