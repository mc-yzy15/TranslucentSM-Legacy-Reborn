#pragma once
#include "framework.h"

static DependencyObject FindDescendantByName(DependencyObject const& root, hstring const& name)
{
    if (root == nullptr) return nullptr;

    const int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; i++)
    {
        DependencyObject child = VisualTreeHelper::GetChild(root, i);
        if (child == nullptr) continue;

        if (auto element = child.try_as<FrameworkElement>())
        {
            if (element.Name() == name)
                return child;
        }

        DependencyObject result = FindDescendantByName(child, name);
        if (result != nullptr)
            return result;
    }

    return nullptr;
}

static DWORD GetVal(LPCWSTR val)
{
    DWORD dw = 0;
    DWORD dwSize = sizeof(dw);
    const auto status = RegGetValueW(HKEY_CURRENT_USER, L"Software\\TranslucentSM", val, RRF_RT_DWORD, nullptr, &dw, &dwSize);
    return status == ERROR_SUCCESS ? dw : 0;
}

static HRESULT SetVal(LPCWSTR key, DWORD val)
{
    HKEY subkey = nullptr;
    const auto createStatus = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\TranslucentSM",
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        KEY_SET_VALUE,
        nullptr,
        &subkey,
        nullptr);
    if (createStatus != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(createStatus);
    }

    const auto setStatus = RegSetValueExW(subkey, key, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&val), sizeof(val));
    RegCloseKey(subkey);
    return HRESULT_FROM_WIN32(setStatus);
}
