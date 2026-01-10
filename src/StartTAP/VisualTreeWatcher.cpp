#include "VisualTreeWatcher.h"
#include "helpers.h"

// Declaration for GetIUnknownFromHandle function (from XAML Diagnostics API)
typedef HRESULT (WINAPI *GetIUnknownFromHandleProto)(InstanceHandle handle, IUnknown** ppUnknown);

// Global GetIUnknownFromHandle function pointer
static GetIUnknownFromHandleProto GetIUnknownFromHandleFn = nullptr;

// Template function to convert VisualElement handle to XAML object
template<typename T>
static T FromHandle(InstanceHandle handle)
{
    if (!handle) return nullptr;
    
    try
    {
        // Initialize GetIUnknownFromHandle function pointer if needed
        if (!GetIUnknownFromHandleFn)
        {
            HMODULE hXamlDll = LoadLibraryEx(L"Windows.UI.Xaml.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (hXamlDll)
            {
                GetIUnknownFromHandleFn = (GetIUnknownFromHandleProto)GetProcAddress(hXamlDll, "GetIUnknownFromHandle");
                // We intentionally don't free the library to keep the function pointer valid
            }
        }
        
        if (!GetIUnknownFromHandleFn) return nullptr;
        
        // Get the IUnknown from the handle
        winrt::com_ptr<IUnknown> unk;
        HRESULT hr = GetIUnknownFromHandleFn(handle, unk.put());
        if (FAILED(hr)) return nullptr;
        
        // Query for the specific interface
        return unk.as<T>();
    }
    catch (...)
    {
        return nullptr;
    }
}

// Template function to find descendant by name
template<typename T>
static T FindDescendantByName(DependencyObject root, hstring name)
{
    if (!root) return nullptr;
    
    try
    {
        int count = VisualTreeHelper::GetChildrenCount(root);
        for (int i = 0; i < count; i++)
        {
            auto child = VisualTreeHelper::GetChild(root, i);
            if (!child) continue;
            
            auto frameworkElement = child.try_as<FrameworkElement>();
            if (frameworkElement && frameworkElement.Name() == name)
            {
                return child.as<T>();
            }
            
            auto result = FindDescendantByName<T>(child, name);
            if (result) return result;
        }
    }
    catch (...)
    {
        // Handle exceptions
    }
    
    return nullptr;
}

// Windows 10 specific fix for acrylic background
static void ApplyWindows10Fix(const VisualElement& element)
{
    try
    {
        auto border = FromHandle<Border>(element.Handle);
        if (border && border.Background())
        {
            auto acrylicBrush = border.Background().try_as<AcrylicBrush>();
            if (acrylicBrush)
            {
                acrylicBrush.TintOpacity(0.0);
            }
        }
    }
    catch (...)
    {
        // Handle exceptions
    }
}

VisualTreeWatcher::VisualTreeWatcher(winrt::com_ptr<IUnknown> site)
    : m_site(site)
    , m_heightToken(0)
    , m_visibilityToken(0)
    , m_pad(15.0)
    , m_oldSearchHeight(0.0)
{
    // Get IXamlDiagnostics interface
    m_xamlDiagnostics = site.as<IXamlDiagnostics>();
    
    // Load current configuration
    m_tintOpacity = GetRegistryValue(L"TintOpacity", 30);
    m_tintLuminosityOpacity = GetRegistryValue(L"TintLuminosityOpacity", 30);
    m_hideSearch = GetRegistryValue(L"HideSearch", 0);
    m_hideBorder = GetRegistryValue(L"HideBorder", 0);
    m_hideRecommended = GetRegistryValue(L"HideRecommended", 0);
    m_editButton = GetRegistryValue(L"EditButton", 1);
    
    // Start monitoring in a separate thread
    HANDLE thread = CreateThread(nullptr, 0, [](LPVOID param) -> DWORD {
        auto watcher = reinterpret_cast<VisualTreeWatcher*>(param);
        watcher->AdviseVisualTreeChange();
        return 0;
    }, this, 0, nullptr);
    
    if (thread)
    {
        CloseHandle(thread);
    }
}

VisualTreeWatcher::~VisualTreeWatcher()
{
    if (m_xamlDiagnostics)
    {
        // Unregister from visual tree changes if needed
        auto treeService = m_xamlDiagnostics.as<IVisualTreeService3>();
        if (treeService)
        {
            treeService->UnadviseVisualTreeChange(this);
        }
    }
}

void VisualTreeWatcher::AdviseVisualTreeChange()
{
    if (!m_xamlDiagnostics) return;
    
    try
    {
        auto treeService = m_xamlDiagnostics.as<IVisualTreeService3>();
        if (treeService)
        {
            treeService->AdviseVisualTreeChange(this);
        }
    }
    catch (...)
    {
        // Handle exception
    }
}

HRESULT VisualTreeWatcher::OnVisualTreeChange(ParentChildRelation relation, VisualElement element, VisualMutationType mutationType)
{
    if (mutationType != VisualMutationType::Add) return S_OK;
    
    try
    {
        std::wstring_view type(element.Type, SysStringLen(element.Type));
        std::wstring_view name(element.Name, SysStringLen(element.Name));
        
        // Apply settings based on element type and name
        if (name == L"AcrylicBorder")
        {
            ApplyTransparencySettings(element);
        }
        else if (name == L"BackgroundElement" && type == L"Windows.UI.Xaml.Controls.Border")
        {
            // Windows 10 fix
            ApplyWindows10Fix(element);
        }
        else if (type == L"StartDocked.SearchBoxToggleButton")
        {
            ApplySearchBoxSettings(element);
        }
        else if (name == L"RootGrid")
        {
            if (m_editButton == 1)
            {
                AddSettingsPanel(element);
            }
        }
        else if (name == L"AcrylicOverlay")
        {
            ApplyBorderSettings(element);
        }
        else if (name == L"SuggestionsParentContainer" || name == L"ShowMoreSuggestions")
        {
            if (m_hideRecommended == 1)
            {
                auto frameworkElement = FromHandle<FrameworkElement>(element.Handle);
                if (frameworkElement)
                {
                    frameworkElement.Visibility(Visibility::Collapsed);
                }
            }
        }
        else if (name == L"TopLevelSuggestionsListHeader")
        {
            if (m_hideRecommended == 1)
            {
                auto frameworkElement = FromHandle<FrameworkElement>(element.Handle);
                if (frameworkElement)
                {
                    frameworkElement.Visibility(Visibility::Collapsed);
                    
                    if (m_visibilityToken == 0)
                    {
                        m_visibilityToken = frameworkElement.RegisterPropertyChangedCallback(
                            UIElement::VisibilityProperty(),
                            [](DependencyObject sender, DependencyProperty property) {
                                auto element = sender.try_as<FrameworkElement>();
                                if (element)
                                {
                                    element.Visibility(Visibility::Collapsed);
                                }
                            });
                    }
                }
            }
        }
        else if (name == L"StartMenuPinnedList")
        {
            ApplyRecommendedSettings(element);
        }
    }
    catch (...)
    {
        // Handle any exceptions during element processing
    }
    
    return S_OK;
}

HRESULT VisualTreeWatcher::OnElementStateChanged(InstanceHandle handle, VisualElementState state, LPCWSTR context)
{
    return S_OK;
}

void VisualTreeWatcher::ApplyTransparencySettings(const VisualElement& element)
{
    try
    {
        auto acrylicBorder = FromHandle<Border>(element.Handle);
        if (!acrylicBorder) return;
        
        auto background = acrylicBorder.Background();
        if (!background) return;
        
        auto acrylicBrush = background.try_as<AcrylicBrush>();
        if (!acrylicBrush) return;
        
        // Apply opacity settings
        acrylicBrush.TintOpacity(OpacityToDouble(m_tintOpacity));
        acrylicBrush.TintLuminosityOpacity(OpacityToDouble(m_tintLuminosityOpacity));
    }
    catch (...)
    {
        // Handle exceptions
    }
}

void VisualTreeWatcher::ApplySearchBoxSettings(const VisualElement& element)
{
    try
    {
        auto searchBox = FromHandle<Control>(element.Handle);
        if (!searchBox) return;
        
        // Store original values if not already stored
        if (m_oldSearchHeight == 0.0)
        {
            m_oldSearchHeight = searchBox.Height();
            m_oldSearchMargin = searchBox.Margin();
        }
        
        if (m_hideSearch == 1)
        {
            searchBox.Height(0);
            searchBox.Margin({ 0, 0, 0, 0 });
            
            // Calculate padding for recommended section
            m_pad = searchBox.ActualHeight() + 
                   searchBox.Padding().Bottom + 
                   searchBox.Padding().Top + 55;
        }
        else
        {
            searchBox.Height(m_oldSearchHeight);
            searchBox.Margin(m_oldSearchMargin);
            m_pad = 15;
        }
    }
    catch (...)
    {
        // Handle exceptions
    }
}

void VisualTreeWatcher::ApplyRecommendedSettings(const VisualElement& element)
{
    if (m_hideRecommended != 1) return;
    
    try
    {
        auto pinList = FromHandle<FrameworkElement>(element.Handle);
        if (!pinList) return;
        
        auto parent = pinList.Parent();
        if (!parent) return;
        
        auto topLevelRoot = parent.try_as<FrameworkElement>();
        if (!topLevelRoot) return;
        
        auto suggHeader = FindDescendantByName<FrameworkElement>(topLevelRoot, L"TopLevelSuggestionsListHeader");
        auto suggContainer = FindDescendantByName<FrameworkElement>(topLevelRoot, L"SuggestionsParentContainer");
        auto suggBtn = FindDescendantByName<FrameworkElement>(topLevelRoot, L"ShowMoreSuggestions");
        
        if (!suggHeader || !suggContainer || !suggBtn) return;
        
        // Calculate new height for pinned list
        double height = pinList.Height() + suggContainer.ActualHeight() + suggBtn.ActualHeight();
        
        // Store current pinList in a shared_ptr to capture in lambda
        auto sharedPinList = std::make_shared<FrameworkElement>(pinList);
        auto sharedPad = std::make_shared<double>(m_pad);
        
        if (m_heightToken == 0)
        {
            m_heightToken = pinList.RegisterPropertyChangedCallback(
                FrameworkElement::HeightProperty(),
                [sharedPinList, sharedPad](DependencyObject sender, DependencyProperty property) {
                    auto element = sender.try_as<FrameworkElement>();
                    if (element)
                    {
                        // Use the captured pad value
                        element.Height(element.Height() + *sharedPad);
                    }
                });
        }
        
        // Apply new height with padding
        pinList.Height(height + m_pad);
        
        // Hide recommended section elements
        suggHeader.Visibility(Visibility::Collapsed);
        suggContainer.Visibility(Visibility::Collapsed);
        suggBtn.Visibility(Visibility::Collapsed);
    }
    catch (...)
    {
        // Handle exceptions
    }
}

void VisualTreeWatcher::ApplyBorderSettings(const VisualElement& element)
{
    if (m_hideBorder != 1) return;
    
    try
    {
        auto acrylicOverlay = FromHandle<Border>(element.Handle);
        if (!acrylicOverlay) return;
        
        auto background = acrylicOverlay.Background();
        if (!background) return;
        
        auto solidBrush = background.try_as<SolidColorBrush>();
        if (solidBrush)
        {
            solidBrush.Opacity(0.0);
        }
    }
    catch (...)
    {
        // Handle exceptions
    }
}

void VisualTreeWatcher::AddSettingsPanel(const VisualElement& element)
{
    try
    {
        auto rootGrid = FromHandle<Grid>(element.Handle);
        if (!rootGrid) return;
        
        // Create settings button
        auto button = Button();
        button.BorderThickness(Thickness{ 0 });
        button.Background(SolidColorBrush(Colors::Transparent()));
        
        // Create icon
        auto fontIcon = FontIcon();
        fontIcon.FontSize(16);
        fontIcon.Glyph(L"\uE104"); // Settings icon
        fontIcon.FontFamily(Media::FontFamily(L"Segoe Fluent Icons"));
        
        button.Content(fontIcon);
        button.ToolTip(box_value(L"TranslucentSM settings"));
        
        // Create settings panel
        auto stackPanel = StackPanel();
        
        // Tint Opacity control
        auto tintOpacityText = TextBlock();
        tintOpacityText.FontFamily(Media::FontFamily(L"Segoe UI Variable"));
        tintOpacityText.FontSize(13.0);
        tintOpacityText.Text(L"TintOpacity");
        stackPanel.Children().Append(tintOpacityText);
        
        auto tintOpacitySlider = Slider();
        tintOpacitySlider.Width(140);
        tintOpacitySlider.Value(m_tintOpacity);
        stackPanel.Children().Append(tintOpacitySlider);
        
        // Tint Luminosity Opacity control
        auto tintLuminosityText = TextBlock();
        tintLuminosityText.FontFamily(Media::FontFamily(L"Segoe UI Variable"));
        tintLuminosityText.FontSize(13.0);
        tintLuminosityText.Text(L"TintLuminosityOpacity");
        stackPanel.Children().Append(tintLuminosityText);
        
        auto tintLuminositySlider = Slider();
        tintLuminositySlider.Width(140);
        tintLuminositySlider.Value(m_tintLuminosityOpacity);
        stackPanel.Children().Append(tintLuminositySlider);
        
        // Event handlers
        tintOpacitySlider.ValueChanged([this](IInspectable const&, RoutedEventArgs const&) {
            // This would need access to the acrylic border, which is tricky
            // For now, just update registry
            // In a full implementation, we'd need to find the acrylic border again
        });
        
        tintLuminositySlider.ValueChanged([this](IInspectable const&, RoutedEventArgs const&) {
            // Similar to above
        });
        
        // Add to UI
        // Note: This is simplified - full implementation would need to handle
        // Windows 10 vs Windows 11 differences
        auto flyout = Flyout();
        flyout.Content(stackPanel);
        button.Flyout(flyout);
        
        // Find appropriate parent and add button
        // This would need more specific logic based on Windows version
    }
    catch (...)
    {
        // Handle exceptions
    }
}