#include "Win32.h"
#include <memory>
#include <vector>
#include <cstring>

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

static VkInstance instance;
static VkDebugReportCallbackEXT debugCallback;
static VkSurfaceKHR surface;
static VkPhysicalDevice physicalDevice;
static VkPhysicalDeviceProperties physicalDeviceProperties;
static int presentQueueIndex;

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
    auto vkEnumeratePhysicalDevices = getVulkanAPI<PFN_vkEnumeratePhysicalDevices>(hVulkanDll, "vkEnumeratePhysicalDevices");
    auto vkGetPhysicalDeviceProperties = getVulkanAPI<PFN_vkGetPhysicalDeviceProperties>(hVulkanDll, "vkGetPhysicalDeviceProperties");
    auto vkGetPhysicalDeviceQueueFamilyProperties = getVulkanAPI<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(hVulkanDll, "vkGetPhysicalDeviceQueueFamilyProperties");
    if (!vkCreateInstance ||
        !vkGetInstanceProcAddr ||
        !vkEnumerateInstanceExtensionProperties ||
        !vkEnumerateInstanceLayerProperties ||
        !vkEnumeratePhysicalDevices ||
        !vkGetPhysicalDeviceProperties ||
        !vkGetPhysicalDeviceQueueFamilyProperties)
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

    std::vector<const char*> enabledExtensions;
    enabledExtensions.emplace_back("VK_KHR_surface");
    enabledExtensions.emplace_back("VK_KHR_win32_surface");
  #ifndef NDEBUG
    bool enableDebugReport = false;
    if (foundValidation && isVulkanExtensionAvailable(availableExtensions, extensionCount, "VK_EXT_debug_report")) {
        enableDebugReport = true;
        enabledExtensions.emplace_back("VK_EXT_debug_report");
    }
  #endif

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
    instanceInfo.enabledExtensionCount = enabledExtensions.size();
    instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();

    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        MessageBox(hWnd, TEXT("Unable to create Vulkan instance."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return false;
    }

    // Enable validation layer in debug builds

  #ifndef NDEBUG
    if (enableDebugReport) {
        auto vkCreateDebugReportCallbackEXT =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        if (!vkCreateDebugReportCallbackEXT)
            MessageBox(hWnd, TEXT("Missing Vulkan API \"vkCreateDebugReportCallbackEXT\"."), TEXT("Error"), MB_ICONERROR | MB_OK);
        else {
            VkDebugReportCallbackCreateInfoEXT info;
            info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
            info.pNext = nullptr;
            info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            info.pfnCallback = debugReportCallback;
            info.pUserData = nullptr;

            result = vkCreateDebugReportCallbackEXT(instance, &info, nullptr, &debugCallback);
            if (result != VK_SUCCESS) {
                MessageBox(hWnd, TEXT("Unable to install debug report callback."), TEXT("Error"), MB_ICONERROR | MB_OK);
                return false;
            }
        }
    }
  #endif

    // Create Win32 surface

    auto vkCreateWin32SurfaceKHR =
        (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
    if (!vkCreateWin32SurfaceKHR) {
        MessageBox(hWnd, TEXT("Missing Vulkan API \"vkCreateWin32SurfaceKHR\"."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return false;
    }

    VkWin32SurfaceCreateInfoKHR surfaceInfo;
    surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.pNext = nullptr;
    surfaceInfo.flags = 0;
    surfaceInfo.hinstance = GetModuleHandle(nullptr);
    surfaceInfo.hwnd = hWnd;

    result = vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
    if (result != VK_SUCCESS) {
        MessageBox(hWnd, TEXT("Unable to create Vulkan surface."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return false;
    }

    // Enumerate devices and queues

    auto vkGetPhysicalDeviceSurfaceSupportKHR =
        (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
    if (!vkGetPhysicalDeviceSurfaceSupportKHR) {
        MessageBox(hWnd, TEXT("Missing Vulkan API \"vkGetPhysicalDeviceSurfaceSupportKHR\"."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return false;
    }

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
    std::unique_ptr<VkPhysicalDevice[]> physicalDevices{new VkPhysicalDevice[physicalDeviceCount]};
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.get());

    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        VkPhysicalDeviceProperties deviceProperties = {};
        vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, nullptr);
        std::unique_ptr<VkQueueFamilyProperties[]> queueFamilyProperties{new VkQueueFamilyProperties[queueFamilyCount]};
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i], &queueFamilyCount, queueFamilyProperties.get());

        for (uint32_t j = 0; j < queueFamilyCount; ++j) {
            VkBool32 supportsPresent;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[i], j, surface, &supportsPresent);
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
        MessageBox(hWnd, TEXT("No physical Vulkan device found."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return false;
    }

    return true;
}
