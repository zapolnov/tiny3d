#include "VulkanCommon.h"
#include <memory>

bool vulkanHasValidationLayer;
VkInstance vulkanInstance;
VkSurfaceKHR vulkanSurface;

PFN_vkCreateInstance vkCreateInstance;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkCreateDevice vkCreateDevice;
PFN_vkGetDeviceQueue vkGetDeviceQueue;
PFN_vkCreateCommandPool vkCreateCommandPool;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
PFN_vkCreateFence vkCreateFence;
PFN_vkCreateSemaphore vkCreateSemaphore;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
PFN_vkEndCommandBuffer vkEndCommandBuffer;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
PFN_vkQueueSubmit vkQueueSubmit;
PFN_vkWaitForFences vkWaitForFences;
PFN_vkResetFences vkResetFences;
PFN_vkDestroySemaphore vkDestroySemaphore;
PFN_vkResetCommandBuffer vkResetCommandBuffer;

PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
PFN_vkQueuePresentKHR vkQueuePresentKHR;

const char* const vulkanValidationLayer[1] = { "VK_LAYER_LUNARG_standard_validation" };
static std::unique_ptr<VkExtensionProperties[]> vulkanAvailableExtensions;
static uint32_t vulkanAvailableExtensionCount;

bool isVulkanExtensionAvailable(const char* name)
{
    for (uint32_t i = 0; i < vulkanAvailableExtensionCount; ++i) {
        if (!strcmp(vulkanAvailableExtensions[i].extensionName, name))
            return true;
    }
    return false;
}

bool ensureVulkanExtensionAvailable(const char* name)
{
    if (!isVulkanExtensionAvailable(name)) {
        char buf[1024];
        snprintf(buf, sizeof(buf), "Required Vulkan extension \"%s\" is not supported.", name);
        vulkanError(buf);
        return false;
    }
    return true;
}

void vulkanEnumerateAvailableLayers()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::unique_ptr<VkLayerProperties[]> availableLayers{new VkLayerProperties[layerCount]};
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.get());

    vulkanHasValidationLayer = false;
#ifndef NDEBUG
    for (uint32_t i = 0; i < layerCount; ++i) {
        if (!strcmp(availableLayers[i].layerName, vulkanValidationLayer[0])) {
            vulkanHasValidationLayer = true;
            break;
        }
    }
  #endif
}

void vulkanEnumerateAvailableExtensions()
{
    vulkanAvailableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkanAvailableExtensionCount, nullptr);
    vulkanAvailableExtensions.reset(new VkExtensionProperties[vulkanAvailableExtensionCount]);
    vkEnumerateInstanceExtensionProperties(nullptr, &vulkanAvailableExtensionCount, vulkanAvailableExtensions.get());
}

bool vulkanCreateInstance(const std::vector<const char*>& enabledExtensions)
{
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "Game";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = nullptr;
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = nullptr;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = (vulkanHasValidationLayer ? 1 : 0);
    instanceInfo.ppEnabledLayerNames = (vulkanHasValidationLayer ? vulkanValidationLayer : nullptr);
    instanceInfo.enabledExtensionCount = enabledExtensions.size();
    instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();

    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &vulkanInstance);
    if (result != VK_SUCCESS) {
        vulkanError("Unable to create Vulkan instance.");
        return false;
    }

    if (!getVulkanProc("vkGetPhysicalDeviceSurfaceSupportKHR", vkGetPhysicalDeviceSurfaceSupportKHR) ||
        !getVulkanProc("vkGetPhysicalDeviceSurfaceFormatsKHR", vkGetPhysicalDeviceSurfaceFormatsKHR) ||
        !getVulkanProc("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", vkGetPhysicalDeviceSurfaceCapabilitiesKHR) ||
        !getVulkanProc("vkGetPhysicalDeviceSurfacePresentModesKHR", vkGetPhysicalDeviceSurfacePresentModesKHR) ||
        !getVulkanProc("vkCreateSwapchainKHR", vkCreateSwapchainKHR) ||
        !getVulkanProc("vkGetSwapchainImagesKHR", vkGetSwapchainImagesKHR) ||
        !getVulkanProc("vkAcquireNextImageKHR", vkAcquireNextImageKHR) ||
        !getVulkanProc("vkQueuePresentKHR", vkQueuePresentKHR))
        return false;

    return true;
}
