/*
 * TranslucentSM DLL - Windows Start Menu Transparency Tool
 * Version: 1.0.0
 * Author: mc-yzy15
 * Email: yingmoliuguang@yeah.net
 * Date: 2026-01-09
 *
 * Description: This DLL is injected into Start Menu process to modify XAML for transparency effects
 * This tool uses XAML diagnostic technology to inject DLL into process and modify XAML
 *
 * Note: This DLL must be used on Windows 11 22000 or above
 */

/* Disable warnings for unsafe functions */
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <dwmapi.h>
#include <tlhelp32.h>
#include <string.h>
#include <shlwapi.h>

/* Linked libraries */
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")

/* Export function declarations */
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif

#define DLL_EXPORT EXTERN_C __declspec(dllexport)

/* Global variables */
HMODULE g_hModule = NULL;
HWINEVENTHOOK g_hEventHook = NULL;

/*
 * Function: ApplyTransparency
 * Description: Apply transparency effect to specified window
 * Parameters: hWnd - Target window handle
 * Return: None
 */
void ApplyTransparency(HWND hWnd);

/*
 * Function: FindStartMenuWindow
 * Description: Find Start Menu window
 * Parameters: None
 * Return: HWND - Start Menu window handle, or NULL if not found
 */
HWND FindStartMenuWindow(void);

/*
 * Function: WinEventProc
 * Description: Window event callback function
 * Parameters:
 *   hWinEventHook - Event hook handle
 *   event - Event type
 *   hwnd - Target window handle
 *   idObject - Object ID
 *   idChild - Child object ID
 *   idEventThread - Event thread ID
 *   dwmsEventTime - Event time
 * Return: None
 */
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                           LONG idObject, LONG idChild, DWORD idEventThread,
                           DWORD dwmsEventTime);

/*
 * Function: GetRegistryValue
 * Description: Get DWORD value from registry
 * Parameters:
 *   subKey - Subkey path
 *   valueName - Value name
 *   defaultValue - Default value
 * Return: DWORD - Retrieved value or default value
 */
DWORD GetRegistryValue(const char* subKey, const char* valueName, DWORD defaultValue);

/*
 * Function: WindowEnumProc
 * Description: Window enumeration callback function
 * Parameters:
 *   hwnd - Window handle
 *   lParam - Parameter passed to EnumWindows
 * Return: BOOL - TRUE to continue enumeration, FALSE to stop
 */
BOOL CALLBACK WindowEnumProc(HWND hwnd, LPARAM lParam);

