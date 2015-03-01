#pragma once

#include "stdafx.h"
#include "PEReadWrite.h"
#include "ManagedFuncs.h"
#include "MiscFuncs.h"
#include "SimpleLayoutManager.h"
#include "CWindow.h"
#include "RTTI.h"
#include <vector>
#include <stack>
#include <string>
#include <WindowsX.h>
#include <commctrl.h>
#include <Richedit.h>
#include <GdiPlus.h>

// Defines and macros
#define NUM_OF_PAGES				15UL	// Current no. of page implemented


extern HMENU g_hGenericContextMenu;

// 'PropertyPageHandler' class that handles page specific things
class PropertyPageHandler abstract
{
protected:
	enum class SortOrder { None, Ascending, Descending };

	static const LPWSTR szPreferredFont;
	static const DWORD cTabs = 100;

	typedef struct CompareFuncParam
	{
		HWND hListView;
		int iColumn;
		SortOrder Order;
	} *pCompareFuncParam;

	static int CALLBACK funcStringSort(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		pCompareFuncParam pParams = pCompareFuncParam(lParamSort);
		WCHAR szBuffer1[1024], szBuffer2[1024];

		ListView_GetItemText(pParams->hListView, lParam1, pParams->iColumn, szBuffer1, ARRAYSIZE(szBuffer1));
		ListView_GetItemText(pParams->hListView, lParam2, pParams->iColumn, szBuffer2, ARRAYSIZE(szBuffer2));

		if (pParams->Order == SortOrder::Ascending)
			return _wcsicmp(szBuffer1, szBuffer2);
		else
			return _wcsicmp(szBuffer2, szBuffer1);
	};
	static int CALLBACK funcNumberSort(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		pCompareFuncParam pParams = pCompareFuncParam(lParamSort);
		WCHAR szBuffer1[1024], szBuffer2[1024];

		ListView_GetItemText(pParams->hListView, lParam1, pParams->iColumn, szBuffer1, ARRAYSIZE(szBuffer1));
		ListView_GetItemText(pParams->hListView, lParam2, pParams->iColumn, szBuffer2, ARRAYSIZE(szBuffer2));

		if (wcslen(szBuffer1) == 0)
			return -1;
		else if (wcslen(szBuffer2) == 0)
			return 1;

		int iBuffer1 = _wtoi(szBuffer1), iBuffer2 = _wtoi(szBuffer2);
		if (pParams->Order == SortOrder::Ascending)
			return (iBuffer1 == iBuffer2 ? 0 : (iBuffer1 < iBuffer2 ? -1 : 1));
		else
			return (iBuffer1 == iBuffer2 ? 0 : (iBuffer1 > iBuffer2 ? -1 : 1));
	};
	static int CALLBACK funcHexNumberSort(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		pCompareFuncParam pParams = pCompareFuncParam(lParamSort);
		WCHAR szBuffer1[1024], szBuffer2[1024];

		ListView_GetItemText(pParams->hListView, lParam1, pParams->iColumn, szBuffer1, ARRAYSIZE(szBuffer1));
		ListView_GetItemText(pParams->hListView, lParam2, pParams->iColumn, szBuffer2, ARRAYSIZE(szBuffer2));

		if (wcslen(szBuffer1) == 0)
			return -1;
		else if (wcslen(szBuffer2) == 0)
			return 1;

		int iBuffer1 = wcstol(szBuffer1, NULL, 16), iBuffer2 = wcstol(szBuffer2, NULL, 16);

		if (pParams->Order == SortOrder::Ascending)
			return (iBuffer1 == iBuffer2 ? 0 : (iBuffer1 < iBuffer2 ? -1 : 1));
		else
			return (iBuffer1 == iBuffer2 ? 0 : (iBuffer1 > iBuffer2 ? -1 : 1));
	};

	HWND m_hWnd;
	SimpleLayoutManager m_LayoutManager;
	PEReadWrite& m_PEReaderWriter;

	// 'TextAndData' struct used to hold generic list view description, data and annotation
	struct TextAndData
	{
		wstring Text;
		wstring Data;
		wstring Comments;
	};

	struct PropertyPageData
	{
		int ResourceID;
		const LPWSTR szPagename;
	} *pPropertyPageData;

	wstring Generic_OnGetTooltip(vector<RTTI::GenericTooltip>& TooltipInfo, int Index)
	{
		return TooltipInfo[Index].FullName +
				L"\nType: " + TooltipInfo[Index].Type +
				L"\nSize: " + TooltipInfo[Index].Size +
				L"\nFile offset: " + TooltipInfo[Index].FileOffset;
	}

