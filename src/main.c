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
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlobj.h> // For BROWSE-INFO and SHBrowseForFolderA

/* Linked libraries */
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")

/* Constants */
#define WINDOW_WIDTH 850
#define WINDOW_HEIGHT 600
#define WINDOW_CLASS "TranslucentSMClass"
#define WINDOW_TITLE "TranslucentSM - Windows Start Menu Transparency Tool"

/* Color definitions for modern UI */
#define COLOR_PRIMARY RGB(41, 128, 185) /* Blue */
#define COLOR_SECONDARY RGB(52, 152, 219) /* Light Blue */
#undef COLOR_BACKGROUND
#define COLOR_BACKGROUND RGB(245, 247, 250) /* Light Gray */
#define COLOR_PANEL_BACKGROUND RGB(255, 255, 255) /* White */
#define COLOR_TEXT RGB(44, 62, 80) /* Dark Gray */
#define COLOR_TEXT_LIGHT RGB(127, 140, 141) /* Light Gray */
#define COLOR_BORDER RGB(221, 221, 221) /* Border Gray */

/* Font definitions */
#define FONT_FAMILY "Segoe UI"
#define FONT_SIZE_NORMAL 10
#define FONT_SIZE_LARGE 12
#define FONT_SIZE_HEADER 14

/* Control IDs */
#define ID_INSTALL_BUTTON 1001
#define ID_UNINSTALL_BUTTON 1002
#define ID_BROWSE_BUTTON 1003
#define ID_INSTALL_PATH_EDIT 1004
#define ID_TRANSPARENCY_SLIDER 1005
#define ID_TRANSPARENCY_VALUE 1006
#define ID_THEME_COMBO 1007
#define ID_APPLY_BUTTON 1008
#define ID_CHECK_UPDATE_BUTTON 1009
#define ID_DOWNLOAD_SYMBOLS_BUTTON 1010
#define ID_TAB_CONTROL 1011

/* Global variables */
HWND g_hWnd = NULL;
HWND g_hTabControl = NULL;
HWND g_hInstallButton = NULL;
HWND g_hUninstallButton = NULL;
HWND g_hBrowseButton = NULL;
HWND g_hInstallPathEdit = NULL;
HWND g_hTransparencySlider = NULL;
HWND g_hTransparencyValue = NULL;
HWND g_hThemeCombo = NULL;
HWND g_hApplyButton = NULL;
HWND g_hCheckUpdateButton = NULL;
HWND g_hDownloadSymbolsButton = NULL;

/* Tab page handles */
HWND g_hPage1 = NULL;
HWND g_hPage2 = NULL;
HWND g_hPage3 = NULL;

/*
 * Function: CreateCustomFont
 * Description: Create a custom font with specified size and style
 * Parameters:
 *   size - Font size
 *   bold - TRUE for bold font, FALSE for regular
 *   italic - TRUE for italic font, FALSE for regular
 * Return: FONT - Handle to the created font, or NULL on failure
 */
HFONT CreateCustomFont(int size, BOOL bold, BOOL italic);

/*
 * Function: SetControlStyles
 * Description: Set modern styles for a control
 * Parameters:
 *   hWnd - Control handle
 *   isButton - TRUE if control is a button, FALSE otherwise
 * Return: None
 */
void SetControlStyles(HWND hWnd, BOOL isButton);

/*
 * Function: WndProc
 * Description: Window message processing function
 * Parameters:
 *   hWnd - Window handle
 *   uMsg - Message type
 *   wParam - Message parameter
 *   lParam - Message parameter
 * Return: RESULT - Message processing result
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*
 * Function: InitAppCommonControls
 * Description: Initialize common controls
 * Parameters: None
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL InitAppCommonControls(void);

/*
 * Function: CreateTabControl
 * Description: Create tab control
 * Parameters: hParent - Parent window handle
 * Return: HAND - Tab control handle
 */
HWND CreateTabControl(HWND hParent);

/*
 * Function: CreatePage1
 * Description: Create install/uninstall page
 * Parameters: hParent - Parent window handle
 * Return: HAND - Page handle
 */
HWND CreatePage1(HWND hParent);

