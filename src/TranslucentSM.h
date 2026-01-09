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

#ifndef TRANSLUCENTSM_H
#define TRANSLUCENTSM_H

#include <windows.h>

/* Constant definitions */
#define APP_NAME "TranslucentSM"
#define APP_VERSION "1.0.0"
#define REGISTRY_PATH "SOFTWARE\\TranslucentSM"
#define DLL_NAME "TranslucentSM.dll"

/* Configuration parameters */
typedef struct {
    int tintLuminosityOpacity;  /* Tint luminosity opacity (0-100) */
    int tintOpacity;            /* Main acrylic opacity (0-100) */
    char installPath[MAX_PATH]; /* Installation path */
    char theme[32];             /* Theme name */
} Config;

/* Global configuration instance */
extern Config g_config;

/*
 * Function: initConfig
 * Description: Initialize configuration, load from registry or default values
 * Parameters: None
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL initConfig(void);

/*
 * Function: saveConfig
 * Description: Save configuration to registry
 * Parameters: None
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL saveConfig(void);

/*
 * Function: installTranslucentSM
 * Description: Install TranslucentSM to system
 * Parameters: installPath - Install path
 * Return: int - 0 for success, other values for error codes
 */
int installTranslucentSM(const char* installPath);

/*
 * Function: uninstallTranslucentSM
 * Description: Uninstall TranslucentSM from system
 * Parameters: None
 * Return: int - 0 for success, other values for error codes
 */
int uninstallTranslucentSM(void);

/*
 * Function: applyTransparencySettings
 * Description: Apply transparency settings to specified process
 * Parameters:
 *   processName - Target process name
 *   opacity - Opacity value (0-100)
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL applyTransparencySettings(const char* processName, int opacity);

/*
 * Function: checkInstallationStatus
 * Description: Check if TranslucentSM is installed
 * Parameters: None
 * Return: BOOL - TRUE if installed, FALSE if not installed
 */
BOOL checkInstallationStatus(void);

/*
 * Function: downloadFile
 * Description: Download file to specified path
 * Parameters:
 *   url - Download URL
 *   destPath - Target file path
 *   progressCallback - Progress callback function (optional)
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL downloadFile(const char* url, const char* destPath, void (*progressCallback)(int));

/*
 * Function: showMessageBox
 * Description: Show message box
 * Parameters:
 *   title - Message box title
 *   message - Message content
 *   type - Message box type (MB_OK, MB_OKCANCEL, etc.)
 * Return: int - Button ID selected by user
 */
int showMessageBox(const char* title, const char* message, int type);

#endif /* TRANSLUCENTSM_H */
