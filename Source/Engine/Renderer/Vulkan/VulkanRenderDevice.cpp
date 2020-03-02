#pragma once
#include "VulkanRenderDevice.h"
#include "VulkanRenderBuffer.h"
#include "VulkanPipelineState.h"
#include "VulkanTexture.h"
#include "VulkanShaderProgram.h"
#include "VulkanCommon.h"
#include "Engine/Renderer/VertexFormat.h"
#include "Engine/Renderer/TextureData.h"
#include "Engine/Renderer/ShaderCode.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat3x4.hpp>
#include <cassert>

VulkanRenderDevice::VulkanRenderDevice(/*MTKView* view*/)
    : mInitialized(false)
{
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    int presentQueueIndex;

    // Enumerate devices and queues

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(vulkanInstance, &physicalDeviceCount, nullptr);
    std::unique_ptr<VkPhysicalDevice[]> physicalDevices{new VkPhysicalDevice[physicalDeviceCount]};
    vkEnumeratePhysicalDevices(vulkanInstance, &physicalDeviceCount, physicalDevices.get());

    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        VkPhysicalDeviceProperties deviceProperties = {};
        vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, nullptr);
        std::unique_ptr<VkQueueFamilyProperties[]> queueFamilyProperties{new VkQueueFamilyProperties[queueFamilyCount]};
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, queueFamilyProperties.get());

        for (uint32_t j = 0; j < queueFamilyCount; ++j) {
            VkBool32 supportsPresent;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], j, vulkanSurface, &supportsPresent);
            if (supportsPresent && (queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                physicalDevice = physicalDevices[i];
                physicalDeviceProperties = deviceProperties;
                presentQueueIndex = j;
                break;
            }
        }

        if (physicalDevice)
            break;
    }

    if (!physicalDevice) {
        vulkanError("No physical Vulkan device found.");
        return;
    }

    // Create rendering device

    static const float queuePriorities[] = { 1.0f };
    static const char* deviceExtensions[] = { "VK_KHR_swapchain" };

    VkPhysicalDeviceFeatures features = {};
    features.shaderClipDistance = VK_TRUE;

    VkDeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.flags = 0;
    queueCreateInfo.queueFamilyIndex = presentQueueIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo deviceInfo;
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = nullptr;
    deviceInfo.flags = 0;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceInfo.enabledLayerCount = (vulkanHasValidationLayer ? 1 : 0);
    deviceInfo.ppEnabledLayerNames = (vulkanHasValidationLayer ? vulkanValidationLayer : nullptr);
    deviceInfo.enabledExtensionCount = 1;
    deviceInfo.ppEnabledExtensionNames = deviceExtensions;
    deviceInfo.pEnabledFeatures = &features;

    VkResult result = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &mDevice);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to create Vulkan device.");
        return;
    }

    // Select color format

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkanSurface, &formatCount, nullptr);
    std::unique_ptr<VkSurfaceFormatKHR[]> surfaceFormats{new VkSurfaceFormatKHR[formatCount]};
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkanSurface, &formatCount, surfaceFormats.get());

    VkColorSpaceKHR colorSpace = surfaceFormats[0].colorSpace;
    VkFormat colorFormat = (formatCount > 1 || surfaceFormats[0].format != VK_FORMAT_UNDEFINED ?
        surfaceFormats[0].format  : VK_FORMAT_B8G8R8_UNORM);

    // Determine surface capabilities

    VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vulkanSurface, &surfaceCapabilities);

    uint32_t desiredImageCount = 2;
    if (desiredImageCount < surfaceCapabilities.minImageCount)
        desiredImageCount = surfaceCapabilities.minImageCount;
    else if (surfaceCapabilities.maxImageCount != 0 && desiredImageCount > surfaceCapabilities.maxImageCount)
        desiredImageCount = surfaceCapabilities.maxImageCount;

    VkExtent2D surfaceResolution = surfaceCapabilities.currentExtent;
    if (surfaceResolution.width != -1) {
        mSurfaceWidth = surfaceResolution.width;
        mSurfaceHeight = surfaceResolution.height;
    } else {
        getVulkanWindowSize(&mSurfaceWidth, &mSurfaceHeight);
        surfaceResolution.width = mSurfaceWidth;
        surfaceResolution.height = mSurfaceHeight;
    }

    VkSurfaceTransformFlagBitsKHR preTransform = surfaceCapabilities.currentTransform;
    if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    // Select presentation mode

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkanSurface, &presentModeCount, nullptr);
    std::unique_ptr<VkPresentModeKHR[]> presentModes{new VkPresentModeKHR[presentModeCount]};
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkanSurface, &presentModeCount, presentModes.get());

    VkPresentModeKHR presentationMode = VK_PRESENT_MODE_FIFO_KHR; // always supported.
    for (uint32_t i = 0; i < presentModeCount; ++i) {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    // Create swap chain

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.pNext = nullptr;
    swapChainCreateInfo.flags = 0;
    swapChainCreateInfo.surface = vulkanSurface;
    swapChainCreateInfo.minImageCount = desiredImageCount;
    swapChainCreateInfo.imageFormat = colorFormat;
    swapChainCreateInfo.imageColorSpace = colorSpace;
    swapChainCreateInfo.imageExtent = surfaceResolution;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfo.queueFamilyIndexCount = 0;
    swapChainCreateInfo.pQueueFamilyIndices = nullptr;
    swapChainCreateInfo.preTransform = preTransform;
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCreateInfo.presentMode = presentationMode;
    swapChainCreateInfo.clipped = true;
    swapChainCreateInfo.oldSwapchain = nullptr;

    result = vkCreateSwapchainKHR(mDevice, &swapChainCreateInfo, nullptr, &mSwapChain);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to create Vulkan swap chain.");
        return;
    }

    // Create command pool

    vkGetDeviceQueue(mDevice, presentQueueIndex, 0, &mPresentQueue);

    VkCommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = presentQueueIndex;

    VkCommandPool commandPool;
    result = vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &commandPool);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to create command pool.");
        return;
    }

    // Create command buffers

    VkCommandBufferAllocateInfo commandBufferAllocationInfo;
    commandBufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocationInfo.pNext = nullptr;
    commandBufferAllocationInfo.commandPool = commandPool;
    commandBufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocationInfo.commandBufferCount = 1;

    result = vkAllocateCommandBuffers(mDevice, &commandBufferAllocationInfo, &mSetupCommandBuffer);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to create setup command buffer.");
        return;
    }

    result = vkAllocateCommandBuffers(mDevice, &commandBufferAllocationInfo, &mDrawCommandBuffer);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to create draw command buffer.");
        return;
    }

    // Get swap chain images

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
    mPresentImages.reset(new VkImage[imageCount]);
    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mPresentImages.get());

    VkImageViewCreateInfo presentImagesViewCreateInfo;
    presentImagesViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    presentImagesViewCreateInfo.pNext = nullptr;
    presentImagesViewCreateInfo.flags = 0;
    presentImagesViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    presentImagesViewCreateInfo.format = colorFormat;
    presentImagesViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    presentImagesViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    presentImagesViewCreateInfo.subresourceRange.baseMipLevel = 0;
    presentImagesViewCreateInfo.subresourceRange.levelCount = 1;
    presentImagesViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    presentImagesViewCreateInfo.subresourceRange.layerCount = 1;

    VkFenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = 0;

    VkFence submitFence;
    vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &submitFence);

    std::vector<bool> processed(imageCount);
    for (uint32_t processedCount = 0; processedCount < imageCount; ) {
        VkSemaphore presentCompleteSemaphore;
        VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
        vkCreateSemaphore(mDevice, &semaphoreCreateInfo, NULL, &presentCompleteSemaphore);

        uint32_t nextImageIndex = 0;
        vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, &nextImageIndex);

        if (!processed[nextImageIndex]) {
            VkCommandBufferBeginInfo beginInfo;
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.pNext = nullptr;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            beginInfo.pInheritanceInfo = nullptr;
            vkBeginCommandBuffer(mSetupCommandBuffer, &beginInfo);

            VkImageMemoryBarrier layoutTransitionBarrier;
            layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            layoutTransitionBarrier.pNext = nullptr;
            layoutTransitionBarrier.srcAccessMask = 0;
            layoutTransitionBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            layoutTransitionBarrier.image = mPresentImages[nextImageIndex];
            layoutTransitionBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            vkCmdPipelineBarrier(mSetupCommandBuffer,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                0, 0, nullptr, 0, nullptr, 1, &layoutTransitionBarrier);

            vkEndCommandBuffer(mSetupCommandBuffer);

            VkPipelineStageFlags waitStageMash[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            VkSubmitInfo submitInfo;
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.pNext = nullptr;
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
            submitInfo.pWaitDstStageMask = waitStageMash;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &mSetupCommandBuffer;
            submitInfo.signalSemaphoreCount = 0;
            submitInfo.pSignalSemaphores = nullptr;
            result = vkQueueSubmit(mPresentQueue, 1, &submitInfo, submitFence);

            vkWaitForFences(mDevice, 1, &submitFence, VK_TRUE, UINT64_MAX);
            vkResetFences(mDevice, 1, &submitFence);
            vkDestroySemaphore(mDevice, presentCompleteSemaphore, nullptr);
            vkResetCommandBuffer(mSetupCommandBuffer, 0);

            processed[nextImageIndex] = true;
            ++processedCount;
        }

        VkPresentInfoKHR presentInfo;
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.waitSemaphoreCount = 0;
        presentInfo.pWaitSemaphores = NULL;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &mSwapChain;
        presentInfo.pImageIndices = &nextImageIndex;
        presentInfo.pResults = nullptr;
        vkQueuePresentKHR(mPresentQueue, &presentInfo);
    }

    /*
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

    mInitialized = true;
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
