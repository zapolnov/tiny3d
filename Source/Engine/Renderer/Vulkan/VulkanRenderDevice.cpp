#pragma once
#include "VulkanRenderDevice.h"
#include "VulkanRenderBuffer.h"
#include "VulkanPipelineState.h"
#include "VulkanTexture.h"
#include "VulkanShaderProgram.h"
#include "Engine/Renderer/VertexFormat.h"
#include "Engine/Renderer/TextureData.h"
#include "Engine/Renderer/ShaderCode.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat3x4.hpp>
#include <cassert>

VulkanRenderDevice::VulkanRenderDevice(/*MTKView* view*/)
    //: mView(view)
    //, mViewport{0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}
{
    /*
    mDevice = view.device;
    mCommandQueue = [mDevice newCommandQueue];

    MTLDepthStencilDescriptor* depthDesc = [MTLDepthStencilDescriptor new];
    depthDesc.depthCompareFunction = MTLCompareFunctionLessEqual;
    depthDesc.depthWriteEnabled = YES;
    mDepthStencilState = [mDevice newDepthStencilStateWithDescriptor:depthDesc];

    const glm::mat4 identity4 = glm::mat4(1.0f);
    const glm::mat3x4 identity3 = glm::mat3x4(1.0f);
    memcpy(&mVertexUniforms.projectionMatrix, &identity4[0][0], 16 * sizeof(float));
    memcpy(&mVertexUniforms.viewMatrix, &identity4[0][0], 16 * sizeof(float));
    memcpy(&mVertexUniforms.modelMatrix, &identity4[0][0], 16 * sizeof(float));
    memcpy(&mVertexUniforms.normalMatrix, &identity3[0][0], 12 * sizeof(float));
    memset(&mVertexUniforms.lightPosition, 0, 3 * sizeof(float));

    const glm::vec4 ambient = glm::vec4(0.0f);
    memcpy(&mFragmentUniforms.ambientColor, &ambient[0], 4 * sizeof(float));
    */
}

VulkanRenderDevice::~VulkanRenderDevice()
{
}

glm::vec2 VulkanRenderDevice::viewportSize() const
{
    return glm::vec2(0, 0);//mViewport.width, mViewport.height);
}

std::unique_ptr<IRenderBuffer> VulkanRenderDevice::createBuffer(size_t size)
{
    return std::make_unique<VulkanRenderBuffer>(this, size);
}

std::unique_ptr<IRenderBuffer> VulkanRenderDevice::createBufferWithData(const void* data, size_t size)
{
    return std::make_unique<VulkanRenderBuffer>(this, data, size);
}

std::unique_ptr<ITexture> VulkanRenderDevice::createTexture(const TextureData* data)
{
    /*
    MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
    desc.pixelFormat = MTLPixelFormatRGBA8Unorm;
    desc.width = data->width;
    desc.height = data->height;

    id<MTLTexture> texture = [mDevice newTextureWithDescriptor:desc];

    MTLRegion region = { { 0, 0, 0 }, { data->width, data->height, 1 } };
    [texture replaceRegion:region mipmapLevel:0 withBytes:data->pixels bytesPerRow:(data->width * 4)];
    */

    return std::make_unique<VulkanTexture>(this/*, texture*/);
}

std::unique_ptr<IShaderProgram> VulkanRenderDevice::createShaderProgram(const ShaderCode* code)
{
    /*
    dispatch_data_t data = dispatch_data_create(code->metal, code->metalSize,
        nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);

    NSError* error = nil;
    id<MTLLibrary> library = [mDevice newLibraryWithData:data error:&error];
    if (error != nil)
        NSLog(@"Unable to load shader: %@", error);
    */

    return std::make_unique<VulkanShaderProgram>(this/*, library*/);
}

std::unique_ptr<IPipelineState> VulkanRenderDevice::createPipelineState(const std::unique_ptr<IShaderProgram>& shader, const VertexFormat& vertexFormat)
{
    assert(dynamic_cast<VulkanShaderProgram*>(shader.get()) != nullptr);
    auto vulkanShader = static_cast<VulkanShaderProgram*>(shader.get());

    /*
    MTLVertexDescriptor* vertexDesc = [MTLVertexDescriptor vertexDescriptor];
    int i = 0;
    for (const auto& attr : vertexFormat.attributes()) {
        switch (attr.type) {
            case VertexType::Float2: vertexDesc.attributes[i].format = MTLVertexFormatFloat2; break;
            case VertexType::Float3: vertexDesc.attributes[i].format = MTLVertexFormatFloat3; break;
            case VertexType::Float4: vertexDesc.attributes[i].format = MTLVertexFormatFloat4; break;
            case VertexType::UByte4: vertexDesc.attributes[i].format = MTLVertexFormatUChar4; break;
        }
        vertexDesc.attributes[i].bufferIndex = attr.bufferIndex;
        vertexDesc.attributes[i].offset = attr.offset;
        ++i;
    }
    for (unsigned layoutIndex = 0; layoutIndex < vertexFormat.bufferCount(); layoutIndex++)
        vertexDesc.layouts[layoutIndex].stride = vertexFormat.stride(layoutIndex);

    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.vertexDescriptor = vertexDesc;
    pipelineDesc.vertexFunction = vulkanShader->vertexFunction();
    pipelineDesc.fragmentFunction = vulkanShader->fragmentFunction();
    pipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth24Unorm_Stencil8;
    pipelineDesc.stencilAttachmentPixelFormat = MTLPixelFormatDepth24Unorm_Stencil8;
    pipelineDesc.colorAttachments[0].pixelFormat = mView.colorPixelFormat;

    NSError* error = nil;
    id<MTLRenderPipelineState> state = [mDevice newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    if (error != nil)
        NSLog(@"Unable to create pipeline state: %@", error);
    */

    return std::make_unique<VulkanPipelineState>(this/*, state*/);
}

