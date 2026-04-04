#include "VisualTreeWatcher.h"
#include "helpers.h"
#include "misc.h"
#include <algorithm>

namespace
{
std::wstring_view ToView(BSTR value) noexcept
{
    if (value == nullptr)
    {
        return {};
    }

    return { value, static_cast<size_t>(SysStringLen(value)) };
}

double CalculateSearchPad(Control const& searchControl) noexcept
{
    const auto baseHeight = searchControl.ActualHeight() > 0 ? searchControl.ActualHeight() : std::max(searchControl.Height(), 0.0);
    return baseHeight + searchControl.Padding().Bottom + searchControl.Padding().Top + 55.0;
}

AcrylicBrush TryGetAcrylicBrush(Border const& border) noexcept
{
    if (!border)
    {
        return nullptr;
    }

    return border.Background().try_as<AcrylicBrush>();
}

SolidColorBrush TryGetSolidColorBrush(Border const& border) noexcept
{
    if (!border)
    {
        return nullptr;
    }

    return border.Background().try_as<SolidColorBrush>();
}

void CollapseElement(FrameworkElement const& element) noexcept
{
    if (element)
    {
        element.Visibility(Visibility::Collapsed);
    }
}
}

DWORD dwOpacity = 0, dwLuminosity = 0, dwHide = 0, dwBorder = 0, dwRec = 0;

double oldSrchHeight = 0;
Thickness oldSrchMar{};
int64_t token = 0, token_vis = 0;


VisualTreeWatcher::VisualTreeWatcher(winrt::com_ptr<IUnknown> const& site)
    : m_XamlDiagnostics(site ? site.as<IXamlDiagnostics>() : nullptr),
    m_visualTreeService(m_XamlDiagnostics ? m_XamlDiagnostics.as<IVisualTreeService3>() : nullptr)
{
}

VisualTreeWatcher::~VisualTreeWatcher() noexcept
{
    if (m_visualTreeService && m_isAdvised)
    {
        const auto hr = m_visualTreeService->UnadviseVisualTreeChange(this);
        (void)hr;
    }
}

HRESULT VisualTreeWatcher::AdviseVisualTreeChange() noexcept
{
    if (!m_visualTreeService)
    {
        return E_NOINTERFACE;
    }

    if (m_isAdvised)
    {
        return S_FALSE;
    }

    const auto hr = m_visualTreeService->AdviseVisualTreeChange(this);
    if (SUCCEEDED(hr))
    {
        m_isAdvised = true;
    }

    return hr;
}

HRESULT VisualTreeWatcher::OnElementStateChanged(InstanceHandle, VisualElementState, LPCWSTR) noexcept
{
    return S_OK;
}