	void Generic_OnContextMenu(vector<RTTI::GenericTooltip>& TooltipInfo,
							   vector<PropertyPageHandler::TextAndData>& ItemsInfo,
							   LONG x,
							   LONG y,
							   int Index)
	{
		if (!OpenClipboard(m_hWnd))
			return;

		HMENU hContextMenu = GetSubMenu(g_hGenericContextMenu, 0);	// Must select first subitem for context menu to work
		wstring Data;

		switch (TrackPopupMenu(hContextMenu,
								TPM_NONOTIFY | TPM_RETURNCMD,
								x,
								y,
								0,
								m_hWnd,
								NULL))
		{
			case ID_COPYVALUE:
				Data = ItemsInfo[Index].Data;

				break;

			case ID_COPYANNOTATION:
				Data = ItemsInfo[Index].Comments;

				break;

			case ID_COPYVARIABLEFULLNAME:
				Data = TooltipInfo[Index].FullName;

				break;

			case ID_COPYFILEOFFSET:
				Data = TooltipInfo[Index].FileOffset;

				break;

			default:
				CloseClipboard();

				return;
		}

		// Allocate global memory object
		HGLOBAL hGlobalData = GlobalAlloc(GMEM_MOVEABLE, sizeof(WCHAR) * (Data.size() + 1));
		if (hGlobalData)
		{
			LPVOID pGlobalData = GlobalLock(hGlobalData);

			if (!pGlobalData)
			{
				GlobalFree(hGlobalData);
				CloseClipboard();

				return;
			}

			CopyMemory(pGlobalData, Data.c_str(), sizeof(WCHAR) * (Data.size() + 1));
			GlobalUnlock(hGlobalData);

			EmptyClipboard();

			SetClipboardData(CF_UNICODETEXT, hGlobalData);
		}

		CloseClipboard();
	}

public:
	static wchar_t *const szGenericColumnText[3];
	static const PropertyPageData PropertyPagesData[];

	PropertyPageHandler(HWND hWnd, PEReadWrite& PEReaderWriter)
		:	m_hWnd(hWnd),
			m_PEReaderWriter(std::ref(PEReaderWriter)),
			m_LayoutManager(hWnd) {}
	virtual void OnInitDialog() {}
	virtual void OnShowWindow() {}
	virtual void OnHideWindow() {}
	virtual void OnPaint(HDC hdc, const RECT& rectUpdate) {}
	void OnSize(WPARAM wParam, LPARAM lParam)
	{
		// When notified about page resize ask layout manager to do so
		m_LayoutManager.DoLayout(wParam, lParam);
	}
	virtual void OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl) {}
	virtual void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam) {}
	virtual HBRUSH OnControlColorStatic(HDC hDC, HWND hControl)
	{
		// NOTE: The following is needed to make static control background transparent
		// CAUTION: Disabled edit control will also have this message called. The default
		// window procedure must handle this message for those controls. So return 'NULL' for them.
		SetBkMode(hDC, TRANSPARENT);
		return HBRUSH(GetStockObject(NULL_BRUSH));
	}
	virtual HBRUSH OnControlColorButton(HDC hDC, HWND hControl)
	{
		// NOTE: The following is need to make checkboxes control background transparent
		return NULL;
	}
	virtual void OnDestroy() {}
};


// For MSDOS Header page
class PropertyPageHandler_MSDOSHeader : public PropertyPageHandler
{
private:
	HWND m_hListViewMSDOSHeader;
	HWND m_hEditMSDOSstub;

	vector<RTTI::GenericTooltip> m_TooltipInfo;
	vector<PropertyPageHandler::TextAndData> m_ItemsInfo;