/*
 * Function: DllMain
 * Description: DLL entry point
 * Parameters:
 *   hModule - DLL module handle
 *   ul_reason_for_call - Reason for call
 *   lpReserved - Reserved parameter
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    UNREFERENCED_PARAMETER(lpReserved);
    
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        /* Save module handle */
        g_hModule = hModule;
        
        /* Install event hook to monitor window creation */
        g_hEventHook = SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE,
            NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
        
        /* Immediately find and process existing Start Menu window */
        HWND hStartMenuWnd = FindStartMenuWindow();
        if (hStartMenuWnd) {
            ApplyTransparency(hStartMenuWnd);
        }
        break;
    }
    case DLL_PROCESS_DETACH: {
        /* Unhook hook */
        if (g_hEventHook) {
            UnhookWinEvent(g_hEventHook);
            g_hEventHook = NULL;
        }
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

/*
 * Function: GetRegistryValue
 * Description: Get DWORD value from registry
 * Parameters:
 *   subKey - Subkey path
 *   valueName - Value name
 *   defaultValue - Default value
 * Return: DWORD - Retrieved value or default value
 */
DWORD GetRegistryValue(const char* subKey, const char* valueName, DWORD defaultValue) {
    HKEY hKey;
    DWORD dwValue = defaultValue;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType;
    LONG lResult;

    /* Open registry key */
    lResult = RegOpenKeyExA(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey);
    if (lResult != ERROR_SUCCESS) {
        return defaultValue;
    }

    /* Read value */
    lResult = RegQueryValueExA(hKey, valueName, NULL, &dwType, 
                              (LPBYTE)&dwValue, &dwSize);
    if (lResult != ERROR_SUCCESS || dwType != REG_DWORD) {
        dwValue = defaultValue;
    }

    /* Close registry key */
    RegCloseKey(hKey);

    return dwValue;
}

/*
 * Function: ApplyTransparency
 * Description: Apply transparency effect to specified window
 * Parameters: hWnd - Target window handle
 * Return: None
 */
void ApplyTransparency(HWND hWnd) {
    /* Get transparency settings from registry */
    DWORD opacity = GetRegistryValue("SOFTWARE\\TranslucentSM", "TintOpacity", 50);
    
    /* Convert to 0-255 range */
    BYTE alpha = (BYTE)(opacity * 255 / 100);
    
    /* Set window transparency */
    LONG_PTR dwExStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    if (!(dwExStyle & WS_EX_LAYERED)) {
        SetWindowLongPtr(hWnd, GWL_EXSTYLE, dwExStyle | WS_EX_LAYERED);
    }
    
    /* Set color key and transparency */
    COLORREF crKey = RGB(255, 255, 255);
    SetLayeredWindowAttributes(hWnd, crKey, alpha, LWA_ALPHA | LWA_COLORKEY);
    
    /* Windows 11 24H2 specific Mica effect */
    #ifndef DWMWA_MICA_EFFECT
    #define DWMWA_MICA_EFFECT 1029
    #endif
    BOOL attributeValue = TRUE; /* Enable Mica effect */
    DwmSetWindowAttribute(hWnd, DWMWA_MICA_EFFECT, &attributeValue, sizeof(attributeValue));
}

/*
 * Function: WindowEnumProc
 * Description: Window enumeration callback function
 * Parameters:
 *   hwnd - Window handle
 *   lParam - Parameter passed to EnumWindows
 * Return: BOOL - TRUE to continue enumeration, FALSE to stop
 */
BOOL CALLBACK WindowEnumProc(HWND hwnd, LPARAM lParam) {
    HWND* pHWnd = (HWND*)lParam;
    char className[256];
    
    /* Get window class name */
    if (GetClassNameA(hwnd, className, sizeof(className)) == 0) {
        return TRUE; /* Continue enumeration */
    }
    
    /* Check if it's Start Menu window */
    if (strcmp(className, "Windows.UI.Core.CoreWindow") == 0 ||
        strcmp(className, "ApplicationFrameWindow") == 0) {
        char title[256];
        /* Get window title */
        if (GetWindowTextA(hwnd, title, sizeof(title)) == 0) {
            return TRUE; /* Continue enumeration */
        }
        
        /* Check if title contains "Start" */
        if (strstr(title, "Start") != NULL) {
            *pHWnd = hwnd;
            return FALSE; /* Stop enumeration after finding */
        }
    }
    return TRUE; /* Continue enumeration */
}

/*
 * Function: FindStartMenuWindow
 * Description: Find Start Menu window
 * Parameters: None
 * Return: HWND - Start Menu window handle, or NULL if not found
 */
HWND FindStartMenuWindow(void) {
    HWND hWnd = NULL;
    
    /* Call enumeration function */
    EnumWindows(WindowEnumProc, (LPARAM)&hWnd);
    
    return hWnd;
}

/*
 * Function: WinEventProc
 * Description: Window event callback function
 * Parameters:
 *   hWinEventHook - Event hook handle
 *   event - Event type
 *   hwnd - Target window handle
 *   idObject - Object ID
 *   idChild - Child object ID
 *   idEventThread - Event thread ID
 *   dwmsEventTime - Event time
 * Return: None
 */
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd,
                           LONG idObject, LONG idChild, DWORD idEventThread,
                           DWORD dwmsEventTime) {
    /* Disable unused parameter warnings */
    UNREFERENCED_PARAMETER(hWinEventHook);
    UNREFERENCED_PARAMETER(event);
    UNREFERENCED_PARAMETER(idEventThread);
    UNREFERENCED_PARAMETER(dwmsEventTime);
    
    /* Only process events for client objects */
    if (idObject == OBJID_CLIENT && idChild == CHILDID_SELF) {
        /* Try to apply transparency */
        ApplyTransparency(hwnd);
    }
}

/*
 * Function: InitializeTransparency
 * Description: Initialize transparency settings
 * Parameters: None
 * Return: None
 *
 * Note: This function is called by injector to ensure DLL is loaded
 */
DLL_EXPORT void InitializeTransparency(void) {
    /* Empty implementation, only used to ensure DLL is loaded */
}

/*
 * Function: ApplyTransparencyToStartMenu
 * Description: Apply transparency to Start Menu
 * Parameters: None
 * Return: BOOL - TRUE for success, FALSE for failure
 */
DLL_EXPORT BOOL ApplyTransparencyToStartMenu(void) {
    HWND hStartMenuWnd = FindStartMenuWindow();
    if (hStartMenuWnd == NULL) {
        return FALSE;
    }
    
    ApplyTransparency(hStartMenuWnd);
    return TRUE;
}
