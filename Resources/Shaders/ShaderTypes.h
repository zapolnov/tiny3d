#pragma once
#include <simd/simd.h>

enum VertexInputIndex
{
    VertexInputIndex_Vertices = 0,
    VertexInputIndex_SkinningVertices,
    VertexInputIndex_SkinningMatrices,
    VertexInputIndex_VertexUniforms,
    VertexInputIndex_FragmentUniforms,
};

struct VertexUniforms
{
    simd::float4x4 modelMatrix;
    simd::float4x4 viewMatrix;
    simd::float4x4 projectionMatrix;
    simd::float3x3 normalMatrix;
    simd::float3 lightPosition;
};

struct FragmentUniforms
{
    simd::float4 ambientColor;
};
