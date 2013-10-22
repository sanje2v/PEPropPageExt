#pragma once

#include "stdafx.h"
#include "PEPropPageExt_i.h"
#include "PEReadWrite.h"
#include <ShlObj.h>

using namespace ATL;


// CPEPropPageExt
class ATL_NO_VTABLE CPEPropPageExt :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPEPropPageExt, &CLSID_PEPropPageExt>,
	public IShellExtInit,
	public IShellPropSheetExt
{
private:
	HMODULE hRichEditDll;
	PEReadWrite PEReaderWriter;

	static UINT CALLBACK PropertyPageLifeEventCallback(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
	static INT_PTR CALLBACK PropertyPagesProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	CPEPropPageExt() {}

	DECLARE_REGISTRY_RESOURCEID(IDR_PEPROPPAGEEXT)
	DECLARE_NOT_AGGREGATABLE(CPEPropPageExt)

	BEGIN_COM_MAP(CPEPropPageExt)
		COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY(IShellPropSheetExt)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

    // IShellExtInit
    STDMETHODIMP Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

    // IShellPropSheetExt
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE, LPARAM);
    STDMETHODIMP ReplacePage(UINT, LPFNADDPROPSHEETPAGE, LPARAM) { return E_NOTIMPL; }
};

OBJECT_ENTRY_AUTO(__uuidof(PEPropPageExt), CPEPropPageExt)