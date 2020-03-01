#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#import "ShaderTypes.h"

struct VertexInput
{
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float3 tangent [[attribute(2)]];
    float3 bitangent [[attribute(3)]];
    float2 texCoord [[attribute(4)]];
    float4 boneWeights [[attribute(5)]];
    uchar4 boneIndices [[attribute(6)]];
};

struct FragmentInput
{
    float4 position [[position]];
    float2 texCoord;
};

vertex FragmentInput vertexShader(
    VertexInput in [[stage_in]],
    constant float4x4* matrices [[buffer(VertexInputIndex_SkinningMatrices)]],
    constant CameraUniforms& cameraUniforms [[buffer(VertexInputIndex_CameraUniforms)]]
    )
{
    float4x4 viewProjectionMatrix = cameraUniforms.projectionMatrix * cameraUniforms.viewMatrix * cameraUniforms.modelMatrix;

    float4x4 boneTransform = matrices[in.boneIndices.x] * in.boneWeights.x;
    boneTransform += matrices[in.boneIndices.y] * in.boneWeights.y;
    boneTransform += matrices[in.boneIndices.z] * in.boneWeights.z;
    boneTransform += matrices[in.boneIndices.w] * in.boneWeights.w;

    FragmentInput out;
    out.position = viewProjectionMatrix * boneTransform * float4(in.position, 1.0);
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
