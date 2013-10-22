// PEPropPageExt.cpp : Implementation of DLL Exports.
#include "stdafx.h"
#include "Resource.h"
#include "PEPropPageExt_i.h"
#include "PEPropPageExt.h"
#include "dllmain.h"
#include "CommonDefs.h"
#include "ScopedPointerVector.h"
#include "PropertyPageHandler.h"
#include "Settings.h"
#include <string>
#include <list>
#include <algorithm>
#include <Windows.h>


using namespace std;


THREAD_ISOLATED_STORAGE HMENU hGenericContextMenu;

HRESULT CPEPropPageExt::FinalConstruct()
{
	// Load common controls library
	INITCOMMONCONTROLSEX initcommctrl = { sizeof(INITCOMMONCONTROLSEX),
											ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_TAB_CLASSES };
	InitCommonControlsEx(&initcommctrl);

	// Needed for RichEdit 2.0 control to work
	hRichEditDll = LoadLibrary(_T("Riched20.dll"));

	// Create generic popup menu
	hGenericContextMenu = LoadMenu(_AtlModule.hInstance, MAKEINTRESOURCE(IDR_GENERIC_CONTEXT_MENU));

	return S_OK;
}

void CPEPropPageExt::FinalRelease()
{
	DestroyMenu(hGenericContextMenu);
	FreeLibrary(hRichEditDll);
}


/////////////////////// This dll program starts running from here ///////////////////////
STDMETHODIMP CPEPropPageExt::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hProgID)
{
	// It is possible to disable this property page extension using
	//	a registry value, check if it is set
	if (ReadSettings(Hide_AllTabs))
		return E_FAIL;	

	TCHAR szFilename[MAX_PATH];	// Holds the filename whose properties is requested
	HRESULT Retval = E_FAIL;	// Return value for this function
	FORMATETC FormatEtc = {CF_HDROP, NULL, DVASPECT_CONTENT, -1L, TYMED_HGLOBAL};
	STGMEDIUM StgMedium;		// Represents GlobalLock object

	// Retrieve data object containing the filename(s)
	if (FAILED(pDataObj->GetData(&FormatEtc, &StgMedium)))
		return E_FAIL;

	// Perform a global lock on the data object to read
	HDROP hDropData = (HDROP) GlobalLock(StgMedium.hGlobal);
	if (!hDropData)
		GOTO_RELEASE_HANDLER(0, E_FAIL);

	// Make sure there are only one filename in the data object
	if (DragQueryFile(hDropData, (LONG_PTR) -1, NULL, 0) != 1)
		GOTO_RELEASE_HANDLER(1, E_NOTIMPL);

	// Retrieve the filename and store in a local variable
	if (!DragQueryFile(hDropData, 0, szFilename, MAX_PATH))
		GOTO_RELEASE_HANDLER(1, E_FAIL);

	// Open the file with 'PEReadWrite'
	if (!PEReaderWriter.Open(szFilename))
		GOTO_RELEASE_HANDLER(1, E_FAIL);
	
	// All operations succeeded
	GOTO_RELEASE_HANDLER(1, S_OK);

	// Error handlers
	DEFINE_RELEASE_HANDLER(1, GlobalUnlock(hDropData););
	DEFINE_RELEASE_HANDLER(0, ReleaseStgMedium(&StgMedium););
	
	return Retval;
}

