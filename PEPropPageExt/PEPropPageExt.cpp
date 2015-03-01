#include "stdafx.h"
#include "PEPropPageExt.h"
#include "PropertyPageHandler.h"
#include "PictureBoxControl.h"
#include "DialogControl.h"
#include "Settings.h"
#include "unique_handle.h"
#include <algorithm>
#include <mutex>


extern CPEPropPageExtModule g_PEPropPageExtModule;

static std::mutex g_lockInitOnce;
static int32_t g_cCPEPropPageExtInstances = 0;
static HMODULE g_hRichEditDll = NULL;

HMENU g_hGenericContextMenu = NULL;

static void InitializeOnce()
{
	// Needed for RichEdit 2.0 control to work
	if (g_hRichEditDll == NULL)
		g_hRichEditDll = LoadLibrary(L"Riched20.dll"); // Load library if not yet loaded

	// Load common controls library
	INITCOMMONCONTROLSEX initcommctrl =
	{
		sizeof(INITCOMMONCONTROLSEX),
		ICC_STANDARD_CLASSES |
		ICC_ANIMATE_CLASS |
		ICC_BAR_CLASSES |
		ICC_COOL_CLASSES |
		ICC_DATE_CLASSES |
		ICC_INTERNET_CLASSES |
		ICC_LINK_CLASS |
		ICC_LISTVIEW_CLASSES |
		ICC_NATIVEFNTCTL_CLASS |
		ICC_PAGESCROLLER_CLASS |
		ICC_PROGRESS_CLASS |
		ICC_TAB_CLASSES |
		ICC_TREEVIEW_CLASSES |
		ICC_UPDOWN_CLASS |
		ICC_USEREX_CLASSES |
		ICC_WIN95_CLASSES
	};
	InitCommonControlsEx(&initcommctrl);

	// Register our custom picture box control
	PictureBoxControl::registerControlWindowClass(g_PEPropPageExtModule.getInstance());

	// Register parent window class for dialog preview
	DialogControl::registerParentWindowClass(g_PEPropPageExtModule.getInstance());
	
	// Create generic popup menu
	g_hGenericContextMenu = LoadMenu(g_PEPropPageExtModule.getInstance(),
									 MAKEINTRESOURCE(IDR_GENERIC_CONTEXT_MENU));
}

void UninitializeOnce()
{
	// Unregister our custom picture box control
	PictureBoxControl::unregisterControlWindowClass();

	// Unregister parent window class for dialog preview
	DialogControl::unregisterParentWindowClass();
}

CPEPropPageExt::CPEPropPageExt()
{
	// Call only-once function
	lock_guard<mutex> lock(g_lockInitOnce);
	if (g_cCPEPropPageExtInstances++ == 0)
		InitializeOnce();
}

CPEPropPageExt::~CPEPropPageExt()
{
	lock_guard<mutex> lock(g_lockInitOnce);
	if (--g_cCPEPropPageExtInstances == 0)
		UninitializeOnce();
}

/////////////////////// This dll program starts running from here ///////////////////////
STDMETHODIMP CPEPropPageExt::Initialize(LPCITEMIDLIST pidlFolder,
										LPDATAOBJECT pDataObj,
										HKEY hProgID)
{
	// It is possible to disable this property page extension using
	//	a registry value, check if it is set
	if (ReadSettings(SettingType::Hide_AllTabs))
		return E_FAIL;

	WCHAR szFilename[MAX_PATH];	// Holds the filename whose properties is requested
	FORMATETC FormatEtc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1L, TYMED_HGLOBAL };
	STGMEDIUM StgMedium = { 0 };		// Represents GlobalLock object

	// Retrieve data object containing the filename(s)
	if (FAILED(pDataObj->GetData(&FormatEtc, &StgMedium)))
		return E_UNEXPECTED;

	// Perform a global lock on the data object to read
	auto funcDropHandleReleaser = [](HDROP handle) { GlobalUnlock(handle); };
	unique_handle<HDROP, decltype(funcDropHandleReleaser)> hDropData(HDROP(GlobalLock(StgMedium.hGlobal)),
																	 funcDropHandleReleaser);
	if (!hDropData)
		return E_OUTOFMEMORY;

	// Make sure there are only one filename in the data object
	const UINT NUM_OF_FILENAMES_NEEDED = 1UL;
	if (DragQueryFile(hDropData.get(), -1, NULL, 0) != NUM_OF_FILENAMES_NEEDED)
		return E_NOTIMPL;

	// Retrieve the filename and store in a local variable
	if (!DragQueryFile(hDropData.get(), 0, szFilename, MAX_PATH))
		return E_OUTOFMEMORY;

	// Open the file with 'PEReadWrite'
	if (!m_PEReaderWriter.open(szFilename))
	{
		LogError(L"ERROR: Failed to open file for reading!\n"
				 L"Another process might have the file open without sharing read permission.", true);
	
		return E_FAIL;
	}

	return S_OK;
}

