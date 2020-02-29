#import "MetalRenderDevice.h"
#import "MetalRenderBuffer.h"
#import "MetalPipelineState.h"
#import "MetalTexture.h"
#import "MetalShaderProgram.h"
#import "Engine/Renderer/VertexFormat.h"
#import "Engine/Renderer/TextureData.h"
#import "Engine/Renderer/ShaderCode.h"
#import <cassert>

MetalRenderDevice::MetalRenderDevice(MTKView* view)
    : mView(view)
    , mViewport{0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}
{
    mDevice = view.device;
    mCommandQueue = [mDevice newCommandQueue];

    const glm::mat4 identity = glm::mat4(1.0f);
    memcpy(&mCameraUniforms.projectionMatrix, &identity[0][0], 16 * sizeof(float));
    memcpy(&mCameraUniforms.viewMatrix, &identity[0][0], 16 * sizeof(float));
}

MetalRenderDevice::~MetalRenderDevice()
{
}

glm::vec2 MetalRenderDevice::viewportSize() const
{
    return glm::vec2(mViewport.width, mViewport.height);
}

std::unique_ptr<IRenderBuffer> MetalRenderDevice::createBuffer(size_t size)
{
    return std::make_unique<MetalRenderBuffer>(this, size);
}

std::unique_ptr<IRenderBuffer> MetalRenderDevice::createBufferWithData(const void* data, size_t size)
{
    return std::make_unique<MetalRenderBuffer>(this, data, size);
}

std::unique_ptr<ITexture> MetalRenderDevice::createTexture(const TextureData* data)
{
    MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
    desc.pixelFormat = MTLPixelFormatRGBA8Unorm;
    desc.width = data->width;
    desc.height = data->height;

    id<MTLTexture> texture = [mDevice newTextureWithDescriptor:desc];

    MTLRegion region = { { 0, 0, 0 }, { data->width, data->height, 1 } };
    [texture replaceRegion:region mipmapLevel:0 withBytes:data->pixels bytesPerRow:(data->width * 4)];

    return std::make_unique<MetalTexture>(this, texture);
}

std::unique_ptr<IShaderProgram> MetalRenderDevice::createShaderProgram(const ShaderCode* code)
{
    dispatch_data_t data = dispatch_data_create(code->metal, code->metalSize,
        nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);

    NSError* error = nil;
    id<MTLLibrary> library = [mDevice newLibraryWithData:data error:&error];
    if (error != nil)
        NSLog(@"Unable to load shader: %@", error);

    return std::make_unique<MetalShaderProgram>(this, library);
}

