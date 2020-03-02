#include "Win32.h"
#include <memory>

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

static HWND hWnd;

static LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_ERASEBKGND:
            return true;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_CLOSE:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

template <typename T> static T GetVulkanAPI(HMODULE hVulkanDll, const char* name)
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

static bool InitVulkan()
{
    HMODULE hVulkanDll = LoadLibrary(TEXT("vulkan-1.dll"));
    if (!hVulkanDll) {
        MessageBox(hWnd, TEXT("Unable to load Vulkan DLL."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return false;
    }

    auto vkCreateInstance = GetVulkanAPI<PFN_vkCreateInstance>(hVulkanDll, "vkCreateInstance");
    auto vkEnumerateInstanceLayerProperties = GetVulkanAPI<PFN_vkEnumerateInstanceLayerProperties>(hVulkanDll, "vkEnumerateInstanceLayerProperties");
    if (!vkCreateInstance || !vkEnumerateInstanceLayerProperties)
        return false;

    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::unique_ptr<VkLayerProperties[]> availableLayers{new VkLayerProperties[layerCount]};
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.get());

    bool foundValidation = false;
    static const char* validationLayer[] = { "VK_LAYER_LUNARG_standard_validation" };
  #ifndef NDEBUG
    for (uint32_t i = 0; i < layerCount; ++i) {
        if (!strcmp(availableLayers[i].layerName, validationLayer[0])) {
            OutputDebugString(TEXT("Found Vulkan validation layer!\n"));
            foundValidation = true;
            break;
        }
    }
  #endif

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

    return true;
}

static void MainLoop()
{
    MSG msg;
    for (;;) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                return;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = TEXT("MainWindow");
    if (!RegisterClass(&wc)) {
        MessageBox(nullptr, TEXT("Unable to register main window class."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return 1;
    }

    const int width = 1024;
    const int height = 768;

    RECT r;
    r.left = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    r.top = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    r.right = r.left + width;
    r.bottom = r.top + height;

    DWORD dwExStyle = WS_EX_APPWINDOW;
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    AdjustWindowRectEx(&r, dwStyle, false, dwExStyle);

    hWnd = CreateWindowEx(dwExStyle, wc.lpszClassName, TEXT("Game"), dwStyle,
        r.left, r.top, r.right - r.left, r.bottom - r.top, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) {
        MessageBox(nullptr, TEXT("Unable to create main window."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return 1;
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    if (!InitVulkan()) {
        DestroyWindow(hWnd);
        return 1;
    }

    MainLoop();

    DestroyWindow(hWnd);

    return 0;
}