/*
 * Function: CreatePage2
 * Description: Create settings page
 * Parameters: hParent - Parent window handle
 * Return: HAND - Page handle
 */
HWND CreatePage2(HWND hParent);

/*
 * Function: CreatePage3
 * Description: Create about page
 * Parameters: hParent - Parent window handle
 * Return: HAND - Page handle
 */
HWND CreatePage3(HWND hParent);

/*
 * Function: UpdateUIState
 * Description: Update UI state
 * Parameters: None
 * Return: None
 */
void UpdateUIState(void);

/*
 * Function: OnInstallClicked
 * Description: Handle install button click event
 * Parameters: None
 * Return: None
 */
void OnInstallClicked(void);

/*
 * Function: OnUninstallClicked
 * Description: Handle uninstall button click event
 * Parameters: None
 * Return: None
 */
void OnUninstallClicked(void);

/*
 * Function: OnBrowseClicked
 * Description: Handle browse button click event
 * Parameters: None
 * Return: None
 */
void OnBrowseClicked(void);

/*
 * Function: OnApplyClicked
 * Description: Handle apply button click event
 * Parameters: None
 * Return: None
 */
void OnApplyClicked(void);

/*
 * Function: OnCheckUpdateClicked
 * Description: Handle check update button click event
 * Parameters: None
 * Return: None
 */
void OnCheckUpdateClicked(void);

/*
 * Function: OnDownloadSymbolsClicked
 * Description: Handle download symbols button click event
 * Parameters: None
 * Return: None
 */
void OnDownloadSymbolsClicked(void);

/*
 * Function: OnTransparencyChanged
 * Description: Handle transparency slider change event
 * Parameters: value - Slider value
 * Return: None
 */
void OnTransparencyChanged(int value);

/*
 * Function: OnThemeChanged
 * Description: Handle theme combo box change event
 * Parameters: index - Selected index
 * Return: None
 */
void OnThemeChanged(int index);

/*
 * Function: UpdateTabSelection
 * Description: Update tab selection
 * Parameters: tabIndex - Selected tab index
 * Return: None
 */
void UpdateTabSelection(int tabIndex);

/*
 * Function: RegisterWindowClass
 * Description: Register window class
 * Parameters: hInstance - Application instance handle
 * Return: ATOM - Window class atom
 */
ATOM RegisterWindowClass(HINSTANCE hInstance);

/*
 * Function: InitInstance
 * Description: Initialize application instance
 * Parameters:
 *   hInstance - Application instance handle
 *   nCmdShow - Window show mode
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);

/*
 * Function: main
 * Description: Application entry point
 * Parameters:
 *   hInstance - Application instance handle
 *   hPrevInstance - Previous application instance handle
 *   lpCmdLine - Command line parameters
 *   nCmdShow - Window show mode
 * Return: int - Application exit code
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    MSG msg;
    BOOL bRet;
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(hPrevInstance);

    /* Initialize common controls */
    if (!InitAppCommonControls()) {
        MessageBox(NULL, "Failed to initialize common controls library", "Error", MB_ICONERROR | MB_OK);
    }

    /* Register window class */
    if (!RegisterWindowClass(hInstance)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    /* Initialize configuration */
    if (!initConfig()) {
        MessageBox(NULL, "Failed to initialize configuration", "Warning", MB_ICONWARNING | MB_OK);
    }

    /* Initialize application instance */
    if (!InitInstance(hInstance, nCmdShow)) {
        return 1;
    }

    /* Message loop */
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) {
            /* Handle error */
            break;
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

/*
 * Function: RegisterWindowClass
 * Description: Register window class
 * Parameters: hInstance - Application instance handle
 * Return: ATOM - Window class atom
 */
ATOM RegisterWindowClass(HINSTANCE hInstance) {
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = WINDOW_CLASS;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    return RegisterClassEx(&wcex);
}

/*
 * Function: InitInstance
 * Description: Initialize application instance
 * Parameters:
 *   hInstance - Application instance handle
 *   nCmdShow - Window show mode
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    /* Create main window */
    g_hWnd = CreateWindowEx(
        0,
        WINDOW_CLASS,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!g_hWnd) {
        return FALSE;
    }

    /* Show window */
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    return TRUE;
}

