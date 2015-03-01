#pragma once
#include "stdafx.h"
#include "PEReadWrite.h"
#include <ShlObj.h>


// CPEPropPageExt
class ATL_NO_VTABLE CPEPropPageExt :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPEPropPageExt, &CLSID_PEPropPageExt>,
	public IShellExtInit,
	public IShellPropSheetExt
{
private:
	static UINT CALLBACK PropertyPageLifeEventCallback(HWND hWnd,
													   UINT uMsg,
													   LPPROPSHEETPAGE ppsp);
	static INT_PTR CALLBACK PropertyPagesProc(HWND hWnd,
											  UINT uMsg,
											  WPARAM wParam,
											  LPARAM lParam);

	PEReadWrite m_PEReaderWriter;

public:
	CPEPropPageExt();
	~CPEPropPageExt();

	DECLARE_REGISTRY_RESOURCEID(IDR_REGISTRATIONSCRIPT)
	DECLARE_NOT_AGGREGATABLE(CPEPropPageExt)

	BEGIN_COM_MAP(CPEPropPageExt)
		COM_INTERFACE_ENTRY(IShellExtInit)
		COM_INTERFACE_ENTRY(IShellPropSheetExt)
	END_COM_MAP()

	// IShellExtInit
	STDMETHODIMP Initialize(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

	// IShellPropSheetExt
	STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE, LPARAM);
	STDMETHODIMP ReplacePage(UINT, LPFNADDPROPSHEETPAGE, LPARAM) { return E_NOTIMPL; }
};

OBJECT_ENTRY_AUTO(__uuidof(PEPropPageExt), CPEPropPageExt)