// We add property pages to the shell properties dialog here
STDMETHODIMP CPEPropPageExt::AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
	unsigned int PageCnt = 0;							// No. of pages successfully added
	PROPSHEETPAGE PropSheetPage;						// Represents current property page to be added
	HPROPSHEETPAGE hPropSheetPage[NUM_OF_PAGES];		// Handles to all add pages
	ZeroMemory(hPropSheetPage, sizeof(hPropSheetPage));
	
	// Add new property pages to shell properties dialog
	for	(int i = 0; i < NUM_OF_PAGES; i++)
	{
		// If secondary header is not PE then no need for other tabs
		if (PEReaderWriter.GetSecondaryHeaderType() != PEReadWrite::PEHeader &&
			PropertyPageHandler::PropertyPagesData[i].ResourceID == IDD_PROPPAGE_SECTIONS)
			break;

		// If certain data directory is not applicable, skip its tab creation
		switch (PropertyPageHandler::PropertyPagesData[i].ResourceID)
		{
		case IDD_PROPPAGE_MSDOSHEADER:
			if (ReadSettings(Hide_MSDOSHeaderTab))
				continue;

			break;

		case IDD_PROPPAGE_PEHEADERS:
			if (ReadSettings(Hide_PEHeadersTab))
				continue;

			break;

		case IDD_PROPPAGE_SECTIONS:
			if (ReadSettings(Hide_SectionsTab))
				continue;

			break;

		case IDD_PROPPAGE_MANIFEST:
			if (ReadSettings(Hide_ManifestTab))
				continue;

			break;

		case IDD_PROPPAGE_IMPORTS:
			if (!PEReaderWriter.HasImports() ||
					ReadSettings(Hide_ImportsTab))
				continue;

			break;

		case IDD_PROPPAGE_EXPORTS:
			if (!PEReaderWriter.HasExports() ||
					ReadSettings(Hide_ExportsTab))
				continue;

			break;

		case IDD_PROPPAGE_RESOURCES:
			if (!PEReaderWriter.HasResources() ||
					ReadSettings(Hide_ResourcesTab))
				continue;

			break;

		case IDD_PROPPAGE_EXCEPTION:
			if (!PEReaderWriter.HasExceptionHandlingData() ||
					ReadSettings(Hide_ExceptionTab))
				continue;

			break;

		case IDD_PROPPAGE_BASERELOC:
			if (!PEReaderWriter.HasBaseRelocationData() ||
					ReadSettings(Hide_BaseRelocTab))
				continue;

			break;

		case IDD_PROPPAGE_DEBUG:
			if (!PEReaderWriter.HasDebugData() ||
					ReadSettings(Hide_DebugTab))
				continue;

			break;

		case IDD_PROPPAGE_TLS:
			if (!PEReaderWriter.HasTLSData() ||
					ReadSettings(Hide_TLSTab))
				continue;

			break;

		case IDD_PROPPAGE_LOADCONFIG:
			if (!PEReaderWriter.HasLoadConfigData() ||
					ReadSettings(Hide_LoadConfigTab))
				continue;

			break;

		case IDD_PROPPAGE_CLR:
			if (!PEReaderWriter.HasCLRData() ||
					ReadSettings(Hide_CLRTab))
				continue;

			break;

		case IDD_PROPPAGE_OVERVIEW:
			if (ReadSettings(Hide_OverviewTab))
				continue;

			break;

		case IDD_PROPPAGE_TOOLS:
			if (ReadSettings(Hide_ToolsTab))
				continue;
		}

		// Because we reuse 'PropSheetPage' in this loop
		ZeroMemory(&PropSheetPage, sizeof(PROPSHEETPAGE));

		// Fill in details in 'PropSheetPage'
		PropSheetPage.dwSize = sizeof(PROPSHEETPAGE);
		PropSheetPage.dwFlags = PSP_USETITLE | PSP_USECALLBACK | PSP_USEICONID;
		PropSheetPage.hInstance = _AtlModule.hInstance; 
		PropSheetPage.pszTemplate = MAKEINTRESOURCE(PropertyPageHandler::PropertyPagesData[i].ResourceID);
		PropSheetPage.pszIcon = MAKEINTRESOURCE(IDI_ICONPROPPAGE);
		PropSheetPage.pszTitle = (LPTSTR) PropertyPageHandler::PropertyPagesData[i].Pagename.c_str();
		PropSheetPage.pfnDlgProc = PropertyPagesProc;
		PropSheetPage.lParam = (LPARAM) this;
		PropSheetPage.pfnCallback = PropertyPageLifeEventCallback;
		PropSheetPage.pcRefParent = NULL;

		// Create this property page
		hPropSheetPage[i] = CreatePropertySheetPage(&PropSheetPage);
		if (!hPropSheetPage[i])	// Was it created?
			continue;

		// Add this property page
		if (!pfnAddPage(hPropSheetPage[i], lParam))
		{
			DestroyPropertySheetPage(hPropSheetPage[i]);
			continue;
		}

		// Increment reference so that this object is not destroyed
		// NOTE: This statement has to be here not in 'PropertyPageLifeEventCallback'
		this->AddRef();

		// Keep count of successful property page additions
		PageCnt++;
	}

	// Check if not all of the property pages were added
	if (PageCnt == 0)
		return E_FAIL;		// No property page was added so fail

	// Continue with all of the successful property page additions
	return S_OK;
}

UINT CALLBACK CPEPropPageExt::PropertyPageLifeEventCallback(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
	CPEPropPageExt *pPEPropPageExt = reinterpret_cast<CPEPropPageExt *>(ppsp->lParam);

	if (pPEPropPageExt)
	{
		switch(uMsg)
		{
		case PSPCB_CREATE:
				return TRUE;	// Must return TRUE to enable the page to be created

		case PSPCB_RELEASE:
			{
				// When the callback function receives the PSPCB_RELEASE notification, 
				// the ppsp parameter of the PropSheetPageProc contains a pointer to 
				// the PROPSHEETPAGE structure. The lParam member of the PROPSHEETPAGE 
				// structure contains the extension pointer which can be used to 
				// release the object.

				// Release the property sheet extension object. This is called even 
				// if the property page was never actually displayed.
				pPEPropPageExt->Release();
			}

			break;
		}
	}

    return FALSE;
}

