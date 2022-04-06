//
// Created by tjdic on 06/19/2021.
//

#include "tjd_win32_download.h"

#include <windows.h>
#include <wininet.h>
#include <cassert>

class WinBindStatusCallback : public IBindStatusCallback
{
public:
    STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown* punk)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(OnStartBinding)(
        /* [in] */ DWORD dwReserved,
        /* [in] */ IBinding __RPC_FAR* pib
    )
    {
        return E_NOTIMPL;
    }

    STDMETHOD(GetPriority)(
        /* [out] */ LONG __RPC_FAR* pnPriority
    )
    {
        return E_NOTIMPL;
    }

    STDMETHOD(OnLowResource)(
        /* [in] */ DWORD reserved
    )
    {
        return E_NOTIMPL;
    }

    STDMETHOD(OnProgress)(
        /* [in] */ ULONG ulProgress,
        /* [in] */ ULONG ulProgressMax,
        /* [in] */ ULONG ulStatusCode,
        /* [in] */ LPCWSTR wszStatusText
    );

    STDMETHOD(OnStopBinding)(
        /* [in] */ HRESULT hresult,
        /* [unique][in] */ LPCWSTR szError
    );

    STDMETHOD(GetBindInfo)(
        /* [out] */ DWORD __RPC_FAR* grfBINDF,
        /* [unique][out][in] */ BINDINFO __RPC_FAR* pbindinfo
    )
    {
        return E_NOTIMPL;
    }

    STDMETHOD(OnDataAvailable)(
        /* [in] */ DWORD grfBSCF,
        /* [in] */ DWORD dwSize,
        /* [in] */ FORMATETC __RPC_FAR* pformatetc,
        /* [in] */ STGMEDIUM __RPC_FAR* pstgmed
    )
    {
        return E_NOTIMPL;
    }

    STDMETHOD_(ULONG, AddRef)()
    {
        return 0;
    }

    STDMETHOD_(ULONG, Release)()
    {
        return 0;
    }

    STDMETHOD(QueryInterface)(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ void __RPC_FAR* __RPC_FAR* ppvObject
    )
    {
        return E_NOTIMPL;
    }
};

HRESULT WinBindStatusCallback::OnStopBinding(HRESULT hresult, LPCWSTR szError)
{
    const char* filename = "C:\\tmp\\testing_radar.nx3";
    sacw_RadarInit(filename, 94);
    return 0;
}

HRESULT WinBindStatusCallback::OnProgress(
    /* [in] */ ULONG ulProgress,
    /* [in] */ ULONG ulProgressMax,
    /* [in] */ ULONG ulStatusCode,
    /* [in] */ LPCWSTR wszStatusText
)
{
    printf("Progress: %d/%d\n", ulProgress, ulProgressMax);
    return 0;
}

void StartDownload(const char* siteName, NexradProduct* nexradProduct)
{
    assert(nexradProduct != nullptr);

    std::string file_url;
    GetUrlForProduct(&file_url, siteName, nexradProduct);

    g_CurrentSite = FindSiteByName(siteName);

    const char* filename = "C:\\tmp\\testing_radar.nx3";

    DeleteUrlCacheEntry(file_url.c_str());
    WinBindStatusCallback wbcb;
    URLDownloadToFile(nullptr, file_url.c_str(), filename, 0, &wbcb);
}


void StartDownload(RdaSite* rdaSite, NexradProduct* nexradProduct)
{
    StartDownload(rdaSite->name, nexradProduct);
}