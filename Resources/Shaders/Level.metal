#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#import "ShaderTypes.h"

struct VertexInput
{
    float3 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
};

struct FragmentInput
{
    float4 position [[position]];
    float2 texCoord;
};

vertex FragmentInput vertexShader(
    VertexInput in [[stage_in]],
    constant CameraUniforms& cameraUniforms [[buffer(VertexInputIndex_CameraUniforms)]]
    )
{
    float4x4 viewProjectionMatrix = cameraUniforms.projectionMatrix * cameraUniforms.viewMatrix * cameraUniforms.modelMatrix;

    FragmentInput out;
    out.position = viewProjectionMatrix * float4(in.position, 1.0);
    out.texCoord = in.texCoord;

    return out;
}

fragment float4 fragmentShader(
    FragmentInput in [[stage_in]],
    texture2d<float> texture [[texture(0)]]
    )
{
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear);
    return texture.sample(textureSampler, in.texCoord);
}
