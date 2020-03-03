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

VulkanRenderDevice::VulkanRenderDevice()
    : mInitialized(false)
    , mDevice(nullptr)
    , mSwapChain(nullptr)
    , mPresentQueue(nullptr)
    , mCommandPool(nullptr)
    , mSetupCommandBuffer(nullptr)
    , mDrawCommandBuffer(nullptr)
    , mPresentCompleteSemaphore(nullptr)
    , mRenderingCompleteSemaphore(nullptr)
    , mSubmitFence(nullptr)
    , mDepthImage(nullptr)
    , mDepthImageView(nullptr)
    , mRenderPass(nullptr)
    , mDescriptorSetLayout(nullptr)
    , mDescriptorPool(nullptr)
    , mCurrentPipelineLayout(nullptr)
    , mCurrentImageView(nullptr)
    , mCurrentSampler(nullptr)
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

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &mMemoryProperties);

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

    result = vkCreateCommandPool(mDevice, &commandPoolCreateInfo, nullptr, &mCommandPool);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to create command pool.");
        return;
    }

    // Create command buffers

    VkCommandBufferAllocateInfo commandBufferAllocationInfo;
    commandBufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocationInfo.pNext = nullptr;
    commandBufferAllocationInfo.commandPool = mCommandPool;
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

    mImageCount = 0;
    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &mImageCount, nullptr);
    mPresentImages.reset(new VkImage[mImageCount]);
    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &mImageCount, mPresentImages.get());

    VkFenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = 0;

    vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mSubmitFence);

    std::vector<bool> processed(mImageCount);
    for (uint32_t processedCount = 0; processedCount < mImageCount; ) {
        VkSemaphore presentCompleteSemaphore = createSemaphore();

        uint32_t nextImageIndex = 0;
        vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, &nextImageIndex);

        if (processed[nextImageIndex])
            destroySemaphore(presentCompleteSemaphore);
        else {
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
            result = vkQueueSubmit(mPresentQueue, 1, &submitInfo, mSubmitFence);

            vkWaitForFences(mDevice, 1, &mSubmitFence, VK_TRUE, UINT64_MAX);
            vkResetFences(mDevice, 1, &mSubmitFence);
            destroySemaphore(presentCompleteSemaphore);
            vkResetCommandBuffer(mSetupCommandBuffer, 0);

            processed[nextImageIndex] = true;
            ++processedCount;
        }

        VkPresentInfoKHR presentInfo;
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;
        presentInfo.waitSemaphoreCount = 0;
        presentInfo.pWaitSemaphores = nullptr;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &mSwapChain;
        presentInfo.pImageIndices = &nextImageIndex;
        presentInfo.pResults = nullptr;
        vkQueuePresentKHR(mPresentQueue, &presentInfo);
    }

    std::unique_ptr<VkImageView[]> presentImageViews{new VkImageView[mImageCount]};
    for (uint32_t i = 0; i < mImageCount; ++i) {
        VkImageViewCreateInfo presentImagesViewCreateInfo;
        presentImagesViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        presentImagesViewCreateInfo.pNext = nullptr;
        presentImagesViewCreateInfo.flags = 0;
        presentImagesViewCreateInfo.image = mPresentImages[i];
        presentImagesViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        presentImagesViewCreateInfo.format = colorFormat;
        presentImagesViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        presentImagesViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        presentImagesViewCreateInfo.subresourceRange.baseMipLevel = 0;
        presentImagesViewCreateInfo.subresourceRange.levelCount = 1;
        presentImagesViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        presentImagesViewCreateInfo.subresourceRange.layerCount = 1;
                presentImagesViewCreateInfo.image = mPresentImages[i];

        result = vkCreateImageView(mDevice, &presentImagesViewCreateInfo, nullptr, &presentImageViews[i]);
        if (result != VK_SUCCESS) {
            vulkanError("Unable to create view for swap chain image.");
            return;
        }
    }

    // Setup depth buffer

    VkImageCreateInfo imageCreateInfo;
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = nullptr;
    imageCreateInfo.flags = 0;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = VK_FORMAT_D16_UNORM;
    imageCreateInfo.extent = { uint32_t(mSurfaceWidth), uint32_t(mSurfaceHeight), 1 };
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.queueFamilyIndexCount = 0;
    imageCreateInfo.pQueueFamilyIndices = nullptr;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    result = vkCreateImage(mDevice, &imageCreateInfo, nullptr, &mDepthImage);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to create depth image.");
        return;
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(mDevice, mDepthImage, &memoryRequirements);
    auto imageMemory = allocDeviceMemory(memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    result = vkBindImageMemory(mDevice, mDepthImage, imageMemory, 0);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to bind memory for depth image.");
        return;
    }

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
    layoutTransitionBarrier.dstAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutTransitionBarrier.image = mDepthImage;
    VkImageSubresourceRange resourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
    layoutTransitionBarrier.subresourceRange = resourceRange;

    vkCmdPipelineBarrier(mSetupCommandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 1, &layoutTransitionBarrier);

    vkEndCommandBuffer(mSetupCommandBuffer);

    VkPipelineStageFlags waitStageMask[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = waitStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mSetupCommandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    result = vkQueueSubmit(mPresentQueue, 1, &submitInfo, mSubmitFence);

    vkWaitForFences(mDevice, 1, &mSubmitFence, VK_TRUE, UINT64_MAX);
    vkResetFences(mDevice, 1, &mSubmitFence);
    vkResetCommandBuffer(mSetupCommandBuffer, 0);

    VkImageViewCreateInfo imageViewCreateInfo;
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = nullptr;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.image = mDepthImage;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = imageCreateInfo.format;
    imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                       VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    result = vkCreateImageView(mDevice, &imageViewCreateInfo, nullptr, &mDepthImageView);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to create view for depth image.");
        return;
    }

    // Create render pass

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentReference = {};
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;
    subpass.pDepthStencilAttachment = &depthAttachmentReference;

    VkAttachmentDescription passAttachments[2] = {};
    passAttachments[0].format = colorFormat;
    passAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    passAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    passAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    passAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    passAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    passAttachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    passAttachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    passAttachments[1].format = VK_FORMAT_D16_UNORM;
    passAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    passAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    passAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    passAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    passAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    passAttachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    passAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 2;
    renderPassCreateInfo.pAttachments = passAttachments;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpass;

    result = vkCreateRenderPass(mDevice, &renderPassCreateInfo, nullptr, &mRenderPass);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to create render pass.");
        return;
    }

    // Create framebuffer

    VkImageView frameBufferAttachments[2];
    frameBufferAttachments[1] = mDepthImageView;

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.renderPass = mRenderPass;
    frameBufferCreateInfo.attachmentCount = 2;
    frameBufferCreateInfo.pAttachments = frameBufferAttachments;
    frameBufferCreateInfo.width = mSurfaceWidth;
    frameBufferCreateInfo.height = mSurfaceHeight;
    frameBufferCreateInfo.layers = 1;

    mFramebuffers.reset(new VkFramebuffer[mImageCount]);
    for (uint32_t i = 0; i < mImageCount; ++i) {
        frameBufferAttachments[0] = presentImageViews[i];
        result = vkCreateFramebuffer(mDevice, &frameBufferCreateInfo, nullptr, &mFramebuffers[i]);
        if (result != VK_SUCCESS) {
            vulkanError("Unable to create framebuffer.");
            return;
        }
    }

    // Create descriptor set layouts

    VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[2] = {};
    descriptorSetLayoutBindings[0].binding = 0;
    descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorSetLayoutBindings[0].descriptorCount = 1;
    descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorSetLayoutBindings[1].binding = 1;
    descriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorSetLayoutBindings[1].descriptorCount = 1;
    descriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 2;
    layoutInfo.pBindings = descriptorSetLayoutBindings;

    result = vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to create descriptor set layout for vertex uniforms.");
        return;
    }

    // Create descriptor sets

    VkDescriptorPoolSize poolSizes[2] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = mImageCount;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = mImageCount;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = mImageCount;

    result = vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to create descriptor pool.");
        return;
    }

    std::vector<VkDescriptorSetLayout> layouts(mImageCount, mDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.descriptorSetCount = mImageCount;
    allocInfo.pSetLayouts = layouts.data();

    mDescriptorSets.resize(mImageCount);
    result = vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data());
    if (result != VK_SUCCESS) {
        vulkanError("Unable to allocate descriptor sets.");
        return;
    }

    // Setup initial uniform values

    mVertexUniforms.projectionMatrix = glm::mat4(1.0f);
    mVertexUniforms.viewMatrix = glm::mat4(1.0f);
    mVertexUniforms.modelMatrix = glm::mat4(1.0f);
    mVertexUniforms.normalMatrix = glm::mat3x4(1.0f);
    mVertexUniforms.lightPosition = glm::vec4(0.0f);

    mFragmentUniforms.ambientColor = glm::vec4(0.0f);

    mInitialized = true;
}

