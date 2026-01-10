#pragma once
#include "framework.h"

// Utility functions for registry operations
static DWORD GetRegistryValue(LPCWSTR keyName, DWORD defaultValue = 0)
{
    DWORD value = defaultValue;
    DWORD size = sizeof(DWORD);
    HKEY hKey = NULL;
    
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\TranslucentSM", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        RegGetValue(hKey, NULL, keyName, RRF_RT_DWORD, NULL, &value, &size);
        RegCloseKey(hKey);
    }
    
    return value;
}

static HRESULT SetRegistryValue(LPCWSTR keyName, DWORD value)
{
    HKEY hKey = NULL;
    DWORD disposition;
    HRESULT hr = S_OK;
    
    if (RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\TranslucentSM", 0, NULL, 
                       REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disposition) == ERROR_SUCCESS)
    {
        if (RegSetValueEx(hKey, keyName, 0, REG_DWORD, (const BYTE*)&value, sizeof(value)) != ERROR_SUCCESS)
        {
            hr = E_FAIL;
        }
        RegCloseKey(hKey);
    }
    else
    {
        hr = E_FAIL;
    }
    
    return hr;
}

// Utility function to find descendant by name
static DependencyObject FindDescendantByName(DependencyObject root, hstring name)
{
    if (!root) return nullptr;
    
    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; i++)
    {
        auto child = VisualTreeHelper::GetChild(root, i);
        if (!child) continue;
        
        auto frameworkElement = child.try_as<FrameworkElement>();
        if (frameworkElement && frameworkElement.Name() == name)
        {
            return child;
        }
        
        auto result = FindDescendantByName(child, name);
        if (result) return result;
    }
    
    return nullptr;
}

// Utility function to validate opacity values
static DWORD ValidateOpacity(DWORD value)
{
    if (value > 100) return 100;
    if (value < 0) return 0;
    return value;
}

// Utility function to convert DWORD to double (0-100 to 0.0-1.0)
static double OpacityToDouble(DWORD value)
{
    return static_cast<double>(ValidateOpacity(value)) / 100.0;
}

// Utility function to check if process is running
static BOOL IsProcessRunning(LPCWSTR processName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return FALSE;
    
    PROCESSENTRY32 entry = { 0 };
    entry.dwSize = sizeof(PROCESSENTRY32);
    
    BOOL found = FALSE;
    if (Process32First(snapshot, &entry))
    {
        do
        {
            if (wcscmp(entry.szExeFile, processName) == 0)
            {
                found = TRUE;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }
    
    CloseHandle(snapshot);
    return found;
}

// Utility function to get process ID
static DWORD GetProcessIdByName(LPCWSTR processName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    
    PROCESSENTRY32 entry = { 0 };
    entry.dwSize = sizeof(PROCESSENTRY32);
    
    DWORD pid = 0;
    if (Process32First(snapshot, &entry))
    {
        do
        {
            if (wcscmp(entry.szExeFile, processName) == 0)
            {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }
    
    CloseHandle(snapshot);
    return pid;
}