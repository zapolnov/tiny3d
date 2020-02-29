#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#import "ShaderTypes.h"

struct VertexInput
{
    float3 position [[attribute(0)]];
};

struct FragmentInput
{
    float4 position [[position]];
    float4 color;
};

vertex FragmentInput vertexShader(
    VertexInput in [[stage_in]],
    constant CameraUniforms& cameraUniforms [[buffer(VertexInputIndex_CameraUniforms)]]
    )
{
    float4x4 viewProjectionMatrix = cameraUniforms.projectionMatrix * cameraUniforms.viewMatrix;

    FragmentInput out;
    out.position = viewProjectionMatrix * float4(in.position, 1.0);
    out.color = vector_float4(1.0, 0.0, 0.0, 1.0);

    return out;
}

fragment float4 fragmentShader(
    FragmentInput in [[stage_in]]
    )
{
    return in.color;
}