/*
 * Function: InitAppCommonControls
 * Description: Initialize common controls
 * Parameters: None
 * Return: BOOL - TRUE for success, FALSE for failure
 */
BOOL InitAppCommonControls(void) {
    /* Initialize the structure correctly */
    INITCOMMONCONTROLSEX icex;
    ZeroMemory(&icex, sizeof(icex));
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    
    /* Include all necessary control classes */
    icex.dwICC = ICC_TAB_CLASSES | ICC_BAR_CLASSES | ICC_STANDARD_CLASSES | ICC_COOL_CLASSES;
    
    /* Call the Windows API function */
    BOOL result = InitCommonControlsEx(&icex);
    
    return result;
}

/*
 * Function: WndProc
 * Description: Window message processing function
 * Parameters:
 *   hWnd - Window handle
 *   uMsg - Message type
 *   wParam - Message parameter
 *   lParam - Message parameter
 * Return: LRESULT - Message processing result
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int iCtrlId;
    NMHDR* pnmhdr;

    switch (uMsg) {
    case WM_CREATE:
        /* Create UI controls */
        g_hTabControl = CreateTabControl(hWnd);
        break;

    case WM_COMMAND:
        iCtrlId = LOWORD(wParam);
        switch (iCtrlId) {
        case ID_INSTALL_BUTTON:
            OnInstallClicked();
            break;
        case ID_UNINSTALL_BUTTON:
            OnUninstallClicked();
            break;
        case ID_BROWSE_BUTTON:
            OnBrowseClicked();
            break;
        case ID_APPLY_BUTTON:
            OnApplyClicked();
            break;
        case ID_CHECK_UPDATE_BUTTON:
            OnCheckUpdateClicked();
            break;
        case ID_DOWNLOAD_SYMBOLS_BUTTON:
            OnDownloadSymbolsClicked();
            break;
        case ID_TRANSPARENCY_SLIDER:
            if (HIWORD(wParam) == EN_CHANGE) {
                /* Slider value change */
                int value = (int)SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
                OnTransparencyChanged(value);
            }
            break;
        case ID_THEME_COMBO:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                /* Theme combo box selection change */
                int index = (int)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                OnThemeChanged(index);
            }
            break;
        default: ;
        }
        break;

    case WM_NOTIFY:
        pnmhdr = (NMHDR*)lParam;
        if (pnmhdr->code == TCN_SELCHANGE && pnmhdr->hwndFrom == g_hTabControl) {
            /* Tab selection change */
            int tabIndex = TabCtrl_GetCurSel(g_hTabControl);
            UpdateTabSelection(tabIndex);
        }
        break;

    case WM_SIZE:
        /* Adjust control sizes */
        if (g_hTabControl != NULL) {
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            SetWindowPos(g_hTabControl, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
        }
        break;
    
    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, COLOR_TEXT);
        SetBkMode(hdcStatic, TRANSPARENT);
        return (LRESULT)GetStockObject(HOLLOW_BRUSH);
    }
    
    case WM_CTLCOLORBTN:
    {
        HDC hdcButton = (HDC)wParam;
        SetTextColor(hdcButton, COLOR_TEXT);
        SetBkMode(hdcButton, TRANSPARENT);
        return (LRESULT)GetStockObject(HOLLOW_BRUSH);
    }

    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

/*
 * Function: CreateTabControl
 * Description: Create tab control
 * Parameters: hParent - Parent window handle
 * Return: HAND - Tab control handle
 */
