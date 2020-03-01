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
};

struct FragmentInput
{
    float4 position [[position]];
    float3 normal;
    float3 lightDirection;
    float lightDistance;
    float2 texCoord;
};

vertex FragmentInput vertexShader(
    VertexInput in [[stage_in]],
    constant VertexUniforms& uniforms [[buffer(VertexInputIndex_VertexUniforms)]]
    )
{
    float4 position = uniforms.modelMatrix * float4(in.position, 1.0);

    float3 tangent = normalize(uniforms.normalMatrix * in.tangent);
    float3 bitangent = normalize(uniforms.normalMatrix * in.bitangent);
    float3 normal = normalize(uniforms.normalMatrix * in.normal);
    float3x3 tbn = float3x3(
            float3(tangent.x, bitangent.x, normal.x),
            float3(tangent.y, bitangent.y, normal.y),
            float3(tangent.z, bitangent.z, normal.z)
        );

    float3 lightDirection = uniforms.lightPosition - float3(position);
    float lightDistance = length(lightDirection);
    lightDirection /= lightDistance;

    FragmentInput out;
    out.position = uniforms.projectionMatrix * uniforms.viewMatrix * position;
    out.normal = tbn * normal;
    out.lightDirection = tbn * lightDirection;
    out.lightDistance = lightDistance;
    out.texCoord = in.texCoord;

    return out;
}

fragment float4 fragmentShader(
    FragmentInput in [[stage_in]],
    texture2d<float> texture [[texture(0)]],
    texture2d<float> normalMap [[texture(1)]],
    constant FragmentUniforms& uniforms [[buffer(VertexInputIndex_FragmentUniforms)]]
    )
{
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear);
    float4 color = texture.sample(textureSampler, in.texCoord);
    float3 normal = normalize(normalMap.sample(textureSampler, in.texCoord).rgb * 2.0 - 1.0);

    float intensity = saturate(dot(normal, normalize(in.lightDirection)));
    float attenuation = 0.5 * in.lightDistance;
    intensity = min(intensity / attenuation, 1.2);

    return max(intensity * color, uniforms.ambientColor);
}
