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
    constant CameraUniforms& cameraUniforms [[buffer(VertexInputIndex_CameraUniforms)]],
    constant Vertex* vertices [[buffer(VertexInputIndex_Vertices)]]
    )
{
    float4 inPosition = float4(vertices[vertexID].position, 1.0);
    float4x4 viewProjectionMatrix = cameraUniforms.projectionMatrix * cameraUniforms.viewMatrix;

    RasterizerData out;
    out.position = viewProjectionMatrix * inPosition;
    out.color = vector_float4(1.0, 0.0, 0.0, 1.0);

    return out;
}

fragment float4 fragmentShader(
    RasterizerData in [[stage_in]]
    )
{
    return in.color;
}
