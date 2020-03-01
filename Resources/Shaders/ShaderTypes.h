#pragma once
#include <simd/simd.h>

enum VertexInputIndex
{
    VertexInputIndex_Vertices = 0,
    VertexInputIndex_SkinningVertices,
    VertexInputIndex_SkinningMatrices,
    VertexInputIndex_CameraUniforms,
};

struct CameraUniforms
{
    simd::float4x4 modelMatrix;
    simd::float4x4 viewMatrix;
    simd::float4x4 projectionMatrix;
};