HWND CreateTabControl(HWND hParent) {
    RECT rcClient;
    TCITEM tie;
    char szTabText[256];

    /* Get parent client area size */
    GetClientRect(hParent, &rcClient);

    /* Create tab control */
    HWND hTab;
    hTab = (
        WC_TABCONTROL,
        "",
        WS_CLIPCHILDREN | (WS_TABSTOP | (WS_CHILD | WS_VISIBLE) | WS_CLIPSIBLINGS),
        0,
        0,
        rcClient.right,
        rcClient.bottom,
        hParent,
        (HMENU)ID_TAB_CONTROL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!hTab) {
        return NULL;
    }

    /* Set tab properties */
    tie.mask = TCIF_TEXT;

    /* Add tab pages */
    /* Install/Uninstall tab */
    strcpy(szTabText, "Install/Uninstall");
    tie.pszText = szTabText;
    TabCtrl_InsertItem(hTab, 0, &tie);

    /* Settings tab */
    strcpy(szTabText, "Settings");
    tie.pszText = szTabText;
    TabCtrl_InsertItem(hTab, 1, &tie);

    /* About tab */
    strcpy(szTabText, "About");
    tie.pszText = szTabText;
    TabCtrl_InsertItem(hTab, 2, &tie);

    /* Create tab pages */
    g_hPage1 = CreatePage1(hTab);
    g_hPage2 = CreatePage2(hTab);
    g_hPage3 = CreatePage3(hTab);

    /* Initially show first tab */
    UpdateTabSelection(0);

    return hTab;
}

/*
 * Function: CreatePage1
 * Description: Create install/uninstall page
 * Parameters: hParent - Parent window handle
 * Return: HWND - Page handle
 */
HWND CreatePage1(HWND hParent) {
    RECT rcClient, rcTab;

    /* Get parent client area size */
    GetClientRect(hParent, &rcClient);
    /* Get tab size */
    GetWindowRect(hParent, &rcTab);
    /* Calculate tab height */
    int tabHeight = 32;

    /* Create page */
    HWND hPage = (
        WC_STATIC,
        "",
        WS_CHILD | WS_VISIBLE,
        0,
        tabHeight,
        rcClient.right,
        rcClient.bottom - tabHeight,
        hParent,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!hPage) {
        return NULL;
    }

    /* Set background color */
    SetClassLongPtr(hPage, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(COLOR_BACKGROUND));

    /* Create container for install section */
    HWND hContainer = CreateWindow(
        WC_STATIC,
        "",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        20,
        20,
        rcClient.right - 40,
        rcClient.bottom - tabHeight - 40,
        hPage,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SetClassLongPtr(hContainer, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(COLOR_PANEL_BACKGROUND));
    
    /* Create section title */
    HWND hLabel = (
        WC_STATIC,
        "Installation",
        WS_CHILD | WS_VISIBLE,
        20,
        20,
        200,
        25,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hLabel, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_HEADER, TRUE, FALSE), TRUE);

    /* Create install path label */
    hLabel = CreateWindow(
        WC_STATIC,
        "Install Path:",
        WS_CHILD | WS_VISIBLE,
        20,
        60,
        100,
        25,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hLabel, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, FALSE, FALSE), TRUE);

    /* Create install path edit box */
    g_hInstallPathEdit = CreateWindow(
        WC_EDIT,
        g_config.installPath,
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        130,
        60,
        450,
        30,
        hContainer,
        (HMENU)ID_INSTALL_PATH_EDIT,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(g_hInstallPathEdit, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, FALSE, FALSE), TRUE);

    /* Create browse button */
    g_hBrowseButton = CreateWindow(
        WC_BUTTON,
        "Browse...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        590,
        60,
        100,
        30,
        hContainer,
        (HMENU)ID_BROWSE_BUTTON,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(g_hBrowseButton, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, TRUE, FALSE), TRUE);

    /* Create install description label */
    hLabel = CreateWindow(
        WC_STATIC,
        "TranslucentSM provides transparency effects for Windows 11 Start Menu. Note: This application must be used on Windows 11 22000 or above.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20,
        110,
        rcClient.right - 100,
        40,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hLabel, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, FALSE, FALSE), TRUE);

    /* Create separator */
    CreateWindow(
        WC_STATIC,
        "",
        WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        20,
        160,
        rcClient.right - 100,
        2,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    /* Create install button */
    g_hInstallButton = CreateWindow(
        WC_BUTTON,
        "Install",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        300,
        190,
        120,
        40,
        hContainer,
        (HMENU)ID_INSTALL_BUTTON,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(g_hInstallButton, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, TRUE, FALSE), TRUE);

    /* Create uninstall button */
    g_hUninstallButton = CreateWindow(
        WC_BUTTON,
        "Uninstall",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        430,
        190,
        120,
        40,
        hContainer,
        (HMENU)ID_UNINSTALL_BUTTON,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(g_hUninstallButton, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, TRUE, FALSE), TRUE);

    /* Update UI state */
    UpdateUIState();

    return hPage;
}

