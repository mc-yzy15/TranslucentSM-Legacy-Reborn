/*
 * TranslucentSM - Windows Start Menu Transparency Tool
 * Version: 1.0.0
 * Author: mc-yzy15
 * Email: yingmoliuguang@yeah.net
 * Date: 2026-01-09
 *
 * Description: A lightweight tool to make Windows Start Menu semi-transparent/transparent
 * This tool uses XAML diagnostic technology to inject DLL into process and modify XAML
 *
 * Note: This application must be used on Windows 11 22000 or above
 */

/* Disable warnings for unsafe functions */
#define CRT_SECURE_NO_WARNINGS

#include "translucentsm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlwapi.h>
#include <wininet.h>
#include <process.h>
#include <tlhelp32.h> // For PROCESSENTRY32 and related functions

/* Linked libraries */
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "advapi32.lib")

/* Global configuration instance */
Config g_config;

/*
 * Function: initConfig
 * Description: Initialize configuration, load from registry or default values
 * Parameters: None
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL initConfig(void)
{
    HKEY hKey;
    DWORD dwSize, dwType;
    LONG lResult;
    char defaultPath[MAX_PATH];
    char defaultTheme[] = "dark";

    /* Set default values */
    g_config.tintLuminosityOpacity = 50;
    g_config.tintOpacity = 50;

    /* Get default install path */
    if (GetEnvironmentVariableA("LOCALAPPDATA", defaultPath, MAX_PATH) == 0)
    {
        strcpy(defaultPath, "C:\\Program Files\\TranslucentSM");
    }
    else
    {
        strcat(defaultPath, "\\TranslucentSM");
    }
    strcpy(g_config.installPath, defaultPath);
    strcpy(g_config.theme, defaultTheme);

    /* Try to read configuration from registry */
    lResult = RegOpenKeyExA(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_READ, &hKey);
    if (lResult == ERROR_SUCCESS)
    {
        /* Read tint luminosity opacity */
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueExA(hKey, "TintLuminosityOpacity", NULL, &dwType,
                                   (LPBYTE)&g_config.tintLuminosityOpacity, &dwSize);
        if (lResult != ERROR_SUCCESS || dwType != REG_DWORD)
        {
            g_config.tintLuminosityOpacity = 50;
        }

        /* Read main acrylic opacity */
        dwSize = sizeof(DWORD);
        lResult = RegQueryValueExA(hKey, "TintOpacity", NULL, &dwType,
                                   (LPBYTE)&g_config.tintOpacity, &dwSize);
        if (lResult != ERROR_SUCCESS || dwType != REG_DWORD)
        {
            g_config.tintOpacity = 50;
        }

        /* Read install path */
        dwSize = MAX_PATH;
        lResult = RegQueryValueExA(hKey, "InstallPath", NULL, &dwType,
                                   (LPBYTE)g_config.installPath, &dwSize);
        if (lResult != ERROR_SUCCESS || dwType != REG_SZ)
        {
            strcpy(g_config.installPath, defaultPath);
        }

        /* Read theme */
        dwSize = 32;
        lResult = RegQueryValueExA(hKey, "Theme", NULL, &dwType,
                                   (LPBYTE)g_config.theme, &dwSize);
        if (lResult != ERROR_SUCCESS || dwType != REG_SZ)
        {
            strcpy(g_config.theme, defaultTheme);
        }

        RegCloseKey(hKey);
    }

    /* Ensure transparency values are within valid range (0-100) */
    if (g_config.tintLuminosityOpacity < 0 || g_config.tintLuminosityOpacity > 100)
    {
        g_config.tintLuminosityOpacity = 50;
    }
    if (g_config.tintOpacity < 0 || g_config.tintOpacity > 100)
    {
        g_config.tintOpacity = 50;
    }

    return TRUE;
}

/*
 * Function: saveConfig
 * Description: Save configuration to registry
 * Parameters: None
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL saveConfig(void)
{
    HKEY hKey;
    LONG lResult;

    /* Create or open registry key */
    lResult = RegCreateKeyExA(HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL,
                              REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (lResult != ERROR_SUCCESS)
    {
        return FALSE;
    }

    /* Save tint luminosity opacity */
    lResult = RegSetValueExA(hKey, "TintLuminosityOpacity", 0, REG_DWORD,
                             (LPBYTE)&g_config.tintLuminosityOpacity, sizeof(DWORD));
    if (lResult != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return FALSE;
    }

    /* Save main acrylic opacity */
    lResult = RegSetValueExA(hKey, "TintOpacity", 0, REG_DWORD,
                             (LPBYTE)&g_config.tintOpacity, sizeof(DWORD));
    if (lResult != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return FALSE;
    }

    /* Save install path */
    lResult = RegSetValueExA(hKey, "InstallPath", 0, REG_SZ,
                             (LPBYTE)g_config.installPath,
                             (DWORD)strlen(g_config.installPath) + 1);
    if (lResult != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return FALSE;
    }

    /* Save theme */
    lResult = RegSetValueExA(hKey, "Theme", 0, REG_SZ,
                             (LPBYTE)g_config.theme,
                             (DWORD)strlen(g_config.theme) + 1);
    if (lResult != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return FALSE;
    }

    RegCloseKey(hKey);
    return TRUE;
}

