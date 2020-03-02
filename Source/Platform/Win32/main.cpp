#include "Win32.h"
#include "Engine/Renderer/Vulkan/VulkanRenderDevice.h"
#include "Engine/Renderer/Vulkan/VulkanCommon.h"

HWND hWnd;

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
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRectEx(&r, dwStyle, false, dwExStyle);

    hWnd = CreateWindowEx(dwExStyle, wc.lpszClassName, TEXT("Game"), dwStyle,
        r.left, r.top, r.right - r.left, r.bottom - r.top, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) {
        MessageBox(nullptr, TEXT("Unable to create main window."), TEXT("Error"), MB_ICONERROR | MB_OK);
        return 1;
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    if (!initVulkan()) {
        DestroyWindow(hWnd);
        return 1;
    }

    std::unique_ptr<VulkanRenderDevice> renderDevice{new VulkanRenderDevice};
    if (!renderDevice->initialized()) {
        destroyVulkan();
        DestroyWindow(hWnd);
        return 1;
    }

    MainLoop();

    destroyVulkan();
    DestroyWindow(hWnd);

    return 0;
}
