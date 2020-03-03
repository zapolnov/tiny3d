#pragma once
#include <cstddef>

struct ShaderCode
{
    const void* metal;
    size_t metalSize;
    const void* vulkanVertex;
    size_t vulkanVertexSize;
    const void* vulkanFragment;
    size_t vulkanFragmentSize;
};
