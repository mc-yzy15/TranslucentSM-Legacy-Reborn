#include "misc.h"
#include <algorithm>

namespace
{
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
}

HRESULT AddSettingsPanel(Grid rootGrid)
{
    if (!rootGrid)
    {
        return E_INVALIDARG;
    }

    if (FindDescendantByName(rootGrid, L"TranslucentSMSettingsButton") != nullptr)
    {
        return S_FALSE;
    }

    dwOpacity = GetVal(L"TintOpacity");
    dwLuminosity = GetVal(L"TintLuminosityOpacity");
    dwHide = GetVal(L"HideSearch");
    dwBorder = GetVal(L"HideBorder");
    dwRec = GetVal(L"HideRecommended");

    auto acrylicBorder = FindDescendantByName(rootGrid, L"AcrylicBorder").try_as<Border>();

    Button bt;
    bt.Name(L"TranslucentSMSettingsButton");
    auto f = FontIcon();
    f.FontSize(16);
    bt.BorderThickness(Thickness{ 0 });
    bt.Background(SolidColorBrush(Colors::Transparent()));
    auto stackPanel = StackPanel();

    TextBlock tbx;
    tbx.FontFamily(Windows::UI::Xaml::Media::FontFamily(L"Segoe UI Variable"));
    tbx.FontSize(13.0);
    tbx.Text(L"TintOpacity");
    stackPanel.Children().Append(tbx);

    Slider slider;
    slider.Width(140);
    slider.Value(dwOpacity);
    stackPanel.Children().Append(slider);

    TextBlock tbx1;
    tbx1.FontFamily(Windows::UI::Xaml::Media::FontFamily(L"Segoe UI Variable"));
    tbx1.FontSize(13.0);
    tbx1.Text(L"TintLuminosityOpacity");
    stackPanel.Children().Append(tbx1);

    Slider slider2;
    slider2.Width(140);
    slider2.Value(dwLuminosity);
    stackPanel.Children().Append(slider2);

    slider.ValueChanged([acrylicBorder](Windows::Foundation::IInspectable const& sender, RangeBaseValueChangedEventArgs const&)
        {
            double sliderValue = sender.as<Slider>().Value();
            if (auto brush = TryGetAcrylicBrush(acrylicBorder))
            {
                brush.TintOpacity(sliderValue / 100.0);
            }

            SetVal(L"TintOpacity", static_cast<DWORD>(sliderValue));
        });

    slider2.ValueChanged([acrylicBorder](Windows::Foundation::IInspectable const& sender, RangeBaseValueChangedEventArgs const&)
        {
            double sliderValue = sender.as<Slider>().Value();
            if (auto brush = TryGetAcrylicBrush(acrylicBorder))
            {
                brush.TintLuminosityOpacity(sliderValue / 100.0);
            }

            SetVal(L"TintLuminosityOpacity", static_cast<DWORD>(sliderValue));
        });

    auto srchBoxElm = FindDescendantByName(rootGrid, L"StartMenuSearchBox");
    if (srchBoxElm != nullptr)
    {
        auto srchBox = srchBoxElm.try_as<Control>();

        auto checkBox = CheckBox();
        checkBox.Content(box_value(L"Hide search box"));
        stackPanel.Children().Append(checkBox);
        if (dwHide == 1 && srchBox)
        {
            checkBox.IsChecked(true);
            pad = CalculateSearchPad(srchBox);
        }

        checkBox.Checked([srchBox](Windows::Foundation::IInspectable const& sender, RoutedEventArgs const&)
            {
                SetVal(L"HideSearch", 1);
                if (!srchBox)
                {
                    return;
                }

                srchBox.Height(0);
                srchBox.Margin({ 0 });
                pad = CalculateSearchPad(srchBox);
            });

        checkBox.Unchecked([srchBox](Windows::Foundation::IInspectable const& sender, RoutedEventArgs const&)
            {
                SetVal(L"HideSearch", 0);
                if (!srchBox)
                {
                    return;
                }

                srchBox.Height(oldSrchHeight);
                srchBox.Margin(oldSrchMar);
                pad = 15.0;
            });
    }

    auto acrylicOverlayElm = FindDescendantByName(rootGrid, L"AcrylicOverlay");
    if (acrylicOverlayElm != nullptr)
    {
        auto acrylicOverlay = acrylicOverlayElm.try_as<Border>();

        auto checkBox = CheckBox();
        checkBox.Content(box_value(L"Hide white border"));
        stackPanel.Children().Append(checkBox);
        if (dwBorder == 1)
            checkBox.IsChecked(true);

        checkBox.Checked([acrylicOverlay](Windows::Foundation::IInspectable const& sender, RoutedEventArgs const&)
            {
                SetVal(L"HideBorder", 1);
                if (auto brush = TryGetSolidColorBrush(acrylicOverlay))
                {
                    brush.Opacity(0);
                }
            });

        checkBox.Unchecked([acrylicOverlay](Windows::Foundation::IInspectable const& sender, RoutedEventArgs const&)
            {
                SetVal(L"HideBorder", 0);
                if (auto brush = TryGetSolidColorBrush(acrylicOverlay))
                {
                    brush.Opacity(1);
                }
            });
    }

    auto topRootElm = FindDescendantByName(rootGrid, L"TopLevelRoot");
    if (auto topRoot = topRootElm.try_as<Grid>())
    {
        auto checkBox = CheckBox();
        checkBox.Content(box_value(L"Hide recommended"));
        stackPanel.Children().Append(checkBox);
        if (dwRec == 1)
        {
            checkBox.IsChecked(true);
        }

        auto suggContainer = FindDescendantByName(topRoot, L"SuggestionsParentContainer").try_as<FrameworkElement>();
        auto suggBtn = FindDescendantByName(topRoot, L"ShowMoreSuggestions").try_as<FrameworkElement>();
        auto suggHeader = FindDescendantByName(topRoot, L"TopLevelSuggestionsListHeader").try_as<FrameworkElement>();
        auto pinList = FindDescendantByName(topRoot, L"StartMenuPinnedList").try_as<FrameworkElement>();

        const double pinHeight = pinList ? pinList.Height() : 0.0;
        const double hiddenHeight = pinHeight
            + (suggContainer ? suggContainer.ActualHeight() : 0.0)
            + (suggBtn ? suggBtn.ActualHeight() : 0.0);

        checkBox.Checked([pinList, suggHeader, suggContainer, suggBtn, hiddenHeight](Windows::Foundation::IInspectable const& sender, RoutedEventArgs const&)
            {
                SetVal(L"HideRecommended", 1);

                if (registeredPinnedList != pinList && registeredPinnedList && token != 0)
                {
                    registeredPinnedList.UnregisterPropertyChangedCallback(FrameworkElement::HeightProperty(), token);
                    token = 0;
                }

                registeredPinnedList = pinList;
                if (pinList && token == 0)
                {
                    token = pinList.RegisterPropertyChangedCallback(
                        FrameworkElement::HeightProperty(),
                        [hiddenHeight](DependencyObject sender, DependencyProperty property)
                        {
                            auto current = sender.try_as<FrameworkElement>();
                            if (current)
                            {
                                current.Height(hiddenHeight + pad);
                            }
                        });
                }

                if (registeredSuggestionsHeader != suggHeader && registeredSuggestionsHeader && token_vis != 0)
                {
                    registeredSuggestionsHeader.UnregisterPropertyChangedCallback(UIElement::VisibilityProperty(), token_vis);
                    token_vis = 0;
                }

                registeredSuggestionsHeader = suggHeader;
                if (suggHeader && token_vis == 0)
                {
                    token_vis = suggHeader.RegisterPropertyChangedCallback(
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

                if (pinList)
                {
                    pinList.Height(hiddenHeight + pad);
                }
                if (suggHeader)
                {
                    suggHeader.Visibility(Visibility::Collapsed);
                }
                if (suggContainer)
                {
                    suggContainer.Visibility(Visibility::Collapsed);
                }
                if (suggBtn)
                {
                    suggBtn.Visibility(Visibility::Collapsed);
                }
            });

        checkBox.Unchecked([pinList, suggHeader, suggContainer, suggBtn, pinHeight](Windows::Foundation::IInspectable const& sender, RoutedEventArgs const&)
            {
                SetVal(L"HideRecommended", 0);

                if (suggContainer)
                {
                    suggContainer.Visibility(Visibility::Visible);
                }
                if (suggBtn)
                {
                    suggBtn.Visibility(Visibility::Visible);
                }

                if (registeredPinnedList == pinList && pinList && token != 0)
                {
                    pinList.UnregisterPropertyChangedCallback(FrameworkElement::HeightProperty(), token);
                    token = 0;
                    registeredPinnedList = nullptr;
                }
                if (registeredSuggestionsHeader == suggHeader && suggHeader && token_vis != 0)
                {
                    suggHeader.UnregisterPropertyChangedCallback(UIElement::VisibilityProperty(), token_vis);
                    token_vis = 0;
                    registeredSuggestionsHeader = nullptr;
                }

                if (pinList)
                {
                    pinList.Height(pinHeight);
                }
                if (suggHeader)
                {
                    suggHeader.Visibility(Visibility::Visible);
                }
            });
    }

    auto nvds = FindDescendantByName(rootGrid, L"RootPanel");
    auto nvpane = FindDescendantByName(rootGrid, L"NavigationPanePlacesListView");
    auto rootpanel = FindDescendantByName(nvpane, L"Root");
    // windows 11
    if (rootpanel != nullptr)
    {
        Grid grid{ nullptr };
        if (VisualTreeHelper::GetChildrenCount(rootpanel) > 0)
        {
            grid = VisualTreeHelper::GetChild(rootpanel, 0).try_as<Grid>();
        }

        if (grid != nullptr)
        {
            ToolTipService::SetToolTip(bt, box_value(L"TranslucentSM settings"));

            f.Glyph(L"\uE104");
            f.FontFamily(Media::FontFamily(L"Segoe Fluent Icons"));

            bt.Content(winrt::box_value(f));
            bt.Margin({ -40,0,0,0 });
            bt.Padding({ 11.2,11.2,11.2,11.2 });
            bt.Width(38);
            bt.BorderBrush(SolidColorBrush(Colors::Transparent()));
            grid.Children().Append(bt);

            Flyout flyout;
            flyout.Content(stackPanel);
            bt.Flyout(flyout);
        }
    }
    // windows 10
    else if (auto nvGrid = nvds.try_as<Grid>())
    {
        f.Glyph(L"\uE70F");
        f.FontFamily(Media::FontFamily(L"Segoe MDL2 Assets"));

        RevealBorderBrush rv;
        rv.TargetTheme(ApplicationTheme::Dark);

        RevealBorderBrush rvb;
        rvb.TargetTheme(ApplicationTheme::Dark);
        rvb.Color(ColorHelper::FromArgb(128, 255, 255, 255));
        rvb.Opacity(0.4);

        bt.Resources().Insert(winrt::box_value(L"ButtonBackgroundPointerOver"), rvb);
        bt.Resources().Insert(winrt::box_value(L"ButtonBackgroundPressed"), rvb);

        f.HorizontalAlignment(HorizontalAlignment::Left);
        f.Margin({ 11.2,11.2,11.2,11.2 });

        auto mainpanel = StackPanel();
        mainpanel.Margin({ 0,-94,0,0 });
        mainpanel.Height(45);
        mainpanel.BorderThickness({ 1,1,1,1 });
        mainpanel.BorderBrush(rv);
        Grid::SetRow(mainpanel, 3);
        Canvas::SetZIndex(mainpanel, 10);
        mainpanel.Children().Append(bt);

        auto textBlock = TextBlock();
        textBlock.Text(L"TranslucentSM settings");
        textBlock.Margin({ 5,13,0,0 });
        textBlock.HorizontalAlignment(HorizontalAlignment::Left);
        textBlock.FontSize(15);

        auto panel = StackPanel();
        panel.Margin({ -4,-4,0,0 });
        panel.Children().Append(f);
        panel.Children().Append(textBlock);
        panel.Orientation(Orientation::Horizontal);
        panel.Height(45);
        panel.Width(256);
        panel.HorizontalAlignment(HorizontalAlignment::Left);

        bt.Height(45);
        bt.Width(256);
        bt.Content(panel);

        slider.Width(230);
        slider2.Width(230);

        Flyout flyout;
        flyout.Content(stackPanel);
        bt.Flyout(flyout);

        nvGrid.Children().InsertAt(0, mainpanel);
    }

    return S_OK;
}
