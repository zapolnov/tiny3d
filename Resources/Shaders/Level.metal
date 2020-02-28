#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#import "ShaderTypes.h"

struct RasterizerData
{
    float4 position [[position]];
    float4 color;
};

vertex RasterizerData vertexShader(
    uint vertexID [[vertex_id]],
    constant Vertex* vertices [[buffer(VertexInputIndex_Vertices)]],
    constant vector_uint2* viewportSizePointer [[buffer(VertexInputIndex_ViewportSize)]]
    )
{
    float2 pixelSpacePosition = vertices[vertexID].position.xy;
    vector_float2 viewportSize = vector_float2(*viewportSizePointer);

    RasterizerData out;
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.xy = pixelSpacePosition / (viewportSize / 2.0);
    out.color = vector_float4(1.0, 0.0, 0.0, 1.0);

    return out;
}

fragment float4 fragmentShader(RasterizerData in [[stage_in]])
{
    return in.color;
}