/*
 * Function: CreatePage2
 * Description: Create settings page
 * Parameters: hParent - Parent window handle
 * Return: HWND - Page handle
 */
HWND CreatePage2(HWND hParent) {
    RECT rcClient;

    /* Get parent client area size */
    GetClientRect(hParent, &rcClient);
    /* Calculate tab height */
    int tabHeight = 32;

    /* Create page */
    HWND hPage = CreateWindow(
        WC_STATIC,
        "",
        WS_CHILD | WS_VISIBLE,
        0,
        tabHeight,
        rcClient.right,
        rcClient.bottom - tabHeight,
        hParent,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!hPage) {
        return NULL;
    }

    /* Set background color */
    SetClassLongPtr(hPage, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(COLOR_BACKGROUND));

    /* Create container for settings */
    HWND hContainer = CreateWindow(
        WC_STATIC,
        "",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        20,
        20,
        rcClient.right - 40,
        rcClient.bottom - tabHeight - 40,
        hPage,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SetClassLongPtr(hContainer, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(COLOR_PANEL_BACKGROUND));
    
    /* Create section title */
    HWND hLabel = CreateWindow(
        WC_STATIC,
        "Settings",
        WS_CHILD | WS_VISIBLE,
        20,
        20,
        200,
        25,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hLabel, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_HEADER, TRUE, FALSE), TRUE);

    /* Create transparency setting label */
    hLabel = CreateWindow(
        WC_STATIC,
        "Transparency Setting:",
        WS_CHILD | WS_VISIBLE,
        20,
        60,
        120,
        25,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hLabel, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, FALSE, FALSE), TRUE);

    /* Create transparency slider */
    g_hTransparencySlider = CreateWindow(
        TRACKBAR_CLASS,
        "",
        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE | WS_TABSTOP,
        150,
        60,
        400,
        30,
        hContainer,
        (HMENU)ID_TRANSPARENCY_SLIDER,
        GetModuleHandle(NULL),
        NULL
    );
    /* Set slider range */
    SendMessage(g_hTransparencySlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
    /* Set initial value */
    SendMessage(g_hTransparencySlider, TBM_SETPOS, TRUE, g_config.tintOpacity);
    /* Set slider font */
    SendMessage(g_hTransparencySlider, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, FALSE, FALSE), TRUE);

    /* Create transparency value display */
    g_hTransparencyValue = CreateWindow(
        WC_EDIT,
        "50%",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER,
        560,
        60,
        80,
        30,
        hContainer,
        (HMENU)ID_TRANSPARENCY_VALUE,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(g_hTransparencyValue, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, TRUE, FALSE), TRUE);

    /* Create theme selection label */
    hLabel = CreateWindow(
        WC_STATIC,
        "Theme Selection:",
        WS_CHILD | WS_VISIBLE,
        20,
        110,
        120,
        25,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hLabel, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, FALSE, FALSE), TRUE);

    /* Create theme combo box */
    g_hThemeCombo = CreateWindow(
        WC_COMBOBOX,
        "",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
        150,
        110,
        220,
        200,
        hContainer,
        (HMENU)ID_THEME_COMBO,
        GetModuleHandle(NULL),
        NULL
    );
    /* Set combo box font */
    SendMessage(g_hThemeCombo, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, FALSE, FALSE), TRUE);
    /* Add theme options */
    SendMessage(g_hThemeCombo, CB_ADDSTRING, 0, (LPARAM)"Default Theme");
    SendMessage(g_hThemeCombo, CB_ADDSTRING, 0, (LPARAM)"Dark Theme");
    SendMessage(g_hThemeCombo, CB_ADDSTRING, 0, (LPARAM)"Light Theme");
    /* Set initial selection */
    if (strcmp(g_config.theme, "dark") == 0) {
        SendMessage(g_hThemeCombo, CB_SETCURSEL, 1, 0);
    } else if (strcmp(g_config.theme, "light") == 0) {
        SendMessage(g_hThemeCombo, CB_SETCURSEL, 2, 0);
    } else {
        SendMessage(g_hThemeCombo, CB_SETCURSEL, 0, 0);
    }

    /* Create separator */
    CreateWindow(
        WC_STATIC,
        "",
        WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        20,
        160,
        rcClient.right - 100,
        2,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    /* Create apply button */
    g_hApplyButton = CreateWindow(
        WC_BUTTON,
        "Apply Settings",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        300,
        190,
        120,
        40,
        hContainer,
        (HMENU)ID_APPLY_BUTTON,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(g_hApplyButton, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, TRUE, FALSE), TRUE);

    /* Create check update button */
    g_hCheckUpdateButton = CreateWindow(
        WC_BUTTON,
        "Check Update",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        430,
        190,
        120,
        40,
        hContainer,
        (HMENU)ID_CHECK_UPDATE_BUTTON,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(g_hCheckUpdateButton, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, TRUE, FALSE), TRUE);

    /* Create download symbols button */
    g_hDownloadSymbolsButton = CreateWindow(
        WC_BUTTON,
        "Download Symbols",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        300,
        240,
        250,
        40,
        hContainer,
        (HMENU)ID_DOWNLOAD_SYMBOLS_BUTTON,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(g_hDownloadSymbolsButton, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, TRUE, FALSE), TRUE);

    /* Update UI state */
    UpdateUIState();

    return hPage;
}