VulkanRenderDevice::~VulkanRenderDevice()
{
    mUsedVertexUniformBuffers.clear();
    mFreeVertexUniformBuffers.clear();

    if (mRenderPass)
        vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
    if (mDescriptorPool)
        vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
    if (mDescriptorSetLayout)
        vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
    if (mSubmitFence)
        vkDestroyFence(mDevice, mSubmitFence, nullptr);
    if (mDepthImageView)
        vkDestroyImageView(mDevice, mDepthImageView, nullptr);
    if (mDepthImage)
        vkDestroyImage(mDevice, mDepthImage, nullptr);
    if (mDrawCommandBuffer)
        vkFreeCommandBuffers(mDevice, mCommandPool, 1, &mDrawCommandBuffer);
    if (mSetupCommandBuffer)
        vkFreeCommandBuffers(mDevice, mCommandPool, 1, &mSetupCommandBuffer);
    if (mCommandPool)
        vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
    if (mSwapChain)
        vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
    if (mPresentCompleteSemaphore)
        vkDestroySemaphore(mDevice, mPresentCompleteSemaphore, nullptr);
    if (mRenderingCompleteSemaphore)
        vkDestroySemaphore(mDevice, mRenderingCompleteSemaphore, nullptr);
    if (mDevice)
        vkDestroyDevice(mDevice, nullptr);
}