// Windows message handler for our property pages
INT_PTR CALLBACK CPEPropPageExt::PropertyPagesProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PropertyPageHandler *pPropertyPageHandler = NULL;

	// Determine the window message and handle it
	if (uMsg == WM_INITDIALOG)	// Called once for every property page that is created
	// NOTE: 'hWnd' value is different for each property page creation
	{
		// Get current cursor
		HCURSOR hCurrentCursor = GetCursor();

		// Load and show busy cursor
		HICON hBusyCursor = LoadCursor(NULL, IDC_WAIT);
		SetCursor(hBusyCursor);

		// Get reference to 'PEReadWrite' object
		PEReadWrite& PEReaderWriter = reinterpret_cast<CPEPropPageExt *>(reinterpret_cast<LPPROPSHEETPAGE>(lParam)->lParam)->PEReaderWriter;

		// Determine the property page's dialog resource ID
		int DialogResourceID = (int) reinterpret_cast<LPPROPSHEETPAGE>(lParam)->pszTemplate;

		// Create a new handler for this property page
		switch (DialogResourceID)
		{
		case IDD_PROPPAGE_MSDOSHEADER:
			pPropertyPageHandler = new PropertyPageHandler_MSDOSHeader(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_PEHEADERS:
			pPropertyPageHandler = new PropertyPageHandler_PEHeaders(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_SECTIONS:
			pPropertyPageHandler = new PropertyPageHandler_Sections(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_MANIFEST:
			pPropertyPageHandler = new PropertyPageHandler_Manifest(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_IMPORTS:
			pPropertyPageHandler = new PropertyPageHandler_Imports(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_EXPORTS:
			pPropertyPageHandler = new PropertyPageHandler_Exports(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_RESOURCES:
			pPropertyPageHandler = new PropertyPageHandler_Resources(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_EXCEPTION:
			pPropertyPageHandler = new PropertyPageHandler_ExceptionHandling(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_BASERELOC:
			pPropertyPageHandler = new PropertyPageHandler_BaseReloc(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_DEBUG:
			pPropertyPageHandler = new PropertyPageHandler_Debug(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_TLS:
			pPropertyPageHandler = new PropertyPageHandler_TLS(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_LOADCONFIG:
			pPropertyPageHandler = new PropertyPageHandler_LoadConfiguration(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_CLR:
			pPropertyPageHandler = new PropertyPageHandler_CLR(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_OVERVIEW:
			pPropertyPageHandler = new PropertyPageHandler_Overview(hWnd, PEReaderWriter);

			break;

		case IDD_PROPPAGE_TOOLS:
			pPropertyPageHandler = new PropertyPageHandler_Tools(hWnd, PEReaderWriter);

			break;

	#ifdef DEBUG
		default:
			// If this error occurs, it must be caught and fixed
			//	before shipping the software
			throw std::exception("Unimplemented property sheet caught in 'PropertyPageHandler.cpp'");
	#endif
		}

		// Put the pointer of property page in the page window's user data section
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) pPropertyPageHandler);

		// Call property page handler for this page
		pPropertyPageHandler->OnInitDialog();

		// Restore last cursor
		SetCursor(hCurrentCursor);

		// Destroy loaded system cursor
		DestroyCursor(hBusyCursor);
	}
	else
	{
		// Get the property page handler object for the current page
		pPropertyPageHandler = (PropertyPageHandler *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

		if (pPropertyPageHandler)
		{
			switch (uMsg)
			{
			case WM_SHOWWINDOW:
				if (wParam == TRUE)
					pPropertyPageHandler->OnShowWindow();

				break;

			case WM_PAINT:
				{
					PAINTSTRUCT ps;

					HDC hdc = BeginPaint(hWnd, &ps);
					pPropertyPageHandler->OnPaint(ps.hdc, ps.rcPaint);
					EndPaint(hWnd, &ps);
				}

				break;

			case WM_SIZE:	// Called just once for sizing and positioning controls because
							//	property pages usually don't have resize ability
				pPropertyPageHandler->OnSize(wParam, lParam);
			
				break;

			case WM_DESTROY:	// Called for each page when the main shell property window closes
				pPropertyPageHandler->OnDestroy();
				SAFE_RELEASE(pPropertyPageHandler);
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) NULL);

				break;

			case WM_COMMAND:	// Called for command button clicks
				pPropertyPageHandler->OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND) lParam);

				break;

			case WM_NOTIFY:
				{
					// Notification details object pointer
					NMHDR *pHdr = (NMHDR *) lParam;

					// Redirect notification information to property page handlers
					pPropertyPageHandler->OnNotify(pHdr->code, pHdr->idFrom, pHdr->hwndFrom, lParam);
				}
			}
		}
	}

	if (uMsg == WM_NOTIFY)
	{
		// Notification details object pointer
		NMHDR *pHdr = (NMHDR *) lParam;

		// Determine if the notification is for user clicking 'Apply' on main dialog
		switch (pHdr->code)
		{
		case PSN_APPLY:				// 'Apply' button pressed
		case PSN_SETACTIVE:			// A property page about to be activated
			return PSNRET_NOERROR;	// Allow it
		}
	}

	// Nearly all window messages require that we return zero.
	// For message that require otherwise, it is done so in cases
	// for 'switch' statements above.
	return 0;
}