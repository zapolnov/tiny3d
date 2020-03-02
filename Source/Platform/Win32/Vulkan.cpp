#include "Win32.h"
#include <memory>
#include <cstring>

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

template <typename T> static T getVulkanAPI(HMODULE hVulkanDll, const char* name)
{
    auto fn = (T)GetProcAddress(hVulkanDll, name);
    if (!fn) {
        TCHAR buf[1024];
        wsprintf(buf, TEXT("Entry point \"%hs\" was not found in the Vulkan DLL."), name);
        MessageBox(hWnd, buf, TEXT("Error"), MB_ICONERROR | MB_OK);
        return nullptr;
    }
    return fn;
}

static bool isVulkanExtensionAvailable(
    const std::unique_ptr<VkExtensionProperties[]>& availableExtensions, uint32_t extensionCount, const char* name)
{
    for (uint32_t i = 0; i < extensionCount; ++i) {
        if (!strcmp(availableExtensions[i].extensionName, name))
            return true;
    }
    return false;
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

static bool ensureVulkanExtensionAvailable(
    const std::unique_ptr<VkExtensionProperties[]>& availableExtensions, uint32_t extensionCount, const char* name)
{
    if (!isVulkanExtensionAvailable(availableExtensions, extensionCount, name)) {
        TCHAR buf[1024];
        wsprintf(buf, TEXT("Required Vulkan extension \"%hs\" is not supported."), name);
        MessageBox(hWnd, buf, TEXT("Error"), MB_ICONERROR | MB_OK);
        return false;
    }
    return true;
}

bool initVulkan()
{
    HMODULE hVulkanDll = LoadLibrary(TEXT("vulkan-1.dll"));
    if (!hVulkanDll) {
        MessageBox(hWnd, TEXT("Unable to load Vulkan DLL."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return false;
    }

    auto vkCreateInstance = getVulkanAPI<PFN_vkCreateInstance>(hVulkanDll, "vkCreateInstance");
    auto vkGetInstanceProcAddr = getVulkanAPI<PFN_vkGetInstanceProcAddr>(hVulkanDll, "vkGetInstanceProcAddr");
    auto vkEnumerateInstanceExtensionProperties = getVulkanAPI<PFN_vkEnumerateInstanceExtensionProperties>(hVulkanDll, "vkEnumerateInstanceExtensionProperties");
    auto vkEnumerateInstanceLayerProperties = getVulkanAPI<PFN_vkEnumerateInstanceLayerProperties>(hVulkanDll, "vkEnumerateInstanceLayerProperties");
    if (!vkCreateInstance || !vkEnumerateInstanceExtensionProperties || !vkEnumerateInstanceLayerProperties)
        return false;

    // Enumerate avaiable layers

    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::unique_ptr<VkLayerProperties[]> availableLayers{new VkLayerProperties[layerCount]};
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.get());

    bool foundValidation = false;
  #ifndef NDEBUG
    static const char* const validationLayer[] = { "VK_LAYER_LUNARG_standard_validation" };
    for (uint32_t i = 0; i < layerCount; ++i) {
        if (!strcmp(availableLayers[i].layerName, validationLayer[0])) {
            OutputDebugString(TEXT("Found Vulkan validation layer!\n"));
            foundValidation = true;
            break;
        }
    }
  #endif

    // Enumerate available extensions

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::unique_ptr<VkExtensionProperties[]> availableExtensions{new VkExtensionProperties[extensionCount]};
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.get());

    if (!ensureVulkanExtensionAvailable(availableExtensions, extensionCount, "VK_KHR_surface"))
        return false;
    if (!ensureVulkanExtensionAvailable(availableExtensions, extensionCount, "VK_KHR_win32_surface"))
        return false;

    // Create Vulkan instance

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
    instanceInfo.enabledLayerCount = (foundValidation ? 1 : 0);
    instanceInfo.ppEnabledLayerNames = (foundValidation ? validationLayer : nullptr);
    instanceInfo.enabledExtensionCount = 0;
    instanceInfo.ppEnabledExtensionNames = nullptr;

    VkInstance instance;
    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        MessageBox(hWnd, TEXT("Unable to create Vulkan instance."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return false;
    }

    // Enable validation layer in debug builds

  #ifndef NDEBUG
    if (foundValidation && isVulkanExtensionAvailable(availableExtensions, extensionCount, "VK_EXT_debug_report")) {
        auto vkCreateDebugReportCallbackEXT =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        if (vkCreateDebugReportCallbackEXT) {
            VkDebugReportCallbackCreateInfoEXT info;
            info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
            info.pNext = nullptr;
            info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            info.pfnCallback = debugReportCallback;
            info.pUserData = nullptr;

            VkDebugReportCallbackEXT callback;
            result = vkCreateDebugReportCallbackEXT(instance, &info, nullptr, &callback);
        }
    }
  #endif

    return true;
}