/*
 * Function: createDirectoryRecursive
 * Description: Create directory recursively
 * Parameters: path - Directory path to create
 * Return: BOOL - TRUE for success, FALSE for failure
 */
static BOOL createDirectoryRecursive(const char* path)
{
    char tempPath[MAX_PATH];
    char* p;
    BOOL bSuccess = TRUE;

    /* Copy path */
    strcpy(tempPath, path);

    /* Handle drive letter */
    if (tempPath[1] == ':' && tempPath[2] == '\\')
    {
        p = tempPath + 3;
    }
    else
    {
        p = tempPath;
    }

    /* Create directories recursively */
    while (*p)
    {
        /* Find path separator */
        while (*p && *p != '\\')
        {
            p++;
        }

        /* Save current character */
        char temp = *p;
        *p = '\0';

        /* Create directory */
        if (!CreateDirectoryA(tempPath, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            bSuccess = FALSE;
            break;
        }

        /* Restore original character */
        *p = temp;
        if (*p)
        {
            p++;
        }
    }

    if (bSuccess)
    {
        /* Create final directory */
        if (!CreateDirectoryA(path, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            bSuccess = FALSE;
        }
    }

    return bSuccess;
}

/*
 * Function: getDllPath
 * Description: Get full path of current DLL
 * Parameters: buffer - Buffer to store path
 *             bufferSize - Buffer size
 * Return: BOOL - TRUE for success, FALSE for failure
 */
static BOOL getDllPath(char* buffer, DWORD bufferSize)
{
    if (GetModuleFileNameA(NULL, buffer, bufferSize) == 0)
    {
        return FALSE;
    }

    /* Get directory part */
    char* p = strrchr(buffer, '\\');
    if (p == NULL)
    {
        return FALSE;
    }
    *p = '\0';

    /* Append DLL file name */
    strcat(buffer, "\\");
    strcat(buffer, DLL_NAME);

    return TRUE;
}

/*
 * Function: installTranslucentSM
 * Description: Install TranslucentSM to system
 * Parameters: installPath - Install path
 * Return: int - 0 for success, other values for error codes
 */
int installTranslucentSM(const char* installPath)
{
    char sourceDll[MAX_PATH];
    char destDll[MAX_PATH];
    char destDir[MAX_PATH];
    BOOL bResult;

    /* Validate parameters */
    if (installPath == NULL || installPath[0] == '\0')
    {
        return 1; /* Invalid install path */
    }

    /* Get source DLL path */
    if (!getDllPath(sourceDll, MAX_PATH))
    {
        return 2; /* Failed to get DLL path */
    }

    /* Create target directory */
    strcpy(destDir, installPath);
    if (!createDirectoryRecursive(destDir))
    {
        return 3; /* Failed to create install directory */
    }

    /* Copy DLL to target directory */
    strcpy(destDll, installPath);
    strcat(destDll, "\\");
    strcat(destDll, DLL_NAME);

    bResult = CopyFileA(sourceDll, destDll, FALSE);
    if (!bResult)
    {
        return 4; /* Failed to copy DLL file */
    }

    /* Update configuration */
    strcpy(g_config.installPath, installPath);
    if (!saveConfig())
    {
        return 5; /* Failed to save configuration */
    }

    /* Additional installation steps can be added here, such as creating shortcuts, registering services, etc. */

    return 0; /* Installation successful */
}

/*
 * Function: uninstallTranslucentSM
 * Description: Uninstall TranslucentSM from system
 * Parameters: None
 * Return: int - 0 for success, other values for error codes
 */
int uninstallTranslucentSM(void)
{
    char dllPath[MAX_PATH];
    HKEY hKey;
    LONG lResult;

    /* Stop any running processes (if needed) */
    /* Code to terminate related processes can be added here */

    /* Delete files in install directory */
    strcpy(dllPath, g_config.installPath);
    strcat(dllPath, "\\");
    strcat(dllPath, DLL_NAME);

    if (!DeleteFileA(dllPath))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_FILE_NOT_FOUND)
        {
            return 1; /* Failed to delete DLL file */
        }
    }

    /* Delete install directory */
    if (!RemoveDirectoryA(g_config.installPath))
    {
        DWORD dwError = GetLastError();
        if (dwError != ERROR_FILE_NOT_FOUND && dwError != ERROR_DIR_NOT_EMPTY)
        {
            return 2; /* Failed to delete install directory */
        }
    }

    /* Delete registry key */
    lResult = RegDeleteKeyA(HKEY_CURRENT_USER, REGISTRY_PATH);
    if (lResult != ERROR_SUCCESS && lResult != ERROR_FILE_NOT_FOUND)
    {
        return 3; /* Failed to delete registry key */
    }

    return 0; /* Uninstallation successful */
}