void VulkanRenderDevice::setProjectionMatrix(const glm::mat4& matrix)
{
    //memcpy(&mVertexUniforms.projectionMatrix, &matrix[0][0], 16 * sizeof(float));
}

void VulkanRenderDevice::setViewMatrix(const glm::mat4& matrix)
{
    //memcpy(&mVertexUniforms.viewMatrix, &matrix[0][0], 16 * sizeof(float));
}

void VulkanRenderDevice::setModelMatrix(const glm::mat4& matrix)
{
    /*
    glm::mat3 normalMatrix = glm::mat3(matrix);
    glm::mat3x4 transposedInvertedMatrix = glm::mat3x4(glm::transpose(glm::inverse(normalMatrix)));
    memcpy(&mVertexUniforms.modelMatrix, &matrix[0][0], 16 * sizeof(float));
    memcpy(&mVertexUniforms.normalMatrix, &transposedInvertedMatrix[0][0], 12 * sizeof(float));
    */
}

void VulkanRenderDevice::setTexture(int index, const std::unique_ptr<ITexture>& texture)
{
    assert(dynamic_cast<VulkanTexture*>(texture.get()) != nullptr);
    auto vulkanTexture = static_cast<VulkanTexture*>(texture.get());

    //[mCommandEncoder setFragmentTexture:vulkanTexture->nativeTexture() atIndex:index];
}

void VulkanRenderDevice::setPipelineState(const std::unique_ptr<IPipelineState>& state)
{
    assert(dynamic_cast<VulkanPipelineState*>(state.get()) != nullptr);
    auto vulkanState = static_cast<VulkanPipelineState*>(state.get());

    //[mCommandEncoder setRenderPipelineState:vulkanState->nativeState()];
}

void VulkanRenderDevice::setVertexBuffer(int index, const std::unique_ptr<IRenderBuffer>& buffer, unsigned offset)
{
    assert(dynamic_cast<VulkanRenderBuffer*>(buffer.get()) != nullptr);
    auto vulkanBuffer = static_cast<VulkanRenderBuffer*>(buffer.get());

    //[mCommandEncoder setVertexBuffer:vulkanBuffer->nativeBuffer() offset:offset atIndex:index];
}

void VulkanRenderDevice::setLightPosition(const glm::vec3& position)
{
    //memcpy(&mVertexUniforms.lightPosition, &position[0], 3 * sizeof(float));
}

void VulkanRenderDevice::setAmbientColor(const glm::vec4& color)
{
    //memcpy(&mFragmentUniforms.ambientColor, &color[0], 4 * sizeof(float));
}

/*
static MTLPrimitiveType convertPrimitiveType(PrimitiveType type)
{
    switch (type) {
        case Triangles: return MTLPrimitiveTypeTriangle;
    }

    assert(false);
    return MTLPrimitiveTypeTriangle;
}
*/

void VulkanRenderDevice::drawPrimitive(PrimitiveType type, unsigned start, unsigned count)
{
    bindUniforms();
    //[mCommandEncoder drawPrimitives:convertPrimitiveType(type) vertexStart:start vertexCount:count];
}

void VulkanRenderDevice::drawIndexedPrimitive(PrimitiveType type, const std::unique_ptr<IRenderBuffer>& indexBuffer, unsigned start, unsigned count)
{
    assert(dynamic_cast<VulkanRenderBuffer*>(indexBuffer.get()) != nullptr);
    auto vulkanBuffer = static_cast<VulkanRenderBuffer*>(indexBuffer.get());

    bindUniforms();
    /*
    [mCommandEncoder drawIndexedPrimitives:convertPrimitiveType(type) indexCount:count
        indexType:MTLIndexTypeUInt16 indexBuffer:vulkanBuffer->nativeBuffer() indexBufferOffset:start
        instanceCount:1 baseVertex:0 baseInstance:0];
    */
}

void VulkanRenderDevice::onDrawableSizeChanged(float width, float height)
{
    /*
    mViewport.width = width;
    mViewport.height = height;
    */
}

bool VulkanRenderDevice::beginFrame()
{
    /*
    MTLRenderPassDescriptor* renderPassDescriptor = mView.currentRenderPassDescriptor;
    if (!renderPassDescriptor)
        return false;

    mCommandBuffer = [mCommandQueue commandBuffer];
    mCommandEncoder = [mCommandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

    [mCommandEncoder setViewport:mViewport];
    [mCommandEncoder setDepthStencilState:mDepthStencilState];
    */

    return true;
}

void VulkanRenderDevice::endFrame()
{
    /*
    [mCommandEncoder endEncoding];
    [mCommandBuffer presentDrawable:mView.currentDrawable];
    [mCommandBuffer commit];

    mCommandBuffer = nil;
    mCommandEncoder = nil;
    */
}

void VulkanRenderDevice::bindUniforms()
{
    /*
    [mCommandEncoder setVertexBytes:&mVertexUniforms
        length:sizeof(mVertexUniforms) atIndex:VertexInputIndex_VertexUniforms];
    [mCommandEncoder setFragmentBytes:&mFragmentUniforms
        length:sizeof(mFragmentUniforms) atIndex:VertexInputIndex_FragmentUniforms];
    */
}