/*
 * Function: CreatePage3
 * Description: Create about page
 * Parameters: hParent - Parent window handle
 * Return: HWND - Page handle
 */
HWND CreatePage3(HWND hParent) {
    RECT rcClient;

    /* Get parent client area size */
    GetClientRect(hParent, &rcClient);
    /* Calculate tab height */
    int tabHeight = 32;

    /* Create page */
    HWND hPage = (
        WC_STATIC,
        "",
        WS_CHILD | WS_VISIBLE,
        0,
        tabHeight,
        rcClient.right,
        rcClient.bottom - tabHeight,
        hParent,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!hPage) {
        return NULL;
    }

    /* Set background color */
    SetClassLongPtr(hPage, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(COLOR_BACKGROUND));

    /* Create container for about section */
    HWND hContainer = CreateWindow(
        WC_STATIC,
        "",
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        20,
        20,
        rcClient.right - 40,
        rcClient.bottom - tabHeight - 40,
        hPage,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SetClassLongPtr(hContainer, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(COLOR_PANEL_BACKGROUND));
    
    /* Create section title */
    HWND hLabel = (
        WC_STATIC,
        "About",
        WS_CHILD | WS_VISIBLE,
        20,
        20,
        200,
        25,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hLabel, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_HEADER, TRUE, FALSE), TRUE);

    /* Create application icon */
    CreateWindow(
        WC_STATIC,
        "",
        WS_CHILD | WS_VISIBLE | SS_ICON | SS_CENTER,
        (rcClient.right - 40) / 2 - 32,
        60,
        64,
        64,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    /* Create application name */
    hLabel = CreateWindow(
        WC_STATIC,
        "TranslucentSM",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        0,
        130,
        rcClient.right - 40,
        35,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hLabel, WM_SETFONT, (WPARAM)CreateCustomFont(24, TRUE, FALSE), TRUE);

    /* Create version information */
    hLabel = CreateWindow(
        WC_STATIC,
        "Version 1.0.0",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        0,
        170,
        rcClient.right - 40,
        25,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hLabel, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_LARGE, FALSE, FALSE), TRUE);

    /* Create separator */
    CreateWindow(
        WC_STATIC,
        "",
        WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        40,
        200,
        rcClient.right - 120,
        2,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    /* Create author information */
    hLabel = CreateWindow(
        WC_STATIC,
        "Author: mc-yzy15",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        0,
        220,
        rcClient.right - 40,
        20,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hLabel, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, FALSE, FALSE), TRUE);

    /* Create email information */
    hLabel = CreateWindow(
        WC_STATIC,
        "Email: yingmoliuguang@yeah.net",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        0,
        245,
        rcClient.right - 40,
        20,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hLabel, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, FALSE, FALSE), TRUE);

    /* Create GitHub information */
    hLabel = CreateWindow(
        WC_STATIC,
        "GitHub: https://github.com/mc-yzy15/TranslucentSM-Legacy-Reborn",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        0,
        270,
        rcClient.right - 40,
        20,
        hContainer,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    SendMessage(hLabel, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, FALSE, FALSE), TRUE);

    return hPage;
}