/*
 * Function: findProcessByName
 * Description: Find process ID by process name
 * Parameters: processName - Process name
 *             pProcessId - Output parameter, stores found process ID
 * Return: BOOL - TRUE if found, FALSE if not found
 */
static BOOL findProcessByName(const char* processName, DWORD* pProcessId)
{
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    BOOL bFound = FALSE;

    /* Create process snapshot */
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    /* Set structure size */
    pe32.dwSize = sizeof(PROCESSENTRY32);

    /* Traverse process list */
    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            if (strcmp(pe32.szExeFile, processName) == 0)
            {
                *pProcessId = pe32.th32ProcessID;
                bFound = TRUE;
                break;
            }
        }
        while (Process32Next(hSnapshot, &pe32));
    }

    /* Close snapshot handle */
    CloseHandle(hSnapshot);

    return bFound;
}

/*
 * Function: injectDll
 * Description: Inject DLL into specified process
 * Parameters: processId - Target process ID
 *             dllPath - Full path of DLL
 * Return: BOOL - TRUE for success, FALSE for failure
 */
static BOOL injectDll(DWORD processId, const char* dllPath)
{
    HANDLE hProcess;
    LPVOID lpBaseAddress = NULL;
    LPVOID lpLoadLibrary;
    SIZE_T dwBytesWritten;
    HANDLE hThread;
    BOOL bResult = FALSE;

    /* Open target process */
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL)
    {
        return FALSE;
    }

    /* Get address of LoadLibraryA function */
    lpLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (lpLoadLibrary == NULL)
    {
        goto cleanup;
    }

    /* Allocate memory */
    lpBaseAddress = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (lpBaseAddress == NULL)
    {
        goto cleanup;
    }

    /* Write DLL path */
    if (!WriteProcessMemory(hProcess, lpBaseAddress, dllPath, strlen(dllPath) + 1, &dwBytesWritten))
    {
        goto cleanup;
    }

    /* Create remote thread */
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpLoadLibrary, lpBaseAddress, 0, NULL);
    if (hThread == NULL)
    {
        goto cleanup;
    }

    /* Wait for thread to finish */
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    bResult = TRUE;

cleanup:
    /* Clean up resources */
    if (lpBaseAddress != NULL)
    {
        VirtualFreeEx(hProcess, lpBaseAddress, 0, MEM_RELEASE);
    }
    CloseHandle(hProcess);

    return bResult;
}

/*
 * Function: applyTransparencySettings
 * Description: Apply transparency settings to specified process
 * Parameters: processName - Target process name
 *             opacity - Opacity value (0-100)
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL applyTransparencySettings(const char* processName, int opacity)
{
    DWORD processId;
    char dllPath[MAX_PATH];
    BOOL bResult;

    /* Validate parameters */
    if (processName == NULL || opacity < 0 || opacity > 100)
    {
        return FALSE;
    }

    /* Update configuration */
    g_config.tintOpacity = opacity;
    if (!saveConfig())
    {
        return FALSE;
    }

    /* Find target process */
    if (!findProcessByName(processName, &processId))
    {
        return FALSE;
    }

    /* Get DLL path */
    if (!getDllPath(dllPath, MAX_PATH))
    {
        return FALSE;
    }

    /* Inject DLL */
    bResult = injectDll(processId, dllPath);
    if (!bResult)
    {
        return FALSE;
    }

    return TRUE;
}

/*
 * Function: checkInstallationStatus
 * Description: Check if TranslucentSM is installed
 * Parameters: None
 * Return: BOOL - TRUE if installed, FALSE if not installed
 */
BOOL checkInstallationStatus(void)
{
    char dllPath[MAX_PATH];

    /* Build DLL path */
    strcpy(dllPath, g_config.installPath);
    strcat(dllPath, "\\");
    strcat(dllPath, DLL_NAME);

    /* Check if file exists */
    return (GetFileAttributesA(dllPath) != INVALID_FILE_ATTRIBUTES);
}