// We add property pages to the shell properties dialog here
STDMETHODIMP CPEPropPageExt::AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
	unsigned int numAddedPages = 0UL;					// No. of pages successfully added
	PROPSHEETPAGE PropSheetPage;						// Represents current property page to be added
	HPROPSHEETPAGE hPropSheetPage[NUM_OF_PAGES];		// Handles to all add pages
	ZeroMemory(hPropSheetPage, sizeof(HPROPSHEETPAGE) * NUM_OF_PAGES);

	// Add new property pages to shell properties dialog
	for (unsigned int i = 0; i < NUM_OF_PAGES; ++i)
	{
		// If secondary header is not PE then no need for other tabs
		if (m_PEReaderWriter.getSecondaryHeaderType() != PEReadWrite::HeaderType::PE &&
			PropertyPageHandler::PropertyPagesData[i].ResourceID == IDD_PROPPAGE_SECTIONS)
			break;

		// If certain data directory is not applicable/disabled, skip its tab creation
		switch (PropertyPageHandler::PropertyPagesData[i].ResourceID)
		{
			case IDD_PROPPAGE_MSDOSHEADER:
				if (ReadSettings(SettingType::Hide_MSDOSHeaderTab))
					continue;

				break;

			case IDD_PROPPAGE_PEHEADERS:
				if (ReadSettings(SettingType::Hide_PEHeadersTab))
					continue;

				break;

			case IDD_PROPPAGE_SECTIONS:
				if (ReadSettings(SettingType::Hide_SectionsTab))
					continue;

				break;

			case IDD_PROPPAGE_MANIFEST:
				if (ReadSettings(SettingType::Hide_ManifestTab))
					continue;

				break;

			case IDD_PROPPAGE_IMPORTS:
				if (!m_PEReaderWriter.hasImports() ||
					ReadSettings(SettingType::Hide_ImportsTab))
					continue;

				break;

			case IDD_PROPPAGE_EXPORTS:
				if (!m_PEReaderWriter.hasExports() ||
					ReadSettings(SettingType::Hide_ExportsTab))
					continue;

				break;

			case IDD_PROPPAGE_RESOURCES:
				if (!m_PEReaderWriter.hasResources() ||
					ReadSettings(SettingType::Hide_ResourcesTab))
					continue;

				break;

			case IDD_PROPPAGE_EXCEPTION:
				if (!m_PEReaderWriter.hasExceptionHandlingData() ||
					ReadSettings(SettingType::Hide_ExceptionTab))
					continue;

				break;

			case IDD_PROPPAGE_BASERELOC:
				if (!m_PEReaderWriter.hasBaseRelocationData() ||
					ReadSettings(SettingType::Hide_BaseRelocTab))
					continue;

				break;

			case IDD_PROPPAGE_DEBUG:
				if (!m_PEReaderWriter.hasDebugData() ||
					ReadSettings(SettingType::Hide_DebugTab))
					continue;

				break;

			case IDD_PROPPAGE_TLS:
				if (!m_PEReaderWriter.hasTLSData() ||
					ReadSettings(SettingType::Hide_TLSTab))
					continue;

				break;

			case IDD_PROPPAGE_LOADCONFIG:
				if (!m_PEReaderWriter.hasLoadConfigData() ||
					ReadSettings(SettingType::Hide_LoadConfigTab))
					continue;

				break;

			case IDD_PROPPAGE_CLR:
				if (!m_PEReaderWriter.hasCLRData() ||
					ReadSettings(SettingType::Hide_CLRTab))
					continue;

				break;

			case IDD_PROPPAGE_OVERVIEW:
				if (ReadSettings(SettingType::Hide_OverviewTab))
					continue;

				break;

			case IDD_PROPPAGE_TOOLS:
				if (ReadSettings(SettingType::Hide_ToolsTab))
					continue;

				break;

			default:
#ifdef DEBUG
				throw exception("Unimplemented property page in 'AddPages()'.");
#else
				LogError(L"ERROR: Internal error. Unimplemented property page caught in '" TEXT(__FILE__) L"'.");
				return E_FAIL;
#endif
		}

		// Because we reuse 'PropSheetPage' in this loop
		ZeroMemory(&PropSheetPage, sizeof(PropSheetPage));

		// Fill in details in 'PropSheetPage'
		PropSheetPage.dwSize = sizeof(PropSheetPage);
		PropSheetPage.dwFlags = PSP_USETITLE | PSP_USECALLBACK | PSP_USEICONID;
		PropSheetPage.hInstance = g_PEPropPageExtModule.getInstance();
		PropSheetPage.pszTemplate = MAKEINTRESOURCE(PropertyPageHandler::PropertyPagesData[i].ResourceID);
		PropSheetPage.pszIcon = MAKEINTRESOURCE(IDI_ICONPROPPAGE);
		PropSheetPage.pszTitle = PropertyPageHandler::PropertyPagesData[i].szPagename;
		PropSheetPage.pfnDlgProc = PropertyPagesProc;
		PropSheetPage.lParam = LPARAM(this);
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
		++numAddedPages;
	}

	// Check if not all of the property pages were added
	if (numAddedPages == 0)
		return E_FAIL;		// No property page was added so fail

	// Continue with all of the successful property page additions
	return S_OK;
}

