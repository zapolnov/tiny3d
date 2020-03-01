#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#import "ShaderTypes.h"

struct VertexInput
{
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
};

struct FragmentInput
{
    float4 position [[position]];
    float3 normal;
    float3 lightDirection;
    float2 texCoord;
};

vertex FragmentInput vertexShader(
    VertexInput in [[stage_in]],
    constant VertexUniforms& uniforms [[buffer(VertexInputIndex_VertexUniforms)]]
    )
{
    float4 position = uniforms.modelMatrix * float4(in.position, 1.0);

    FragmentInput out;
    out.position = uniforms.projectionMatrix * uniforms.viewMatrix * position;
    out.normal = uniforms.normalMatrix * in.normal;
    out.lightDirection = uniforms.lightPosition - float3(position);
    out.texCoord = in.texCoord;

    return out;
}

fragment float4 fragmentShader(
    FragmentInput in [[stage_in]],
    texture2d<float> texture [[texture(0)]],
    constant FragmentUniforms& uniforms [[buffer(VertexInputIndex_FragmentUniforms)]]
    )
{
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear);
    float4 color = texture.sample(textureSampler, in.texCoord) * float4(1.0, 1.0, 0.7, 1.0);

    float3 lightDirection = in.lightDirection;
    float lightDistance = length(lightDirection);
    lightDirection /= lightDistance;

    float intensity = saturate(dot(normalize(in.normal), lightDirection));
    float attenuation = 0.5 * lightDistance;
    intensity = min(intensity / attenuation, 1.2);

    return max(intensity * color, uniforms.ambientColor);
}