	wstring lstMSDOSHeader_OnGetTooltip(int Index);
	void lstMSDOSHeader_OnContextMenu(LONG x, LONG y, int Index);

public:
	PropertyPageHandler_MSDOSHeader(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For PE Header page
class PropertyPageHandler_PEHeaders : public PropertyPageHandler
{
private:
	HWND m_hListViewCOFFHeader;
	HWND m_hListViewOptionalHeader;

	vector<RTTI::GenericTooltip> m_COFFTooltipInfo;
	vector<RTTI::GenericTooltip> m_OptionalTooltipInfo;
	vector<PropertyPageHandler::TextAndData> m_COFFItemsInfo;
	vector<PropertyPageHandler::TextAndData> m_OptionalItemsInfo;

	wstring lstCOFFHeader_OnGetTooltip(int Index);
	wstring lstOptionalHeader_OnGetTooltip(int Index);
	void lstCOFFHeader_OnContextMenu(LONG x, LONG y, int Index);
	void lstOptionalHeader_OnContextMenu(LONG x, LONG y, int Index);

public:
	PropertyPageHandler_PEHeaders(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Sections page
class PropertyPageHandler_Sections : public PropertyPageHandler
{
private:
	HWND m_hTabsSections;
	HWND m_hListViewSections;
	vector<RTTI::GenericTooltip> m_SectionTooltipInfo;
	vector<PropertyPageHandler::TextAndData> m_SectionInfo;

	void tabsSections_OnTabChanged(HWND hControl, int SelectedIndex);
	wstring lstSections_OnGetTooltip(int Index);
	void lstSections_OnContextMenu(LONG x, LONG y, int Index);

public:
	PropertyPageHandler_Sections(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Manifest page
class PropertyPageHandler_Manifest : public PropertyPageHandler
{
private:
	HWND m_hImgInfo;
	HWND m_hStaticManifestSource;
	HWND m_hComboManifestName;
	HWND m_hComboManifestLang;
	HWND m_hEditManifest;

	void cmbManifestName_OnSelectionChanged(HWND hControl, int SelectedIndex);
	void cmbManifestLang_OnSelectionChanged(HWND hControl, int SelectedIndex);

public:
	PropertyPageHandler_Manifest(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
	void OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl);
	HBRUSH OnControlColorStatic(HDC hDC, HWND hControl);
};

// For Imports page
class PropertyPageHandler_Imports : public PropertyPageHandler
{
private:
	struct ImportTypeAndDesc
	{
		PEReadWrite::ImportType ImportType;
		void *pImportDesc;
	};
	enum class TabPages { ImportsEntry, ImportDirTable };

	HWND m_hListViewImportModules;
	HWND m_hTabsImports;
	HWND m_hListViewImportsAndDirTable;
	HWND m_hCheckBoxCPPNameUnmangle;

	vector<ImportTypeAndDesc> m_listImportTypeAndDesc;
	vector<PropertyPageHandler::TextAndData> m_ImportsDirInfo;
	vector<RTTI::GenericTooltip> m_ImportsDirTooltipInfo;
	TabPages m_TabPage;
	SortOrder m_lstModulesSortOrder;
	SortOrder m_lstImportsSortOrder;

	void chkImportsUnmangleCPPNames_OnClick(HWND hControl, bool bChecked);
	void lstImportsModules_OnSelection(HWND hControl, int SelectedIndex);
	void lstImportsModules_OnColumnHeaderClick(int Index);
	void lstImportsAndDirTable_OnColumnHeaderClick(int Index);
	void lstImportsAndDirTable_OnContextMenu(LONG x, LONG y, int Index);
	wstring lstImportsAndDirTable_OnGetTooltip(int Index);
	void tabsImports_OnTabChanged(HWND hControl, int SelectedIndex);
	void ImportsPage_UpdateDisplay();

public:
	PropertyPageHandler_Imports(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
	void OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl);
	HBRUSH OnControlColorStatic(HDC hDC, HWND hControl);
	HBRUSH OnControlColorButton(HDC hDC, HWND hControl);
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Exports page
class PropertyPageHandler_Exports : public PropertyPageHandler
{
private:
	struct RVANameOrdinalForwarder
	{
		wstring RVA;
		wstring Name;
		wstring Ordinal;
		wstring Forwarder;
	};

	HWND m_hListViewExportDir;
	HWND m_hListViewExports;
	HWND m_hCheckBoxCPPNameUnmangle;

	vector<PropertyPageHandler::TextAndData> m_ExportDirInfo;
	vector<RTTI::GenericTooltip> m_ExportDirTooltipInfo;
	SortOrder m_ExportsSortOrder;

	wstring lstExportDir_OnGetTooltip(int Index);
	void lstExportDir_OnContextMenu(LONG x, LONG y, int Index);
	void chkExportsUnmangleCPPNames_OnClick(HWND hControl, bool bChecked);
	void lstExports_OnColumnHeaderClick(int Index);
	void ExportsPage_UpdateDisplay(bool ChkBoxUnmangle_Checked);

public:
	PropertyPageHandler_Exports(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
	void OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl);
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Resources page
class PropertyPageHandler_Resources : public PropertyPageHandler
{
private:
	enum class ResourceType { Native, Managed };
	enum class NodeDataType { ResourceDir, ResourceDirEntry, ResourceData };
	struct NodeDataTypeAndPtr
	{
		ResourceType rsrcType;

		NodeDataType Type;
		union u
		{
			void *pEntry;
			size_t idxStream;

			u(void *ptr) : pEntry(ptr) {}
			u(size_t idx) : idxStream(idx) {}
		}u;
	};

	unique_ptr<ManagedFuncs::ManagedResourceReader> m_readerManagedResource;

	vector<PropertyPageHandler::TextAndData> m_ResourceInfo;
	vector<RTTI::GenericTooltip> m_ResourceTooltipInfo;

	vector<NodeDataTypeAndPtr> m_listNodeDataTypeAndPtr;

	unique_ptr<CWindow> m_pPreviewControl;

	HWND m_hTreeViewResourceEntries;
	HWND m_hListViewResourceInfo;
	HWND m_hGroupBoxResourcePreview;

	wstring lstResourceInfo_OnGetTooltip(int Index);
	void tvwResourceInfo_OnSelection(HWND hControl, NMTVITEMCHANGE *pItemChange);
	void EnumerateNativeResourceDirAndAddToTreeView(HTREEITEM hParentItem,
													unsigned int idxNode,
													const PIMAGE_RESOURCE_DIRECTORY& pResDir);
	void EnumerateManagedResourceDirAndAddToTreeView(HTREEITEM hParentItem,
													 const unique_ptr<ManagedFuncs::ManagedResourceReader>& readerManagedResource);
	void TreeViewItemShowData_IfApplicable(HWND hTreeView, HTREEITEM hItem);
	void displayHexData(LPBYTE pData, int cData, wstring& out);

public:
	PropertyPageHandler_Resources(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
	void OnShowWindow();
	void OnHideWindow();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Exception Handling page
class PropertyPageHandler_ExceptionHandling : public PropertyPageHandler
{
private:
	HWND m_hEditExceptions;

	void tabsExceptionHandlers_OnTabChanged(HWND hControl, int SelectedIndex);

public:
	PropertyPageHandler_ExceptionHandling(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
};

// For Base Relocation Data page
class PropertyPageHandler_BaseReloc : public PropertyPageHandler
{
private:
	HWND m_hTabsBaseReloc;
	HWND m_hListViewBaseRelocTable;
	HWND m_hEditFixupEntries;

	vector<PropertyPageHandler::TextAndData> m_BaseRelocInfo;
	vector<RTTI::GenericTooltip> m_BaseRelocTooltipInfo;

	wstring lstBaseRelocTable_OnGetTooltip(int Index);
	void lstBaseRelocTable_OnContextMenu(LONG x, LONG y, int Index);
	void tabsBaseRelocations_OnTabChanged(HWND hControl, int SelectedIndex);

public:
	PropertyPageHandler_BaseReloc(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Debug Data page
class PropertyPageHandler_Debug : public PropertyPageHandler
{
private:
	HWND m_hTabsDebugDirs;
	HWND m_hListViewDebugDir;
	HWND m_hEditDebugData;

	vector<PropertyPageHandler::TextAndData> m_DebugDirInfo;
	vector<RTTI::GenericTooltip> m_DebugDirTooltipInfo;

	wstring lstDebugDir_OnGetTooltip(int Index);
	void lstDebugDir_OnContextMenu(LONG x, LONG y, int Index);
	void tabsDebugDirs_OnTabChanged(HWND hControl, int SelectedIndex);

public:
	PropertyPageHandler_Debug(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Thread Local Storage page
class PropertyPageHandler_TLS : public PropertyPageHandler
{
private:
	HWND m_hListViewTLSData;
	HWND m_hListViewCallbacks;

	void tabsTLSCallbacksIndexes_OnTabChanged(HWND hControl, int SelectedIndex);

public:
	PropertyPageHandler_TLS(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
};

// For Load Configuration page
class PropertyPageHandler_LoadConfiguration : public PropertyPageHandler
{
private:
	HWND m_hListViewLoadConfig;
	HWND m_hListViewSEH;

	vector<RTTI::GenericTooltip> m_LoadConfigTooltipInfo;
	vector<PropertyPageHandler::TextAndData> m_LoadConfigItemsInfo;

	wstring lstLoadConfig_OnGetTooltip(int Index);
	void lstLoadConfig_OnContextMenu(LONG x, LONG y, int Index);

public:
	PropertyPageHandler_LoadConfiguration(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Common Language Runtime Data page
class PropertyPageHandler_CLR : public PropertyPageHandler
{
private:
	enum Tabs
	{
		tabCLRHeader = 0,
		tabMetadata,
		tabStrongNameSig,
		tabVTableFixups
	};

	HWND m_hTabsCLRData;
	HWND m_hListViewCLRData;
	HWND m_hEditCLRData;

	void tabsCLRData_OnTabChanged(HWND hControl, int SelectedIndex);

public:
	PropertyPageHandler_CLR(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Overview page
class PropertyPageHandler_Overview : public PropertyPageHandler
{
private:
	struct SelectionRectData
	{
		DWORD RVA;
		DWORD Size;
		Gdiplus::Rect R;
	};

	static const int MARGIN_X = 10;

	static const int MEMORYLAYOUT_X = 90;
	static const int MEMORYLAYOUT_Y = 30;
	static const int MEMORYLAYOUT_WIDTH = 70;
	int MEMORYLAYOUT_HEIGHT;

	static const int BLOCK_X = MEMORYLAYOUT_X + 1;
	static const int BLOCK_START_Y = MEMORYLAYOUT_Y + 1;
	static const int BLOCK_WIDTH = MEMORYLAYOUT_WIDTH - 1;

	static const int SELECTION_RECT_ADDR_SPACING = 60;

	static const int CUSTOM_X = 0;
	static const int CUSTOM_Y = 4;

	static const int CAPTION_Y_DIFF = 15;

	static const int SELECTION_RECT_ADDR_X = MARGIN_X + MEMORYLAYOUT_X + MEMORYLAYOUT_WIDTH - 4;

	bool m_firstPageShow;
	unique_handle<HBRUSH, decltype(funcDeleteBrush)> m_hBackgroundBrush;
	ULONG_PTR m_gdiplusToken;
	unique_ptr<Gdiplus::Bitmap> m_pbitmapMemoryMap;
	DWORD m_HighestRVA;
	vector<SelectionRectData> m_vectSelectionRects;
	int m_idxSelectionRect;
	HTREEITEM m_hTreeViewCustomItem;

	HWND m_hTreeViewLegends;
	HWND m_hStaticCustom;
	HWND m_hStaticCustomRVA;
	HWND m_hStaticCustomSize;
	HWND m_hEditCustomRVA;
	HWND m_hEditCustomSize;

	void tvwLegends_OnSelection(HWND hControl, NMTVITEMCHANGE *pItemChange);
	void txtCustomRVA_OnLostFocus(HWND hControl);
	void txtCustomSize_OnLostFocus(HWND hControl);

	void UpdateCustomAddressDrawingBox(bool bInvalidate = false);
	Gdiplus::Pen& RotatePenColor(Gdiplus::Pen& pen, int& Count);

public:
	PropertyPageHandler_Overview(HWND hWnd, PEReadWrite& PEReaderWriter);

	void OnInitDialog();
	void OnShowWindow();
	void OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl);
	HBRUSH OnControlColorStatic(HDC hDC, HWND hControl);
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
	void OnPaint(HDC hdc, const RECT& rectUpdate);
	void OnDestroy();
};

// For Tools page
class PropertyPageHandler_Tools : public PropertyPageHandler
{
private:
	enum class AddressType { RVA, VA, FileOffset };
	enum class DataSize { OneByte, TwoBytes, FourBytes, EightBytes };
	enum class Viewer { Hex, Disassembly };

	HWND m_hComboConvertAddrFrom;
	HWND m_hEditConvertAddrFrom;
	HWND m_hComboConvertAddrTo;
	HWND m_hEditConvertAddrTo;
	HWND m_hBtnConvertAddr;
	HWND m_hEditSHA1Hash;
	HWND m_hEditMD5Hash;
	HWND m_hEditVerifyHash;
	HWND m_hStaticVerify;
	HWND m_hComboHexViewerAddr;
	HWND m_hComboHexViewerDataSize;
	HWND m_hEditHexViewerAddr;
	HWND m_hCheckBoxDisassemble;
	unique_handle<HWND, decltype(funcDestroyWindow)> m_hTooltipDisassemble;
	HWND m_hEditHexViewer;

	HANDLE m_hbitmapCorrect;
	HANDLE m_hbitmapIncorrect;

	wstring m_SHA1;
	wstring m_MD5;

	Viewer m_View;

	void btnConvertAddr_OnClick();
	void txtVerifyHash_Changed();
	void txtHexViewerAddress_Changed();
	void cmbHexViewAddrType_OnSelectionChanged(HWND hControl, int SelectedIndex);
	void cmbHexViewDataSize_OnSelectionChanged(HWND hControl, int SelectedIndex);
	void chkHexViewDisassemble_OnClick(HWND hControl, bool bChecked);

	void UpdateHexViewer();

public:
	PropertyPageHandler_Tools(HWND hWnd, PEReadWrite& PEReaderWriter);
	~PropertyPageHandler_Tools();

	void OnInitDialog();
	void OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl);
	HBRUSH OnControlColorStatic(HDC hDC, HWND hControl);
};