glm::vec2 VulkanRenderDevice::viewportSize() const
{
    return glm::vec2(mSurfaceWidth, mSurfaceHeight);
}

uint32_t VulkanRenderDevice::findDeviceMemory(const VkMemoryRequirements& memory, VkMemoryPropertyFlags desiredFlags) const
{
    for (uint32_t i = 0; i < 32; ++i) {
        if (memory.memoryTypeBits & (1 << i)) {
            if ((mMemoryProperties.memoryTypes[i].propertyFlags & desiredFlags) == desiredFlags)
                return i;
        }
    }

    assert(false);
    return 0;
}

VkDeviceMemory VulkanRenderDevice::allocDeviceMemory(const VkMemoryRequirements& memory, VkMemoryPropertyFlags desiredFlags)
{
    VkMemoryAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.allocationSize = memory.size;
    allocInfo.memoryTypeIndex = findDeviceMemory(memory, desiredFlags);

    VkDeviceMemory allocatedMemory = nullptr;
    VkResult result = vkAllocateMemory(mDevice, &allocInfo, nullptr, &allocatedMemory);
    assert(result == VK_SUCCESS);   // FIXME: better error handling

    return allocatedMemory;
}

VkSemaphore VulkanRenderDevice::createSemaphore()
{
    VkSemaphoreCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;

    VkSemaphore semaphore = nullptr;
    VkResult result = vkCreateSemaphore(mDevice, &info, nullptr, &semaphore);
    assert(result == VK_SUCCESS);   // FIXME: better error handling

    return semaphore;
}