/*
 * Function: UpdateTabSelection
 * Description: Update tab selection
 * Parameters: tabIndex - Selected tab index
 * Return: None
 */
void UpdateTabSelection(int tabIndex) {
    /* Hide all pages */
    ShowWindow(g_hPage1, SW_HIDE);
    ShowWindow(g_hPage2, SW_HIDE);
    ShowWindow(g_hPage3, SW_HIDE);

    /* Show selected page */
    switch (tabIndex) {
    case 0:
        ShowWindow(g_hPage1, SW_SHOW);
        break;
    case 1:
        ShowWindow(g_hPage2, SW_SHOW);
        break;
    case 2:
        ShowWindow(g_hPage3, SW_SHOW);
        break;
    default: ;
    }
}

/*
 * Function: UpdateUIState
 * Description: Update UI state
 * Parameters: None
 * Return: None
 */
void UpdateUIState(void) {
    BOOL isInstalled = checkInstallationStatus();

    /* Update button states */
    EnableWindow(g_hInstallButton, !isInstalled);
    EnableWindow(g_hUninstallButton, isInstalled);
    EnableWindow(g_hApplyButton, isInstalled);
    EnableWindow(g_hTransparencySlider, isInstalled);
    EnableWindow(g_hThemeCombo, isInstalled);
}

/*
 * Function: OnInstallClicked
 * Description: Handle install button click event
 * Parameters: None
 * Return: None
 */
void OnInstallClicked(void) {
    char installPath[MAX_PATH];

    /* Get install path */
    GetWindowTextA(g_hInstallPathEdit, installPath, MAX_PATH);

    /* Validate install path */
    if (installPath[0] == '\0') {
        showMessageBox("Error", "Please enter an install path", MB_OK | MB_ICONERROR);
        return;
    }

    /* Execute install */
    int result = installTranslucentSM(installPath);
    if (result == 0) {
        showMessageBox("Success", "Installation completed successfully!", MB_OK | MB_ICONINFORMATION);
        UpdateUIState();
    } else {
        showMessageBox("Error", "Installation failed, please check logs for detailed information.", MB_OK | MB_ICONERROR);
    }
}

/*
 * Function: OnUninstallClicked
 * Description: Handle uninstall button click event
 * Parameters: None
 * Return: None
 */
void OnUninstallClicked(void) {
    /* Confirm uninstall */
    int choice = showMessageBox("Confirmation", "Are you sure you want to uninstall TranslucentSM?",
                                MB_YESNO | MB_ICONQUESTION);
    if (choice != IDYES) {
        return;
    }

    /* Execute uninstall */
    int result = uninstallTranslucentSM();
    if (result == 0) {
        showMessageBox("Success", "Uninstallation completed successfully!", MB_OK | MB_ICONINFORMATION);
        UpdateUIState();
    } else {
        showMessageBox("Error", "Uninstallation failed, please check logs for detailed information.", MB_OK | MB_ICONERROR);
    }
}

/*
 * Function: OnBrowseClicked
 * Description: Handle browse button click event
 * Parameters: None
 * Return: None
 */
void OnBrowseClicked(void) {
    BROWSEINFO bi;
    char path[MAX_PATH];

    /* Initialize BROWSEINFO */
    memset(&bi, 0, sizeof(bi));
    bi.lpszTitle = "Select Install Directory";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    /* Show browse dialog */
    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl != NULL) {
        /* Get selected path */
        if (SHGetPathFromIDListA(pidl, path)) {
            /* Update edit box */
            SetWindowTextA(g_hInstallPathEdit, path);
        }
        /* Free memory */
        CoTaskMemFree(pidl);
    }
}