/*
 * Function: downloadFile
 * Description: Download file to specified path
 * Parameters: url - Download URL
 *             destPath - Target file path
 *             progressCallback - Progress callback function (optional)
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL downloadFile(const char* url, const char* destPath, void (*progressCallback)(int))
{
    HINTERNET hInternet, hConnect;
    DWORD dwBytesRead, dwTotalSize = 0, dwCurrentSize = 0;
    char buffer[4096];
    BOOL bResult = FALSE;
    HANDLE hFile;

    /* Initialize WinINet */
    hInternet = InternetOpenA(APP_NAME, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet == NULL)
    {
        return FALSE;
    }

    /* Open connection */
    hConnect = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (hConnect == NULL)
    {
        InternetCloseHandle(hInternet);
        return FALSE;
    }

    /* Get file size */
    DWORD dwBufferLength = sizeof(dwTotalSize);
    if (HttpQueryInfoA(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &dwTotalSize, &dwBufferLength,
                       NULL) == FALSE)
    {
        dwTotalSize = 0;
    }

    /* Create target file */
    hFile = CreateFileA(destPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return FALSE;
    }

    /* Download file */
    while (TRUE)
    {
        if (InternetReadFile(hConnect, buffer, sizeof(buffer), &dwBytesRead) == FALSE)
        {
            break;
        }

        if (dwBytesRead == 0)
        {
            bResult = TRUE;
            break;
        }

        /* Write to file */
        if (WriteFile(hFile, buffer, dwBytesRead, &dwBytesRead, NULL) == FALSE)
        {
            break;
        }

        /* Update progress */
        dwCurrentSize += dwBytesRead;
        if (progressCallback != NULL && dwTotalSize > 0)
        {
            int progress = (int)((dwCurrentSize * 100) / dwTotalSize);
            progressCallback(progress);
        }
    }

    /* Clean up resources */
    CloseHandle(hFile);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    if (!bResult)
    {
        /* Delete incomplete file */
        DeleteFileA(destPath);
    }

    return bResult;
}

/*
 * Function: showMessageBox
 * Description: Show message box
 * Parameters: title - Message box title
 *             message - Message content
 *             type - Message box type (MB_OK, MB_OKCANCEL, etc.)
 * Return: int - Button ID selected by user
 */
int showMessageBox(const char* title, const char* message, int type)
{
    return (int)MessageBoxA(NULL, message, title, type);
}

/*
 * Function: restartStartMenuProcess
 * Description: Restart the Start Menu process to apply changes
 * Parameters: None
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL restartStartMenuProcess(void)
{
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    DWORD processId = 0;
    HANDLE hProcess;

    // Take a snapshot of all processes
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Get the first process
    if (!Process32First(hSnapshot, &pe32))
    {
        CloseHandle(hSnapshot);
        return FALSE;
    }

    // Look for StartMenuExperienceHost.exe
    do
    {
        if (strcmp(pe32.szExeFile, "StartMenuExperienceHost.exe") == 0)
        {
            processId = pe32.th32ProcessID;
            break;
        }
    }
    while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    if (processId == 0)
    {
        // Process not found, try to start it
        SHELLEXECUTEINFOA sei = {0};
        sei.cbSize = sizeof(SHELLEXECUTEINFOA);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.lpVerb = "open";
        sei.lpFile = "explorer";
        sei.lpParameters = "shell:::{4234d49b-0245-4df3-b780-389394344a44}"; // Start menu URI
        sei.nShow = SW_SHOWNORMAL;

        if (ShellExecuteExA(&sei))
        {
            CloseHandle(sei.hProcess);
            return TRUE;
        }
        return FALSE;
    }

    // Open the process
    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (hProcess == NULL)
    {
        return FALSE;
    }

    // Terminate the process
    if (!TerminateProcess(hProcess, 0))
    {
        CloseHandle(hProcess);
        return FALSE;
    }

    CloseHandle(hProcess);

    // Wait a bit for the process to terminate
    Sleep(1000);

    // Restart the process
    SHELLEXECUTEINFOA sei = {0};
    sei.cbSize = sizeof(SHELLEXECUTEINFOA);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = "open";
    sei.lpFile = "explorer";
    sei.lpParameters = "shell:::{4234d49b-0245-4df3-b780-389394344a44}"; // Start menu URI
    sei.nShow = SW_SHOWNORMAL;

    if (ShellExecuteExA(&sei))
    {
        CloseHandle(sei.hProcess);
        return TRUE;
    }

    return FALSE;
}
