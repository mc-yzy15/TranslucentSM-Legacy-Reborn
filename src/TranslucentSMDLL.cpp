#include <windows.h>
#include <dwmapi.h>
#include <tlhelp32.h>
#include <string>

#pragma comment(lib, "dwmapi.lib")

// 全局变量
HMODULE hModule = NULL;
HWINEVENTHOOK hEventHook = NULL;

// 函数声明
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime);
void ApplyTransparency(HWND hWnd);
HWND FindStartMenuWindow();

// DLL入口点
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            // 注册窗口事件钩子
            hEventHook = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE,
                NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
            
            // 立即查找并处理已存在的开始菜单窗口
            HWND hStartMenuWnd = FindStartMenuWindow();
            if (hStartMenuWnd) {
                ApplyTransparency(hStartMenuWnd);
            }
            break;
        }
        case DLL_PROCESS_DETACH:
            // 卸载钩子
            if (hEventHook) {
                UnhookWinEvent(hEventHook);
            }
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    return TRUE;
}

// 应用透明度效果
void ApplyTransparency(HWND hWnd) {
    // 获取注册表中的透明度设置
    HKEY hKey;
    DWORD tintLuminosityOpacity = 50;
    DWORD tintOpacity = 50;
    DWORD dataSize = sizeof(DWORD);
    
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\TranslucentSM", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, L"TintLuminosityOpacity", NULL, NULL, (LPBYTE)&tintLuminosityOpacity, &dataSize);
        dataSize = sizeof(DWORD);
        RegQueryValueExW(hKey, L"TintOpacity", NULL, NULL, (LPBYTE)&tintOpacity, &dataSize);
        RegCloseKey(hKey);
    }
    
    // 转换为0-255范围
    BYTE alpha = (BYTE)(tintOpacity * 255 / 100);
    
    // 设置窗口透明度
    DWORD dwExStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    if (!(dwExStyle & WS_EX_LAYERED)) {
        SetWindowLongPtr(hWnd, GWL_EXSTYLE, dwExStyle | WS_EX_LAYERED);
    }
    
    // 设置颜色键和透明度
    COLORREF crKey = RGB(255, 255, 255);
    SetLayeredWindowAttributes(hWnd, crKey, alpha, LWA_ALPHA | LWA_COLORKEY);
    
    // Windows 11 24H2 特有的毛玻璃效果
    DWORD attribute = DWMWA_MICA_EFFECT;
    DWORD value = DWMSBT_MAINWINDOW;
    DwmSetWindowAttribute(hWnd, attribute, &value, sizeof(value));
}

// 查找开始菜单窗口
HWND FindStartMenuWindow() {
    HWND hWnd = NULL;
    // 枚举所有顶级窗口
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        HWND* pHWnd = (HWND*)lParam;
        WCHAR className[256];
        GetClassNameW(hwnd, className, sizeof(className)/sizeof(WCHAR));
        
        // 开始菜单窗口类名
        if (wcscmp(className, L"Windows.UI.Core.CoreWindow") == 0 ||
            wcscmp(className, L"ApplicationFrameWindow") == 0) {
            WCHAR title[256];
            GetWindowTextW(hwnd, title, sizeof(title)/sizeof(WCHAR));
            if (wcsstr(title, L"开始") != NULL || wcsstr(title, L"Start") != NULL) {
                *pHWnd = hwnd;
                return FALSE; // 找到后停止枚举
            }
        }
        return TRUE; // 继续枚举
    }, (LPARAM)&hWnd);
    return hWnd;
}

// 导出函数（供注入器调用）
extern "C" __declspec(dllexport) void InitializeTransparency() {
    // 空实现，仅用于确保DLL被加载
}

// 窗口事件回调函数
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime) {
    if (idObject == OBJID_CLIENT && idChild == CHILDID_SELF) {
        // 尝试应用透明度
        ApplyTransparency(hwnd);
    }
}