/*
 * Function: OnApplyClicked
 * Description: Handle apply button click event
 * Parameters: None
 * Return: None
 */
void OnApplyClicked(void) {
    /* Get transparency value */
    int opacity = (int)SendMessage(g_hTransparencySlider, TBM_GETPOS, 0, 0);
    /* Get theme */
    int themeIndex = (int)SendMessage(g_hThemeCombo, CB_GETCURSEL, 0, 0);
    const char* themeNames[] = { "default", "dark", "light" };
    const char* theme = themeNames[themeIndex];

    /* Save theme */
    strcpy(g_config.theme, theme);
    /* Save configuration */
    if (saveConfig()) {
        /* Apply transparency settings */
        if (applyTransparencySettings("StartMenuExperienceHost.exe", opacity)) {
            showMessageBox("Success", "Transparency settings have been applied!", MB_OK | MB_ICONINFORMATION);
        } else {
            showMessageBox("Warning", "Failed to apply transparency settings, you may need to restart the Start Menu process.", MB_OK | MB_ICONWARNING);
        }
    } else {
        showMessageBox("Error", "Failed to save configuration!", MB_OK | MB_ICONERROR);
    }
}

/*
 * Function: OnCheckUpdateClicked
 * Description: Handle check update button click event
 * Parameters: None
 * Return: None
 */
void OnCheckUpdateClicked(void) {
    showMessageBox("Information", "Check update function not yet implemented.", MB_OK | MB_ICONINFORMATION);
}

/*
 * Function: OnDownloadSymbolsClicked
 * Description: Handle download symbols button click event
 * Parameters: None
 * Return: None
 */
void OnDownloadSymbolsClicked(void) {
    showMessageBox("Information", "Download symbols function not yet implemented.", MB_OK | MB_ICONINFORMATION);
}

/*
 * Function: OnTransparencyChanged
 * Description: Handle transparency slider change event
 * Parameters: value - Slider value
 * Return: None
 */
void OnTransparencyChanged(int value) {
    char buffer[10];
    sprintf(buffer, "%d%%", value);
    SetWindowTextA(g_hTransparencyValue, buffer);
}

/*
 * Function: OnThemeChanged
 * Description: Handle theme combo box change event
 * Parameters: index - Selected index
 * Return: None
 */
void OnThemeChanged(int index) {
    UNREFERENCED_PARAMETER(index);
    /* Theme change handling */
    /* Theme switching logic can be added here */
}

/*
 * Function: CreateCustomFont
 * Description: Create a custom font with specified size and style
 * Parameters:
 *   size - Font size
 *   bold - TRUE for bold font, FALSE for regular
 *   italic - TRUE for italic font, FALSE for regular
 * Return: HFONT - Handle to the created font, or NULL on failure
 */
HFONT CreateCustomFont(int size, BOOL bold, BOOL italic) {
    LOGFONT lf = {0};

    lf.lfHeight = -MulDiv(size, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72);
    lf.lfWidth = 0;
    lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
    lf.lfItalic = (BYTE)italic;
    lf.lfUnderline = 0;
    lf.lfStrikeOut = 0;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = CLEARTYPE_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
    strcpy(lf.lfFaceName, FONT_FAMILY);
    
    return CreateFontIndirect(&lf);
}

/*
 * Function: SetControlStyles
 * Description: Set modern styles for a control
 * Parameters:
 *   hWnd - Control handle
 *   isButton - TRUE if control is a button, FALSE otherwise
 * Return: None
 */
void SetControlStyles(HWND hWnd, BOOL isButton) {
    if (isButton) {
        /* Set button styles */
        SetWindowLongPtr(hWnd, GWL_STYLE, 
                        GetWindowLongPtr(hWnd, GWL_STYLE) | BS_FLAT);
        
        /* Set button colors */
        SendMessage(hWnd, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, TRUE, FALSE), TRUE);
    } else {
        /* Set common control styles */
        SendMessage(hWnd, WM_SETFONT, (WPARAM)CreateCustomFont(FONT_SIZE_NORMAL, FALSE, FALSE), TRUE);
    }
    
    /* Set text color for better readability is handled elsewhere */
}