UINT CALLBACK CPEPropPageExt::PropertyPageLifeEventCallback(HWND hWnd,
															UINT uMsg,
															LPPROPSHEETPAGE ppsp)
{
	switch (uMsg)
	{
		case PSPCB_CREATE:
			return TRUE;	// Must return TRUE to enable the page to be created

		case PSPCB_RELEASE:
		{
			// When the callback function receives the PSPCB_RELEASE notification, 
			// the 'ppsp' parameter of the 'PropSheetPageProc' contains a pointer to 
			// the 'PROPSHEETPAGE' structure. The 'lParam' member of the 'PROPSHEETPAGE' 
			// structure contains the extension pointer which can be used to 
			// release the object.

			// Release the property sheet extension object. This is called even 
			// if the property page was never actually displayed.
			CPEPropPageExt *pPEPropPageExt = reinterpret_cast<CPEPropPageExt *>(ppsp->lParam);

			if (pPEPropPageExt)
				pPEPropPageExt->Release();
		}
	}

	return FALSE;
}

// Windows message handler for our property pages
INT_PTR CALLBACK CPEPropPageExt::PropertyPagesProc(HWND hWnd,
												   UINT uMsg,
												   WPARAM wParam,
												   LPARAM lParam)
{
	PropertyPageHandler *pPropertyPageHandler;

	// Determine the window message and handle it
	if (uMsg == WM_INITDIALOG)	// Called once for every property page that is created
		// NOTE: 'hWnd' value is different for each property page creation
	{
		// Get current cursor
		HCURSOR hCurrentCursor = GetCursor();

		// Load and show busy cursor
		auto BusyCursorReleaser = [&](HCURSOR handle)
		{
			// Restore last cursor
			if (hCurrentCursor != NULL)
				SetCursor(hCurrentCursor);

			// Destroy loaded system cursor
			if (handle != NULL)
				DestroyCursor(handle);
		};
		unique_handle<HCURSOR, decltype(BusyCursorReleaser)> hBusyCursor(LoadCursor(NULL, IDC_WAIT),
																		 BusyCursorReleaser);

		if (hBusyCursor.get())
			SetCursor(hBusyCursor.get());

		// Get reference to 'PEReadWrite' object
		PEReadWrite& PEReaderWriter = reinterpret_cast<CPEPropPageExt *>(LPPROPSHEETPAGE(lParam)->lParam)->m_PEReaderWriter;

		// Determine the property page's dialog resource ID
		int DialogResourceID = int(LPPROPSHEETPAGE(lParam)->pszTemplate);

		// Create a new handler for this property page
		switch (DialogResourceID)
		{
			case IDD_PROPPAGE_MSDOSHEADER:
				pPropertyPageHandler = new PropertyPageHandler_MSDOSHeader(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_PEHEADERS:
				pPropertyPageHandler = new PropertyPageHandler_PEHeaders(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_SECTIONS:
				pPropertyPageHandler = new PropertyPageHandler_Sections(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_MANIFEST:
				pPropertyPageHandler = new PropertyPageHandler_Manifest(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_IMPORTS:
				pPropertyPageHandler = new PropertyPageHandler_Imports(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_EXPORTS:
				pPropertyPageHandler = new PropertyPageHandler_Exports(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_RESOURCES:
				pPropertyPageHandler = new PropertyPageHandler_Resources(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_EXCEPTION:
				pPropertyPageHandler = new PropertyPageHandler_ExceptionHandling(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_BASERELOC:
				pPropertyPageHandler = new PropertyPageHandler_BaseReloc(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_DEBUG:
				pPropertyPageHandler = new PropertyPageHandler_Debug(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_TLS:
				pPropertyPageHandler = new PropertyPageHandler_TLS(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_LOADCONFIG:
				pPropertyPageHandler = new PropertyPageHandler_LoadConfiguration(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_CLR:
				pPropertyPageHandler = new PropertyPageHandler_CLR(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_OVERVIEW:
				pPropertyPageHandler = new PropertyPageHandler_Overview(hWnd, std::ref(PEReaderWriter));

				break;

			case IDD_PROPPAGE_TOOLS:
				pPropertyPageHandler = new PropertyPageHandler_Tools(hWnd, std::ref(PEReaderWriter));

				break;

			default:
#ifdef DEBUG
				// If this error occurs, it must be caught and fixed
				//	before shipping the software
				throw std::exception("Unimplemented property sheet caught in file " __FILE__ " at line " STRINGIFY(__LINE__) ".");
#else
				LogError(L"Unimplemented property sheet caught in file " TEXT(__FILE__) L" at line " TEXT(STRINGIFY(__LINE__)) L".", true);
				return FALSE;
#endif
		}

		// If object allocation in heap failed, fail this page initialization
		if (!pPropertyPageHandler)
		{
			LogError(L"ERROR: Low memory! Failed to initialize property page.", true);
			return FALSE;
		}

		// Put the pointer of property page in the page window's user data section
		// CAUTION: When 'SetWindowLongPtr()' is used with 'GWLP_USERDATA', the return value
		// is the last value stored and not an indication of error. To check if the call was
		// successful, check the last error for this thread as shown below.
		SetLastError(0);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(pPropertyPageHandler));
		if (GetLastError() != 0)
		{
			LogError(L"ERROR: Sytem error. Couldn't put property handler pointer in window's user data storage. Restart Explorer.", true);
			return FALSE;
		}

		// Call 'OnInitDialog()' handler for this page
		try
		{
			pPropertyPageHandler->OnInitDialog();
		}
		catch (WCHAR *e)
		{
			LogError(e, true);
		}
		catch (...)
		{
			LogError(L"ERROR: Unexpected internal error occurred in file " TEXT(__FILE__) L" at line " + DWORD_toString(__LINE__) + L".", true);
		}

		// Return 'TRUE' to allow system to set keyboard focus to this window
		return TRUE;
	}
	else
	{
		// Get the property page handler object for the current page
		pPropertyPageHandler = reinterpret_cast<PropertyPageHandler *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if (pPropertyPageHandler)
		{
			try
			{
				switch (uMsg)
				{
					case WM_SHOWWINDOW:
						if (wParam == TRUE)
							pPropertyPageHandler->OnShowWindow();
						else
							pPropertyPageHandler->OnHideWindow();

						break;

					case WM_PAINT:
					{
						PAINTSTRUCT ps;

						if (BeginPaint(hWnd, &ps))
						{
							pPropertyPageHandler->OnPaint(ps.hdc, ps.rcPaint);
							EndPaint(hWnd, &ps);
						}

						break;
					}

					case WM_SIZE:	// Called just once for sizing and positioning controls because
						//	property pages usually don't have resize ability
						pPropertyPageHandler->OnSize(wParam, lParam);

						break;

					case WM_DESTROY:	// Called for each page when the main shell property window closes
						pPropertyPageHandler->OnDestroy();
						SAFE_RELEASE(pPropertyPageHandler);
						SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(NULL));

						break;

					case WM_COMMAND:	// Called for command button clicks
						pPropertyPageHandler->OnCommand(HIWORD(wParam), LOWORD(wParam), HWND(lParam));

						break;

					case WM_NOTIFY:
					{
						// Notification details object pointer
						LPNMHDR pHdr = LPNMHDR(lParam);

						if (!pHdr)
							return 0;

						switch (pHdr->code)
						{
							case PSN_APPLY:				// 'Apply' button pressed
							case PSN_SETACTIVE:			// A property page about to be activated
								return PSNRET_NOERROR;	// Allow it
						}

						// Redirect notification information to property page handlers
						pPropertyPageHandler->OnNotify(pHdr->code, pHdr->idFrom, pHdr->hwndFrom, lParam);
					}
					break;

					case WM_CTLCOLORSTATIC:
					{
						// This message is used to make the background of static label control transparent
						HBRUSH hBrush = pPropertyPageHandler->OnControlColorStatic(HDC(wParam), HWND(lParam));

						if (hBrush)
							return INT_PTR(hBrush); // Return the brush
						
						// Else, let default window procedure handle it
					}
					break;

					case WM_CTLCOLORBTN:
					{
						// This message is used to make the background of checkboxes transparent
						HBRUSH hBrush = pPropertyPageHandler->OnControlColorButton(HDC(wParam), HWND(lParam));

						if (hBrush)
							return INT_PTR(hBrush); // Return the brush
						
						// Else, let default window procedure handle it
					}
					break;
				}
			}
			catch (LPWSTR e)
			{
				LogError(e, true);
			}
			catch (...)
			{
				LogError(L"ERROR: Unexpected internal error occurred in file " TEXT(__FILE__) L" at line "  + DWORD_toString(__LINE__) + L".", true);
			}
		}
	}

	// Nearly all window messages require that we return zero.
	// For message that require otherwise, it is done so in cases
	// for 'switch' statements above.
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}