void VulkanRenderDevice::destroySemaphore(VkSemaphore semaphore)
{
    vkDestroySemaphore(mDevice, semaphore, nullptr);
}

std::unique_ptr<IRenderBuffer> VulkanRenderDevice::createBuffer(size_t size)
{
    return std::make_unique<VulkanRenderBuffer>(this, size, mImageCount);
}

std::unique_ptr<IRenderBuffer> VulkanRenderDevice::createBufferWithData(const void* data, size_t size)
{
    return std::make_unique<VulkanRenderBuffer>(this, data, size);
}

std::unique_ptr<ITexture> VulkanRenderDevice::createTexture(const TextureData* data)
{
    VulkanRenderBuffer stagingBuffer(this, data->pixels, data->width * data->height * 4);

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = uint32_t(data->width);
    imageInfo.extent.height = uint32_t(data->height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VkImage texture;
    VkResult result = vkCreateImage(mDevice, &imageInfo, nullptr, &texture);
    assert(result == VK_SUCCESS);   // FIXME: better error handling

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(mDevice, texture, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findDeviceMemory(memRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory textureMemory;
    result = vkAllocateMemory(mDevice, &allocInfo, nullptr, &textureMemory);
    assert(result == VK_SUCCESS);   // FIXME: better error handling

    vkBindImageMemory(mDevice, texture, textureMemory, 0);

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;
    vkBeginCommandBuffer(mSetupCommandBuffer, &beginInfo);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { data->width, data->height, 1 };
    vkCmdCopyBufferToImage(mSetupCommandBuffer, stagingBuffer.nativeBuffer(), texture,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    vkEndCommandBuffer(mSetupCommandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mSetupCommandBuffer;
    result = vkQueueSubmit(mPresentQueue, 1, &submitInfo, mSubmitFence);
    assert(result == VK_SUCCESS);   // FIXME: better error handling

    vkWaitForFences(mDevice, 1, &mSubmitFence, VK_TRUE, UINT64_MAX);
    vkResetFences(mDevice, 1, &mSubmitFence);
    vkResetCommandBuffer(mSetupCommandBuffer, 0);

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = texture;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    result = vkCreateImageView(mDevice, &viewInfo, nullptr, &imageView);
    assert(result == VK_SUCCESS);   // FIXME: better error handling

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    VkSampler sampler;
    result = vkCreateSampler(mDevice, &samplerInfo, nullptr, &sampler);
    assert(result == VK_SUCCESS);   // FIXME: better error handling

    return std::make_unique<VulkanTexture>(this, texture, textureMemory, imageView, sampler);
}

std::unique_ptr<IShaderProgram> VulkanRenderDevice::createShaderProgram(const ShaderCode* code)
{
    VkShaderModuleCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = code->vulkanVertexSize;
    info.pCode = reinterpret_cast<const uint32_t*>(code->vulkanVertex);

    VkShaderModule vertexShader;
    VkResult result = vkCreateShaderModule(mDevice, &info, nullptr, &vertexShader);
    assert(result == VK_SUCCESS);   // FIXME: better error handling

    info.codeSize = code->vulkanFragmentSize;
    info.pCode = reinterpret_cast<const uint32_t*>(code->vulkanFragment);

    VkShaderModule fragmentShader;
    result = vkCreateShaderModule(mDevice, &info, nullptr, &fragmentShader);
    assert(result == VK_SUCCESS);   // FIXME: better error handling

    return std::make_unique<VulkanShaderProgram>(this, vertexShader, fragmentShader);
}

std::unique_ptr<IPipelineState> VulkanRenderDevice::createPipelineState(PrimitiveType primitiveType,
    const std::unique_ptr<IShaderProgram>& shader, const VertexFormat& vertexFormat)
{
    assert(dynamic_cast<VulkanShaderProgram*>(shader.get()) != nullptr);
    auto vulkanShader = static_cast<VulkanShaderProgram*>(shader.get());

    int i = 0;
    std::vector<VkVertexInputAttributeDescription> attributes;
    for (const auto& attr : vertexFormat.attributes()) {
        VkVertexInputAttributeDescription desc = {};
        switch (attr.type) {
            case VertexType::Float2: desc.format = VK_FORMAT_R32G32_SFLOAT; break;
            case VertexType::Float3: desc.format = VK_FORMAT_R32G32B32_SFLOAT; break;
            case VertexType::Float4: desc.format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
            case VertexType::UByte4: desc.format = VK_FORMAT_R8G8B8A8_UINT; break;
        }
        desc.location = i;
        desc.binding = attr.bufferIndex;
        desc.offset = attr.offset;
        attributes.emplace_back(std::move(desc));
        ++i;
    }

    std::vector<VkVertexInputBindingDescription> bindings;
    for (unsigned bindingIndex = 0; bindingIndex < vertexFormat.bufferCount(); bindingIndex++) {
        VkVertexInputBindingDescription desc = {};
        desc.binding = bindingIndex;
        desc.stride = vertexFormat.stride(bindingIndex);
        desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindings.emplace_back(std::move(desc));
    }

    VkPipelineLayoutCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.setLayoutCount = 1;
    info.pSetLayouts = &mDescriptorSetLayout;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;

    VkPipelineLayout pipelineLayout;
    VkResult result = vkCreatePipelineLayout(mDevice, &info, nullptr, &pipelineLayout);
    assert(result == VK_SUCCESS);   // FIXME: better error handling

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};
    shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageCreateInfo[0].module = vulkanShader->vertex();
    shaderStageCreateInfo[0].pName = "main";
    shaderStageCreateInfo[0].pSpecializationInfo = nullptr;
    shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStageCreateInfo[1].module = vulkanShader->fragment();
    shaderStageCreateInfo[1].pName = "main";
    shaderStageCreateInfo[1].pSpecializationInfo = nullptr;

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = bindings.size();
    vertexInputStateCreateInfo.pVertexBindingDescriptions = bindings.data();
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributes.size();
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    switch (primitiveType) {
        case Triangles: inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
    }
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = mSurfaceHeight;
    viewport.width = mSurfaceWidth;
    viewport.height = -mSurfaceHeight;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    VkRect2D scissors = {};
    scissors.offset = { 0, 0 };
    scissors.extent = { uint32_t(mSurfaceWidth), uint32_t(mSurfaceHeight) };

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissors;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.depthBiasConstantFactor = 0;
    rasterizationState.depthBiasClamp = 0;
    rasterizationState.depthBiasSlopeFactor = 0;
    rasterizationState.lineWidth = 1;

    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.minSampleShading = 0;
    multisampleState.pSampleMask = nullptr;
    multisampleState.alphaToCoverageEnable = VK_FALSE;
    multisampleState.alphaToOneEnable = VK_FALSE;

    VkStencilOpState noOPStencilState = {};
    noOPStencilState.failOp = VK_STENCIL_OP_KEEP;
    noOPStencilState.passOp = VK_STENCIL_OP_KEEP;
    noOPStencilState.depthFailOp = VK_STENCIL_OP_KEEP;
    noOPStencilState.compareOp = VK_COMPARE_OP_ALWAYS;
    noOPStencilState.compareMask = 0;
    noOPStencilState.writeMask = 0;
    noOPStencilState.reference = 0;

    VkPipelineDepthStencilStateCreateInfo depthState = {};
    depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthState.depthTestEnable = VK_TRUE;
    depthState.depthWriteEnable = VK_TRUE;
    depthState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthState.depthBoundsTestEnable = VK_FALSE;
    depthState.stencilTestEnable = VK_FALSE;
    depthState.front = noOPStencilState;
    depthState.back = noOPStencilState;
    depthState.minDepthBounds = 0;
    depthState.maxDepthBounds = 0;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.blendEnable = VK_FALSE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.colorWriteMask = 0xf;

    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_CLEAR;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachmentState;
    colorBlendState.blendConstants[0] = 0.0;
    colorBlendState.blendConstants[1] = 0.0;
    colorBlendState.blendConstants[2] = 0.0;
    colorBlendState.blendConstants[3] = 0.0;

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = 0;
    dynamicStateCreateInfo.pDynamicStates = nullptr;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStageCreateInfo;
    pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    pipelineCreateInfo.pTessellationState = nullptr;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pDepthStencilState = &depthState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = mRenderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = nullptr;
    pipelineCreateInfo.basePipelineIndex = 0;

    VkPipeline pipeline;
    result = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
    assert(result == VK_SUCCESS);   // FIXME: better error handling

    return std::make_unique<VulkanPipelineState>(this, pipelineLayout, pipeline);
}

void VulkanRenderDevice::setProjectionMatrix(const glm::mat4& matrix)
{
    mVertexUniforms.projectionMatrix = matrix;
}

void VulkanRenderDevice::setViewMatrix(const glm::mat4& matrix)
{
    mVertexUniforms.viewMatrix = matrix;
}

void VulkanRenderDevice::setModelMatrix(const glm::mat4& matrix)
{
    mVertexUniforms.modelMatrix = matrix;
    mVertexUniforms.normalMatrix = glm::mat3x4(glm::transpose(glm::inverse(glm::mat3(matrix))));
}

void VulkanRenderDevice::setTexture(int index, const std::unique_ptr<ITexture>& texture)
{
    assert(dynamic_cast<VulkanTexture*>(texture.get()) != nullptr);
    auto vulkanTexture = static_cast<VulkanTexture*>(texture.get());

    if (index == 0) {
        mCurrentImageView = vulkanTexture->nativeImageView();
        mCurrentSampler = vulkanTexture->nativeSampler();
    }
}

void VulkanRenderDevice::setPipelineState(const std::unique_ptr<IPipelineState>& state)
{
    assert(dynamic_cast<VulkanPipelineState*>(state.get()) != nullptr);
    auto vulkanState = static_cast<VulkanPipelineState*>(state.get());

    mCurrentPipelineLayout = vulkanState->nativeLayout();
    vkCmdBindPipeline(mDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanState->nativePipeline());
}

void VulkanRenderDevice::setVertexBuffer(int index, const std::unique_ptr<IRenderBuffer>& buffer, unsigned offset)
{
    assert(dynamic_cast<VulkanRenderBuffer*>(buffer.get()) != nullptr);
    auto vulkanBuffer = static_cast<VulkanRenderBuffer*>(buffer.get());

    VkDeviceSize offsets = offset;
    vkCmdBindVertexBuffers(mDrawCommandBuffer, index, 1, &vulkanBuffer->nativeBuffer(), &offsets);
}

void VulkanRenderDevice::setLightPosition(const glm::vec3& position)
{
    mVertexUniforms.lightPosition = glm::vec4(position, 0.0f);
}

void VulkanRenderDevice::setAmbientColor(const glm::vec4& color)
{
    mFragmentUniforms.ambientColor = color;
}

void VulkanRenderDevice::drawPrimitive(unsigned start, unsigned count)
{
    bindUniforms();
    vkCmdDraw(mDrawCommandBuffer, count, 1, start, 0);
}

void VulkanRenderDevice::drawIndexedPrimitive(const std::unique_ptr<IRenderBuffer>& indexBuffer, unsigned start, unsigned count)
{
    assert(dynamic_cast<VulkanRenderBuffer*>(indexBuffer.get()) != nullptr);
    auto vulkanBuffer = static_cast<VulkanRenderBuffer*>(indexBuffer.get());

    vkCmdBindIndexBuffer(mDrawCommandBuffer, vulkanBuffer->nativeBuffer(), 0, VK_INDEX_TYPE_UINT16);

    bindUniforms();
    vkCmdDrawIndexed(mDrawCommandBuffer, count, 1, start, 0, 0);
}

bool VulkanRenderDevice::beginFrame()
{
    VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
    vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mPresentCompleteSemaphore);
    vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr, &mRenderingCompleteSemaphore);

    mNextImageIndex = 0;
    vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mPresentCompleteSemaphore, VK_NULL_HANDLE, &mNextImageIndex);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(mDrawCommandBuffer, &beginInfo);

    VkImageMemoryBarrier layoutTransitionBarrier = {};
    layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    layoutTransitionBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    layoutTransitionBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutTransitionBarrier.image = mPresentImages[mNextImageIndex];
    layoutTransitionBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    vkCmdPipelineBarrier(mDrawCommandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 1, &layoutTransitionBarrier);

    VkClearValue clearValue[] = { { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0, 0.0 } };
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = mRenderPass;
    renderPassBeginInfo.framebuffer = mFramebuffers[mNextImageIndex];
    renderPassBeginInfo.renderArea = { 0, 0, uint32_t(mSurfaceWidth), uint32_t(mSurfaceHeight) };
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValue;
    vkCmdBeginRenderPass(mDrawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    return true;
}

void VulkanRenderDevice::endFrame()
{
    vkCmdEndRenderPass(mDrawCommandBuffer);

    VkImageMemoryBarrier prePresentBarrier = {};
    prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    prePresentBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    prePresentBarrier.image = mPresentImages[mNextImageIndex];

    vkCmdPipelineBarrier(mDrawCommandBuffer,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 1, &prePresentBarrier);

    vkEndCommandBuffer(mDrawCommandBuffer);

    VkFence renderFence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &renderFence );

    VkSubmitInfo submitInfo = {};
    VkPipelineStageFlags waitStageMask = { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &mPresentCompleteSemaphore;
    submitInfo.pWaitDstStageMask = &waitStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mDrawCommandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &mRenderingCompleteSemaphore;
    vkQueueSubmit(mPresentQueue, 1, &submitInfo, renderFence);

    vkWaitForFences(mDevice, 1, &renderFence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(mDevice, renderFence, nullptr);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &mRenderingCompleteSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mSwapChain;
    presentInfo.pImageIndices = &mNextImageIndex;
    presentInfo.pResults = nullptr;
    vkQueuePresentKHR(mPresentQueue, &presentInfo);

    vkDestroySemaphore(mDevice, mPresentCompleteSemaphore, nullptr);
    vkDestroySemaphore(mDevice, mRenderingCompleteSemaphore, nullptr);
    mPresentCompleteSemaphore = nullptr;
    mRenderingCompleteSemaphore = nullptr;

    while (!mUsedVertexUniformBuffers.empty()) {
        auto buffer = std::move(mUsedVertexUniformBuffers.back());
        mUsedVertexUniformBuffers.pop_back();
        mFreeVertexUniformBuffers.emplace_back(std::move(buffer));
    }
}

void VulkanRenderDevice::bindUniforms()
{
    std::unique_ptr<VulkanRenderBuffer> uniformBuffer;
    if (mFreeVertexUniformBuffers.empty())
        uniformBuffer = std::make_unique<VulkanRenderBuffer>(this, sizeof(mVertexUniforms), mImageCount);
    else {
        uniformBuffer = std::move(mFreeVertexUniformBuffers.back());
        mFreeVertexUniformBuffers.pop_back();
    }

    unsigned offset = uniformBuffer->uploadData(&mVertexUniforms);

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = uniformBuffer->nativeBuffer();
    bufferInfo.offset = offset;
    bufferInfo.range = sizeof(mVertexUniforms);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = mCurrentImageView;
    imageInfo.sampler = mCurrentSampler;

    VkWriteDescriptorSet descriptorWrites[2] = {};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = mDescriptorSets[mNextImageIndex];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = mDescriptorSets[mNextImageIndex];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(mDevice, 2, descriptorWrites, 0, nullptr);

    vkCmdBindDescriptorSets(mDrawCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        mCurrentPipelineLayout, 0, 1, &mDescriptorSets[mNextImageIndex], 0, nullptr);

    mUsedVertexUniformBuffers.emplace_back(std::move(uniformBuffer));
}
