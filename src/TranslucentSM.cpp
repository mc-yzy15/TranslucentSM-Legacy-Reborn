#include "TranslucentSM.h"
#include <shlwapi.h>
#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include <iostream>
#include <psapi.h>
#include <sddl.h>
#include <versionhelpers.h>

void TranslucentSM::applyTransparencySettings(const QString& processName, int transparencyValue) {
    // 增强版本检测逻辑
    // 使用Windows 11 24H2兼容的版本检测
    BOOL isWindows11OrGreater = FALSE;
    if (GetBuildNumber() >= 22000) {
        isWindows11OrGreater = TRUE;
    } else {
        // 回退到版本信息检查
        OSVERSIONINFOEX osvi = { sizeof(OSVERSIONINFOEX) };
        osvi.dwMajorVersion = 10;
        osvi.dwMinorVersion = 0;
        osvi.dwBuildNumber = 22000;
        DWORDLONG conditionMask = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
        conditionMask = VerSetConditionMask(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
        conditionMask = VerSetConditionMask(conditionMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);
        isWindows11OrGreater = VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER, conditionMask);
    }

    const wchar_t* targetProcessName = processName.toStdWString().c_str();
        
    // 获取进程ID
    DWORD processId = GetProcessIdByName(targetProcessName);
    if (processId == 0) {
        std::cerr << "Failed to find " << processName.toStdString() << " process." << std::endl;
        return;
    }

    // 注入DLL到目标进程
    wchar_t dllPath[MAX_PATH];
    GetModuleFileNameW(NULL, dllPath, MAX_PATH);
    PathRemoveFileSpecW(dllPath);
    PathAppendW(dllPath, L"TranslucentSM.dll");
    
    if (!InjectDLL(processId, dllPath)) {
        std::cerr << "Failed to inject DLL into " << processName.toStdString() << "." << std::endl;
        return;
    }

    // 修改注册表设置
    if (!SetRegistryValues()) {
        std::cerr << "Failed to set registry values." << std::endl;
        return;
    }

    std::cout << "Transparency settings applied successfully." << std::endl;
}

DWORD TranslucentSM::GetProcessIdByName(const wchar_t* processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hSnapshot, &pe)) {
        CloseHandle(hSnapshot);
        return 0;
    }

    do {
        if (_wcsicmp(pe.szExeFile, processName) == 0) {
            CloseHandle(hSnapshot);
            return pe.th32ProcessID;
        }
    } while (Process32NextW(hSnapshot, &pe));

    CloseHandle(hSnapshot);
    return 0;
}

bool TranslucentSM::InjectDLL(DWORD processId, const wchar_t* dllPath) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!hProcess) {
        return false;
    }

    LPVOID pRemoteMem = VirtualAllocEx(hProcess, NULL, (wcslen(dllPath) + 1) * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteMem) {
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pRemoteMem, (LPVOID)dllPath, (wcslen(dllPath) + 1) * sizeof(wchar_t), NULL)) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
    if (!hKernel32) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    FARPROC pLoadLibraryW = GetProcAddress(hKernel32, "LoadLibraryW");
    if (!pLoadLibraryW) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryW, pRemoteMem, 0, NULL);
    if (!hThread) {
        VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    VirtualFreeEx(hProcess, pRemoteMem, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    return true;
}

DWORD TranslucentSM::GetBuildNumber() {
    // 获取系统构建版本号
    HKEY hKey;
    DWORD dwBuildNumber = 0;
    DWORD dataSize = sizeof(DWORD);
    
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, L"CurrentBuildNumber", NULL, NULL, (LPBYTE)&dwBuildNumber, &dataSize);
        RegCloseKey(hKey);
    }
    return dwBuildNumber;
}

bool TranslucentSM::SetRegistryValues() {
    HKEY hKey;
    LONG result = RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\TranslucentSM", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL);
    if (result != ERROR_SUCCESS) {
        return false;
    }

    // 从注册表读取现有值或使用默认值
    DWORD tintLuminosityOpacity = 5;
    DWORD tintOpacity = 5;
    DWORD size = sizeof(DWORD);
    
    // 如果注册表已有值则读取，否则使用默认值
    RegGetValueW(hKey, NULL, L"TintLuminosityOpacity", RRF_RT_REG_DWORD, NULL, &tintLuminosityOpacity, &size);
    RegGetValueW(hKey, NULL, L"TintOpacity", RRF_RT_REG_DWORD, NULL, &tintOpacity, &size);
    
    // 确保值在1-9范围内
    tintLuminosityOpacity = std::max(static_cast<DWORD>(1), std::min(static_cast<DWORD>(9), tintLuminosityOpacity));
    tintOpacity = std::max(static_cast<DWORD>(1), std::min(static_cast<DWORD>(9), tintOpacity));

    result = RegSetValueExW(hKey, L"TintLuminosityOpacity", 0, REG_DWORD, (BYTE*)&tintLuminosityOpacity, sizeof(DWORD));
    if (result != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return false;
    }

    result = RegSetValueExW(hKey, L"TintOpacity", 0, REG_DWORD, (BYTE*)&tintOpacity, sizeof(DWORD));
    if (result != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}