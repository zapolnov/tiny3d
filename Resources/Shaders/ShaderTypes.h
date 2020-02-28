#pragma once
#include <simd/simd.h>

enum VertexInputIndex
{
    VertexInputIndex_Vertices = 0,
    VertexInputIndex_ViewportSize = 1,
};

struct Vertex
{
    vector_float3 position;
};