HRESULT VisualTreeWatcher::OnVisualTreeChange(ParentChildRelation relation, VisualElement element, VisualMutationType mutationType) noexcept
{
    try
    {
        if (mutationType != Add)
        {
            return S_OK;
        }

        const auto type = ToView(element.Type);
        const auto name = ToView(element.Name);
        if (name == L"AcrylicBorder")
        {
            auto acrylicBorder = TryFromHandle<Border>(element.Handle);
            auto acrylicBrush = TryGetAcrylicBrush(acrylicBorder);
            if (!acrylicBrush)
            {
                return S_OK;
            }

            dwOpacity = std::min(GetVal(L"TintOpacity"), 100UL);
            dwLuminosity = std::min(GetVal(L"TintLuminosityOpacity"), 100UL);
            acrylicBrush.TintOpacity(static_cast<double>(dwOpacity) / 100.0);
            acrylicBrush.TintLuminosityOpacity(static_cast<double>(dwLuminosity) / 100.0);
        }
        else if (name == L"BackgroundElement" && type == L"Windows.UI.Xaml.Controls.Border")
        {
            auto backElement = TryFromHandle<Border>(element.Handle);
            if (auto brush = TryGetAcrylicBrush(backElement))
            {
                brush.TintOpacity(0);
            }
        }
        else if (type == L"StartDocked.SearchBoxToggleButton")
        {
            dwHide = GetVal(L"HideSearch");
            auto srch = TryFromHandle<Control>(element.Handle);
            if (!srch)
            {
                return S_OK;
            }

            oldSrchMar = srch.Margin();
            oldSrchHeight = srch.Height();
            if (dwHide == 1)
            {
                srch.Height(0);
                srch.Margin({ 0 });
                pad = CalculateSearchPad(srch);
            }
        }
        else if (name == L"RootGrid")
        {
            auto rootContent = TryFromHandle<Grid>(element.Handle);
            if (rootContent && GetVal(L"EditButton") == 1)
            {
                AddSettingsPanel(rootContent);
            }
        }
        else if (name == L"AcrylicOverlay")
        {
            dwBorder = GetVal(L"HideBorder");
            auto acrylicOverlay = TryFromHandle<Border>(element.Handle);
            if (dwBorder == 1)
            {
                if (auto brush = TryGetSolidColorBrush(acrylicOverlay))
                {
                    brush.Opacity(0);
                }
            }
        }
        else if (name == L"SuggestionsParentContainer" || name == L"ShowMoreSuggestions")
        {
            dwRec = GetVal(L"HideRecommended");
            auto elementRef = TryFromHandle<FrameworkElement>(element.Handle);
            if (dwRec == 1)
            {
                CollapseElement(elementRef);
            }
        }
        else if (name == L"TopLevelSuggestionsListHeader")
        {
            dwRec = GetVal(L"HideRecommended");
            auto elementRef = TryFromHandle<FrameworkElement>(element.Handle);
            if (!elementRef)
            {
                return S_OK;
            }

            if (dwRec == 1)
            {
                CollapseElement(elementRef);
                if (registeredSuggestionsHeader != elementRef && registeredSuggestionsHeader && token_vis != 0)
                {
                    registeredSuggestionsHeader.UnregisterPropertyChangedCallback(UIElement::VisibilityProperty(), token_vis);
                    token_vis = 0;
                }

                registeredSuggestionsHeader = elementRef;
                if (token_vis == 0)
                {
                    token_vis = elementRef.RegisterPropertyChangedCallback(
                        UIElement::VisibilityProperty(),
                        [](DependencyObject sender, DependencyProperty property)
                        {
                            auto current = sender.try_as<FrameworkElement>();
                            if (current)
                            {
                                current.Visibility(Visibility::Collapsed);
                            }
                        });
                }
            }
        }
        else if (name == L"StartMenuPinnedList")
        {
            dwRec = GetVal(L"HideRecommended");
            if (dwRec == 1)
            {
                auto topLevelRoot = TryFromHandle<FrameworkElement>(relation.Parent);
                auto pinList = TryFromHandle<FrameworkElement>(element.Handle);
                if (!topLevelRoot || !pinList)
                {
                    return S_OK;
                }

                auto suggHeader = FindDescendantByName(topLevelRoot, L"TopLevelSuggestionsListHeader").try_as<FrameworkElement>();
                auto suggContainer = FindDescendantByName(topLevelRoot, L"SuggestionsParentContainer").try_as<FrameworkElement>();
                auto suggBtn = FindDescendantByName(topLevelRoot, L"ShowMoreSuggestions").try_as<FrameworkElement>();

                const double height = pinList.Height()
                    + (suggContainer ? suggContainer.ActualHeight() : 0.0)
                    + (suggBtn ? suggBtn.ActualHeight() : 0.0);

                if (registeredPinnedList != pinList && registeredPinnedList && token != 0)
                {
                    registeredPinnedList.UnregisterPropertyChangedCallback(FrameworkElement::HeightProperty(), token);
                    token = 0;
                }

                registeredPinnedList = pinList;
                if (token == 0)
                {
                    token = pinList.RegisterPropertyChangedCallback(
                        FrameworkElement::HeightProperty(),
                        [height](DependencyObject sender, DependencyProperty property)
                        {
                            auto current = sender.try_as<FrameworkElement>();
                            if (current)
                            {
                                current.Height(height + pad);
                            }
                        });
                }

                pinList.Height(height + pad);
                CollapseElement(suggHeader);
                CollapseElement(suggContainer);
                CollapseElement(suggBtn);
            }
        }

        return S_OK;
    }
    catch (...)
    {
        return S_OK;
    }
}