std::unique_ptr<IPipelineState> MetalRenderDevice::createPipelineState(const std::unique_ptr<IShaderProgram>& shader, const VertexFormat& vertexFormat)
{
    assert(dynamic_cast<MetalShaderProgram*>(shader.get()) != nullptr);
    auto metalShader = static_cast<MetalShaderProgram*>(shader.get());

    MTLVertexDescriptor* vertexDesc = [MTLVertexDescriptor vertexDescriptor];
    int i = 0;
    for (const auto& attr : vertexFormat.attributes()) {
        switch (attr.type) {
            case VertexType::Float2: vertexDesc.attributes[i].format = MTLVertexFormatFloat2; break;
            case VertexType::Float3: vertexDesc.attributes[i].format = MTLVertexFormatFloat3; break;
        }
        vertexDesc.attributes[i].bufferIndex = VertexInputIndex_Vertices;
        vertexDesc.attributes[i].offset = attr.offset;
        ++i;
    }
    vertexDesc.layouts[0].stride = vertexFormat.stride();

    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.vertexDescriptor = vertexDesc;
    pipelineDesc.vertexFunction = metalShader->vertexFunction();
    pipelineDesc.fragmentFunction = metalShader->fragmentFunction();
    pipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth24Unorm_Stencil8;
    pipelineDesc.stencilAttachmentPixelFormat = MTLPixelFormatDepth24Unorm_Stencil8;
    pipelineDesc.colorAttachments[0].pixelFormat = mView.colorPixelFormat;

    NSError* error = nil;
    id<MTLRenderPipelineState> state = [mDevice newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    if (error != nil)
        NSLog(@"Unable to create pipeline state: %@", error);

    return std::make_unique<MetalPipelineState>(this, state);
}

void MetalRenderDevice::setProjectionMatrix(const glm::mat4& matrix)
{
    memcpy(&mCameraUniforms.projectionMatrix, &matrix[0][0], 16 * sizeof(float));
}

void MetalRenderDevice::setViewMatrix(const glm::mat4& matrix)
{
    memcpy(&mCameraUniforms.viewMatrix, &matrix[0][0], 16 * sizeof(float));
}

void MetalRenderDevice::setTexture(int index, const std::unique_ptr<ITexture>& texture)
{
    assert(dynamic_cast<MetalTexture*>(texture.get()) != nullptr);
    auto metalTexture = static_cast<MetalTexture*>(texture.get());

    [mCommandEncoder setFragmentTexture:metalTexture->nativeTexture() atIndex:index];
}

void MetalRenderDevice::setPipelineState(const std::unique_ptr<IPipelineState>& state)
{
    assert(dynamic_cast<MetalPipelineState*>(state.get()) != nullptr);
    auto metalState = static_cast<MetalPipelineState*>(state.get());

    [mCommandEncoder setRenderPipelineState:metalState->nativeState()];
}

void MetalRenderDevice::setVertexBuffer(const std::unique_ptr<IRenderBuffer>& buffer, unsigned offset)
{
    assert(dynamic_cast<MetalRenderBuffer*>(buffer.get()) != nullptr);
    auto metalBuffer = static_cast<MetalRenderBuffer*>(buffer.get());

    [mCommandEncoder setVertexBuffer:metalBuffer->nativeBuffer() offset:offset atIndex:VertexInputIndex_Vertices];
}

static MTLPrimitiveType convertPrimitiveType(PrimitiveType type)
{
    switch (type) {
        case Triangles: return MTLPrimitiveTypeTriangle;
    }

    assert(false);
    return MTLPrimitiveTypeTriangle;
}

void MetalRenderDevice::drawPrimitive(PrimitiveType type, unsigned start, unsigned count)
{
    bindUniforms();
    [mCommandEncoder drawPrimitives:convertPrimitiveType(type) vertexStart:start vertexCount:count];
}

void MetalRenderDevice::drawIndexedPrimitive(PrimitiveType type, const std::unique_ptr<IRenderBuffer>& indexBuffer, unsigned start, unsigned count)
{
    assert(dynamic_cast<MetalRenderBuffer*>(indexBuffer.get()) != nullptr);
    auto metalBuffer = static_cast<MetalRenderBuffer*>(indexBuffer.get());

    bindUniforms();
    [mCommandEncoder drawIndexedPrimitives:convertPrimitiveType(type) indexCount:count
        indexType:MTLIndexTypeUInt16 indexBuffer:metalBuffer->nativeBuffer() indexBufferOffset:start
        instanceCount:1 baseVertex:0 baseInstance:0];
}

void MetalRenderDevice::onDrawableSizeChanged(float width, float height)
{
    mViewport.width = width;
    mViewport.height = height;
}

bool MetalRenderDevice::beginFrame()
{
    MTLRenderPassDescriptor* renderPassDescriptor = mView.currentRenderPassDescriptor;
    if (!renderPassDescriptor)
        return false;

    mCommandBuffer = [mCommandQueue commandBuffer];
    mCommandEncoder = [mCommandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

    [mCommandEncoder setViewport:mViewport];

    return true;
}

void MetalRenderDevice::endFrame()
{
    [mCommandEncoder endEncoding];
    [mCommandBuffer presentDrawable:mView.currentDrawable];
    [mCommandBuffer commit];

    mCommandBuffer = nil;
    mCommandEncoder = nil;
}

void MetalRenderDevice::bindUniforms()
{
    [mCommandEncoder setVertexBytes:&mCameraUniforms
        length:sizeof(mCameraUniforms) atIndex:VertexInputIndex_CameraUniforms];
}
