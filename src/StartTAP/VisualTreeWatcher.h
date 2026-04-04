#pragma once
#include "framework.h"

// export variables
extern int64_t token, token_vis;
inline double pad = 15.0;
inline FrameworkElement registeredPinnedList{ nullptr };
inline FrameworkElement registeredSuggestionsHeader{ nullptr };

extern double oldSrchHeight;
extern Thickness oldSrchMar;

extern DWORD dwOpacity, dwLuminosity, dwHide, dwBorder, dwRec;

struct VisualTreeWatcher : winrt::implements<VisualTreeWatcher, IVisualTreeServiceCallback2, winrt::non_agile>
{
public:
    explicit VisualTreeWatcher(winrt::com_ptr<IUnknown> const& site);
    ~VisualTreeWatcher() noexcept;
    VisualTreeWatcher(const VisualTreeWatcher&) = delete;
    VisualTreeWatcher& operator=(const VisualTreeWatcher&) = delete;

    VisualTreeWatcher(VisualTreeWatcher&&) = delete;
    VisualTreeWatcher& operator=(VisualTreeWatcher&&) = delete;

    HRESULT AdviseVisualTreeChange() noexcept;

private:
    HRESULT STDMETHODCALLTYPE OnVisualTreeChange(ParentChildRelation relation, VisualElement element, VisualMutationType mutationType) noexcept override;
    HRESULT STDMETHODCALLTYPE OnElementStateChanged(InstanceHandle element, VisualElementState elementState, LPCWSTR context) noexcept override;

    template<typename T>
    T TryFromHandle(InstanceHandle handle) const noexcept
    {
        T value{ nullptr };
        if (!m_XamlDiagnostics || handle == 0)
        {
            return value;
        }

        IInspectable obj{ nullptr };
        const auto hr = m_XamlDiagnostics->GetIInspectableFromHandle(handle, reinterpret_cast<::IInspectable**>(winrt::put_abi(obj)));
        if (FAILED(hr) || obj == nullptr)
        {
            return value;
        }

        value = obj.try_as<T>();
        return value;
    }

    winrt::com_ptr<IXamlDiagnostics> m_XamlDiagnostics;
    winrt::com_ptr<IVisualTreeService3> m_visualTreeService;
    bool m_isAdvised{ false };
};
