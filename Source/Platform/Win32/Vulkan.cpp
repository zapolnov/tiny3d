#include "Win32.h"
#include "Engine/Renderer/Vulkan/VulkanCommon.h"
#include <memory>
#include <vector>
#include <cstring>

static VkDebugReportCallbackEXT debugCallback;

template <typename T> static bool getVulkanAPI(HMODULE hVulkanDll, const char* name, T& fn)
{
    fn = (T)GetProcAddress(hVulkanDll, name);
    if (!fn) {
        TCHAR buf[1024];
        wsprintf(buf, TEXT("Entry point \"%hs\" was not found in the Vulkan DLL."), name);
        MessageBox(hWnd, buf, TEXT("Error"), MB_ICONERROR | MB_OK);
        return false;
    }
    return true;
}

#ifndef NDEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode,
    const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    TCHAR buf[1024];
    wsprintf(buf, TEXT("%hs %hs\n"), pLayerPrefix, pMessage);
    OutputDebugString(buf);
    return VK_FALSE;
}
#endif

bool initVulkan()
{
    HMODULE hVulkanDll = LoadLibrary(TEXT("vulkan-1.dll"));
    if (!hVulkanDll) {
        MessageBox(hWnd, TEXT("Unable to load Vulkan DLL."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return false;
    }

    if (!getVulkanAPI(hVulkanDll, "vkCreateInstance", vkCreateInstance) ||
        !getVulkanAPI(hVulkanDll, "vkGetInstanceProcAddr", vkGetInstanceProcAddr) ||
        !getVulkanAPI(hVulkanDll, "vkEnumerateInstanceExtensionProperties", vkEnumerateInstanceExtensionProperties) ||
        !getVulkanAPI(hVulkanDll, "vkEnumerateInstanceLayerProperties", vkEnumerateInstanceLayerProperties) ||
        !getVulkanAPI(hVulkanDll, "vkEnumeratePhysicalDevices", vkEnumeratePhysicalDevices) ||
        !getVulkanAPI(hVulkanDll, "vkGetPhysicalDeviceProperties", vkGetPhysicalDeviceProperties) ||
        !getVulkanAPI(hVulkanDll, "vkGetPhysicalDeviceQueueFamilyProperties", vkGetPhysicalDeviceQueueFamilyProperties) ||
        !getVulkanAPI(hVulkanDll, "vkCreateDevice", vkCreateDevice) ||
        !getVulkanAPI(hVulkanDll, "vkDestroyDevice", vkDestroyDevice) ||
        !getVulkanAPI(hVulkanDll, "vkGetDeviceQueue", vkGetDeviceQueue) ||
        !getVulkanAPI(hVulkanDll, "vkCreateCommandPool", vkCreateCommandPool) ||
        !getVulkanAPI(hVulkanDll, "vkDestroyCommandPool", vkDestroyCommandPool) ||
        !getVulkanAPI(hVulkanDll, "vkAllocateCommandBuffers", vkAllocateCommandBuffers) ||
        !getVulkanAPI(hVulkanDll, "vkFreeCommandBuffers", vkFreeCommandBuffers) ||
        !getVulkanAPI(hVulkanDll, "vkCreateFence", vkCreateFence) ||
        !getVulkanAPI(hVulkanDll, "vkDestroyFence", vkDestroyFence) ||
        !getVulkanAPI(hVulkanDll, "vkCreateSemaphore", vkCreateSemaphore) ||
        !getVulkanAPI(hVulkanDll, "vkBeginCommandBuffer", vkBeginCommandBuffer) ||
        !getVulkanAPI(hVulkanDll, "vkEndCommandBuffer", vkEndCommandBuffer) ||
        !getVulkanAPI(hVulkanDll, "vkCmdPipelineBarrier", vkCmdPipelineBarrier) ||
        !getVulkanAPI(hVulkanDll, "vkQueueSubmit", vkQueueSubmit) ||
        !getVulkanAPI(hVulkanDll, "vkWaitForFences", vkWaitForFences) ||
        !getVulkanAPI(hVulkanDll, "vkResetFences", vkResetFences) ||
        !getVulkanAPI(hVulkanDll, "vkDestroySemaphore", vkDestroySemaphore) ||
        !getVulkanAPI(hVulkanDll, "vkResetCommandBuffer", vkResetCommandBuffer) ||
        !getVulkanAPI(hVulkanDll, "vkCreateImageView", vkCreateImageView) ||
        !getVulkanAPI(hVulkanDll, "vkDestroyImageView", vkDestroyImageView) ||
        !getVulkanAPI(hVulkanDll, "vkGetPhysicalDeviceMemoryProperties", vkGetPhysicalDeviceMemoryProperties) ||
        !getVulkanAPI(hVulkanDll, "vkCreateImage", vkCreateImage) ||
        !getVulkanAPI(hVulkanDll, "vkDestroyImage", vkDestroyImage) ||
        !getVulkanAPI(hVulkanDll, "vkGetImageMemoryRequirements", vkGetImageMemoryRequirements) ||
        !getVulkanAPI(hVulkanDll, "vkAllocateMemory", vkAllocateMemory) ||
        !getVulkanAPI(hVulkanDll, "vkFreeMemory", vkFreeMemory) ||
        !getVulkanAPI(hVulkanDll, "vkBindImageMemory", vkBindImageMemory) ||
        !getVulkanAPI(hVulkanDll, "vkCreateRenderPass", vkCreateRenderPass) ||
        !getVulkanAPI(hVulkanDll, "vkDestroyRenderPass", vkDestroyRenderPass) ||
        !getVulkanAPI(hVulkanDll, "vkCreateFramebuffer", vkCreateFramebuffer) ||
        !getVulkanAPI(hVulkanDll, "vkCreateBuffer", vkCreateBuffer) ||
        !getVulkanAPI(hVulkanDll, "vkDestroyBuffer", vkDestroyBuffer) ||
        !getVulkanAPI(hVulkanDll, "vkGetBufferMemoryRequirements", vkGetBufferMemoryRequirements) ||
        !getVulkanAPI(hVulkanDll, "vkMapMemory", vkMapMemory) ||
        !getVulkanAPI(hVulkanDll, "vkUnmapMemory", vkUnmapMemory) ||
        !getVulkanAPI(hVulkanDll, "vkBindBufferMemory", vkBindBufferMemory) ||
        !getVulkanAPI(hVulkanDll, "vkCreateShaderModule", vkCreateShaderModule) ||
        !getVulkanAPI(hVulkanDll, "vkDestroyShaderModule", vkDestroyShaderModule) ||
        !getVulkanAPI(hVulkanDll, "vkCreateGraphicsPipelines", vkCreateGraphicsPipelines) ||
        !getVulkanAPI(hVulkanDll, "vkDestroyPipeline", vkDestroyPipeline) ||
        !getVulkanAPI(hVulkanDll, "vkCreatePipelineLayout", vkCreatePipelineLayout) ||
        !getVulkanAPI(hVulkanDll, "vkDestroyPipelineLayout", vkDestroyPipelineLayout) ||
        !getVulkanAPI(hVulkanDll, "vkCmdBeginRenderPass", vkCmdBeginRenderPass) ||
        !getVulkanAPI(hVulkanDll, "vkCmdEndRenderPass", vkCmdEndRenderPass) ||
        !getVulkanAPI(hVulkanDll, "vkCmdBindPipeline", vkCmdBindPipeline) ||
        !getVulkanAPI(hVulkanDll, "vkCmdBindVertexBuffers", vkCmdBindVertexBuffers) ||
        !getVulkanAPI(hVulkanDll, "vkCmdBindIndexBuffer", vkCmdBindIndexBuffer) ||
        !getVulkanAPI(hVulkanDll, "vkCmdDraw", vkCmdDraw) ||
        !getVulkanAPI(hVulkanDll, "vkCmdDrawIndexed", vkCmdDrawIndexed) ||
        !getVulkanAPI(hVulkanDll, "vkCreateDescriptorSetLayout", vkCreateDescriptorSetLayout) ||
        !getVulkanAPI(hVulkanDll, "vkDestroyDescriptorSetLayout", vkDestroyDescriptorSetLayout) ||
        !getVulkanAPI(hVulkanDll, "vkCreateDescriptorPool", vkCreateDescriptorPool) ||
        !getVulkanAPI(hVulkanDll, "vkDestroyDescriptorPool", vkDestroyDescriptorPool) ||
        !getVulkanAPI(hVulkanDll, "vkAllocateDescriptorSets", vkAllocateDescriptorSets) ||
        !getVulkanAPI(hVulkanDll, "vkUpdateDescriptorSets", vkUpdateDescriptorSets) ||
        !getVulkanAPI(hVulkanDll, "vkCmdBindDescriptorSets", vkCmdBindDescriptorSets) ||
        !getVulkanAPI(hVulkanDll, "vkCmdCopyBufferToImage", vkCmdCopyBufferToImage) ||
        !getVulkanAPI(hVulkanDll, "vkCreateSampler", vkCreateSampler) ||
        !getVulkanAPI(hVulkanDll, "vkDestroySampler", vkDestroySampler))
        return false;

    // Enumerate avaiable layers

    vulkanEnumerateAvailableLayers();
    if (vulkanHasValidationLayer)
        OutputDebugString(TEXT("Found Vulkan validation layer!\n"));

    // Enumerate available extensions

    vulkanEnumerateAvailableExtensions();
    if (!ensureVulkanExtensionAvailable("VK_KHR_surface"))
        return false;
    if (!ensureVulkanExtensionAvailable("VK_KHR_win32_surface"))
        return false;

    std::vector<const char*> enabledExtensions;
    enabledExtensions.emplace_back("VK_KHR_surface");
    enabledExtensions.emplace_back("VK_KHR_win32_surface");
  #ifndef NDEBUG
    bool enableDebugReport = false;
    if (vulkanHasValidationLayer && isVulkanExtensionAvailable("VK_EXT_debug_report")) {
        enableDebugReport = true;
        enabledExtensions.emplace_back("VK_EXT_debug_report");
    }
  #endif

    // Create Vulkan instance

    if (!vulkanCreateInstance(enabledExtensions))
        return false;

    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
    if (!getVulkanProc("vkCreateWin32SurfaceKHR", vkCreateWin32SurfaceKHR))
        return false;

    // Enable validation layer in debug builds

  #ifndef NDEBUG
    if (enableDebugReport) {
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
        if (getVulkanProc("vkCreateDebugReportCallbackEXT", vkCreateDebugReportCallbackEXT)) {
            VkDebugReportCallbackCreateInfoEXT info;
            info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
            info.pNext = nullptr;
            info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            info.pfnCallback = debugReportCallback;
            info.pUserData = nullptr;

            VkResult result = vkCreateDebugReportCallbackEXT(vulkanInstance, &info, nullptr, &debugCallback);
            if (result != VK_SUCCESS) {
                MessageBox(hWnd, TEXT("Unable to install debug report callback."), TEXT("Error"), MB_ICONERROR | MB_OK);
                return false;
            }
        }
    }
  #endif

    // Create Win32 surface

    VkWin32SurfaceCreateInfoKHR surfaceInfo;
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.pNext = nullptr;
    surfaceInfo.flags = 0;
    surfaceInfo.hinstance = GetModuleHandle(nullptr);
    surfaceInfo.hwnd = hWnd;

    VkResult result = vkCreateWin32SurfaceKHR(vulkanInstance, &surfaceInfo, nullptr, &vulkanSurface);
    if (result != VK_SUCCESS) {
        MessageBox(hWnd, TEXT("Unable to create Vulkan surface."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return false;
    }

    return true;
}

void destroyVulkan()
{
    // FIXME
}

void getVulkanWindowSize(int* width, int* height)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    *width = rc.right - rc.left;
    *height = rc.bottom - rc.top;
}

void vulkanError(const char* text)
{
    TCHAR buf[1024];
    wsprintf(buf, TEXT("%hs"), text);
    MessageBox(hWnd, buf, TEXT("Error"), MB_ICONERROR | MB_OK);
}
