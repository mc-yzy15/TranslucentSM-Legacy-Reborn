#pragma once
#include "framework.h"

class VisualTreeWatcher : public winrt::implements<VisualTreeWatcher, IVisualTreeServiceCallback2>
{
public:
    VisualTreeWatcher(winrt::com_ptr<IUnknown> site);
    ~VisualTreeWatcher();

    // IVisualTreeServiceCallback2 methods
    HRESULT STDMETHODCALLTYPE OnVisualTreeChange(ParentChildRelation relation, VisualElement element, VisualMutationType mutationType) override;
    HRESULT STDMETHODCALLTYPE OnElementStateChanged(InstanceHandle handle, VisualElementState state, LPCWSTR context) override;

private:
    void AdviseVisualTreeChange();
    
    // Helper methods
    void ApplyTransparencySettings(const VisualElement& element);
    void ApplySearchBoxSettings(const VisualElement& element);
    void ApplyRecommendedSettings(const VisualElement& element);
    void ApplyBorderSettings(const VisualElement& element);
    void AddSettingsPanel(const VisualElement& element);
    

    
    // Member variables
    winrt::com_ptr<IUnknown> m_site;
    winrt::com_ptr<IXamlDiagnostics> m_xamlDiagnostics;
    
    // State tracking
    double m_oldSearchHeight;
    Thickness m_oldSearchMargin;
    int64_t m_heightToken;
    int64_t m_visibilityToken;
    double m_pad;
    
    // Configuration cache
    DWORD m_tintOpacity;
    DWORD m_tintLuminosityOpacity;
    DWORD m_hideSearch;
    DWORD m_hideBorder;
    DWORD m_hideRecommended;
    DWORD m_editButton;
};