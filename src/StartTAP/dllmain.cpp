#include "framework.h"
#include "VisualTreeWatcher.h"
#include "helpers.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// StartTAP implements IObjectWithSite to receive the IXamlDiagnostics site
struct StartTAP : winrt::implements<StartTAP, IObjectWithSite>
{
    HRESULT STDMETHODCALLTYPE SetSite(IUnknown* pUnkSite) noexcept override
    {
        if (!pUnkSite) return E_INVALIDARG;
        
        try
        {
            m_site.copy_from(pUnkSite);
            
            // Create VisualTreeWatcher
            auto treeWatcher = winrt::make_self<VisualTreeWatcher>(m_site);
            
            return S_OK;
        }
        catch (...)
        {
            return winrt::to_hresult();
        }
    }
    
    HRESULT STDMETHODCALLTYPE GetSite(REFIID riid, void** ppvSite) noexcept override
    {
        if (!ppvSite) return E_POINTER;
        
        if (!m_site)
        {
            *ppvSite = nullptr;
            return E_FAIL;
        }
        
        return m_site.as(riid, ppvSite);
    }

private:
    winrt::com_ptr<IUnknown> m_site;
};

// TAPFactory implements IClassFactory to create StartTAP instances
struct TAPFactory : winrt::implements<TAPFactory, IClassFactory>
{
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override
    {
        try
        {
            *ppvObject = nullptr;
            
            if (pUnkOuter)
            {
                return CLASS_E_NOAGGREGATION;
            }
            
            return winrt::make<StartTAP>().as(riid, ppvObject);
        }
        catch (...)
        {
            return winrt::to_hresult();
        }
    }
    
    HRESULT STDMETHODCALLTYPE LockServer(BOOL) noexcept override
    {
        return S_OK;
    }
};

// {36162BD3-3531-4131-9B8B-7FB1A991EF51}
static constexpr GUID tapFactory = 
{ 0x36162bd3, 0x3531, 0x4131, { 0x9b, 0x8b, 0x7f, 0xb1, 0xa9, 0x91, 0xef, 0x51 } };

_Use_decl_annotations_ STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    *ppv = nullptr;
    
    if (rclsid == tapFactory)
    {
        return winrt::make<TAPFactory>().as(riid, ppv);
    }
    
    return CLASS_E_CLASSNOTAVAILABLE;
}

_Use_decl_annotations_ STDAPI DllCanUnloadNow(void)
{
    return winrt::get_module_lock() ? S_FALSE : S_OK;
}