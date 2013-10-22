#pragma once

#include "stdafx.h"
#include "PEReadWrite.h"
#include "udis86.h"
#include "MiscFuncs.h"
#include "resource.h"
#include "SimpleLayoutManager.h"
#include "RTTI.h"
#include <vector>
#include <string>
#include <Windows.h>
#include <WindowsX.h>
#include <commctrl.h>
#include <Richedit.h>
#include <GdiPlus.h>


using namespace std;
using namespace Gdiplus;

// Defines and macros
#define NUM_OF_PAGES				15	// Current no. of page implemented
// The following macro is used to conveniently put data in a 'FillData' struct object
#define FillData(vectorobj, ...)	(vectorobj).push_back(PropertyPageHandler::TextAndData(__VA_ARGS__))

extern THREAD_ISOLATED_STORAGE HMENU hGenericContextMenu;

enum SortOrder { None, Ascending, Descending };


// 'PropertyPageHandler' class that handles page specific things
class PropertyPageHandler abstract
{
protected:
	HWND m_hWnd;
	unique_ptr<SimpleLayoutManager> m_pLayoutManager;
	PEReadWrite& m_PEReaderWriter;

	// 'TextAndData' struct used to hold generic list view description, data and annotation
	struct TextAndData
	{
		tstring szText;
		tstring szData;
		tstring szComments;

		TextAndData(tstring sztext,
					tstring szdata,
					tstring szcomments = _T(""))
			: szText(sztext), szData(szdata), szComments(szcomments) { }
	};

	struct PropertyPageData
	{
		int ResourceID;
		tstring Pagename;
	} *pPropertyPageData;

	tstring Generic_OnGetTooltip(vector<RTTI::GenericTooltip>& TooltipInfo, int Index);
	void Generic_OnContextMenu(vector<RTTI::GenericTooltip>& TooltipInfo,
								vector<PropertyPageHandler::TextAndData>& ItemsInfo,
								LONG x, LONG y, int Index);

public:
	static wchar_t * const GenericColumnText[3];
	static const PropertyPageData PropertyPagesData[];

	PropertyPageHandler(HWND hWnd, PEReadWrite& PEReaderWriter)
		: m_hWnd(hWnd),
			m_PEReaderWriter(PEReaderWriter),
			m_pLayoutManager(new SimpleLayoutManager(hWnd)) {}
	virtual void OnInitDialog() {}
	virtual void OnShowWindow() {}
	virtual void OnPaint(HDC hdc, const RECT& rectUpdate) {}
	void OnSize(WPARAM wParam, LPARAM lParam);
	virtual void OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl) {}
	virtual void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam) {}
	virtual HBRUSH OnControlColorEdit(HWND hControl, HDC hdc) { return NULL; }
	virtual void OnDestroy() {}
};

// For MSDOS Header page
class PropertyPageHandler_MSDOSHeader : public PropertyPageHandler
{
private:
	HWND hListViewMSDOSHeader;
	HWND hEditMSDOSstub;

	vector<RTTI::GenericTooltip> TooltipInfo;
	vector<PropertyPageHandler::TextAndData> ItemsInfo;

	tstring lstMSDOSHeader_OnGetTooltip(int Index);
	void lstMSDOSHeader_OnContextMenu(LONG x, LONG y, int Index);

public:
	PropertyPageHandler_MSDOSHeader(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter) {}

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For PE Header page
class PropertyPageHandler_PEHeaders : public PropertyPageHandler
{
private:
	vector<RTTI::GenericTooltip> COFFTooltipInfo;
	vector<RTTI::GenericTooltip> OptionalTooltipInfo;
	vector<PropertyPageHandler::TextAndData> COFFItemsInfo;
	vector<PropertyPageHandler::TextAndData> OptionalItemsInfo;

	tstring lstCOFFHeader_OnGetTooltip(int Index);
	tstring lstOptionalHeader_OnGetTooltip(int Index);
	void lstCOFFHeader_OnContextMenu(LONG x, LONG y, int Index);
	void lstOptionalHeader_OnContextMenu(LONG x, LONG y, int Index);

public:
	PropertyPageHandler_PEHeaders(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter) {}

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Sections page
class PropertyPageHandler_Sections : public PropertyPageHandler
{
private:
	HWND hTabsSections;
	HWND hListViewSections;
	vector<RTTI::GenericTooltip> SectionTooltipInfo;
	vector<PropertyPageHandler::TextAndData> SectionInfo;

	void tabsSections_OnTabChanged(HWND hControl, int SelectedIndex);
	tstring lstSections_OnGetTooltip(int Index);
	void lstSections_OnContextMenu(LONG x, LONG y, int Index);

public:
	PropertyPageHandler_Sections(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter) {}

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Manifest page
class PropertyPageHandler_Manifest : public PropertyPageHandler
{
private:
	void cmbManifestName_OnSelectionChanged(HWND hControl, int SelectedIndex);
	void cmbManifestLang_OnSelectionChanged(HWND hControl, int SelectedIndex);

public:
	PropertyPageHandler_Manifest(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter) {}

	void OnInitDialog();
	void OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl);
};

// For Imports page
class PropertyPageHandler_Imports : public PropertyPageHandler
{
private:
	enum TabPages { TabImports, TabDirTable };

	HWND hListViewImportsModules;
	HWND hTabsImports;
	HWND hListViewImportsAndDirTable;
	HWND hCheckBoxCPPNameUnmangle;

	vector<PEReadWrite::ImportNameType> lstModules;
	DWORD NoOfStaticImportModules;
	vector<PEReadWrite::ImportFunction> lstFunctions;
	vector<PropertyPageHandler::TextAndData> ImportsDirInfo;
	vector<RTTI::GenericTooltip> ImportsDirTooltipInfo;
	TabPages TabPage;
	SortOrder lstModulesSortOrder;
	SortOrder lstImportsSortOrder;

	void chkImportsUnmangleCPPNames_OnClick(HWND hControl, bool bChecked);
	void lstImportsModules_OnSelection(HWND hControl, int SelectedIndex);
	void lstImportsModules_OnColumnHeaderClick(int Index);
	void lstImportsAndDirTable_OnColumnHeaderClick(int Index);
	void lstImportsAndDirTable_OnContextMenu(LONG x, LONG y, int Index);
	tstring lstImportsAndDirTable_OnGetTooltip(int Index);
	void tabsImports_OnTabChanged(HWND hControl, int SelectedIndex);
	void ImportsPage_UpdateDisplay(bool ListViewImportsModules_Changed,
									bool TabsImports_Changed,
									bool ChkBoxUnmangle_Changed);

public:
	PropertyPageHandler_Imports(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter),
			TabPage(TabImports),
			lstModulesSortOrder(None),
			lstImportsSortOrder(None) {}

	void OnInitDialog();
	void OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl);
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Exports page
class PropertyPageHandler_Exports : public PropertyPageHandler
{
private:
	struct RVANameOrdinalForwarder
	{
		tstring RVA;
		tstring Name;
		tstring Ordinal;
		tstring Forwarder;

		RVANameOrdinalForwarder(tstring rva, tstring name, tstring ordinal, tstring forwarder)
			: RVA(rva), Name(name), Ordinal(ordinal), Forwarder(forwarder) { }
	};

	HWND hListViewExportDir;
	HWND hListViewExports;
	HWND hCheckBoxCPPNameUnmangle;

	vector<pair<WORD, string> > lstOrdinalsAndNames;
	vector<DWORD> lstExportRVAs;
	vector<PropertyPageHandler::TextAndData> ExportDirInfo;
	vector<RTTI::GenericTooltip> ExportDirTooltipInfo;
	SortOrder lstExportsSortOrder;

	tstring lstExportDir_OnGetTooltip(int Index);
	void lstExportDir_OnContextMenu(LONG x, LONG y, int Index);
	void chkExportsUnmangleCPPNames_OnClick(HWND hControl, bool bChecked);
	void lstExports_OnColumnHeaderClick(int Index);
	void ExportsPage_UpdateDisplay(bool ChkBoxUnmangle_Checked);

public:
	PropertyPageHandler_Exports(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter),
			lstExportsSortOrder(None) {}

	void OnInitDialog();
	void OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl);
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Resources page
class PropertyPageHandler_Resources : public PropertyPageHandler
{
public:
	PropertyPageHandler_Resources(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter) {}

	void OnInitDialog();
};

// For Exception Handling page
class PropertyPageHandler_ExceptionHandling : public PropertyPageHandler
{
private:
	HWND hEditExceptions;

	void tabsExceptionHandlers_OnTabChanged(HWND hControl, int SelectedIndex);

public:
	PropertyPageHandler_ExceptionHandling(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter) {}

	void OnInitDialog();
};

// For Base Relocation Data page
class PropertyPageHandler_BaseReloc : public PropertyPageHandler
{
private:
	HWND hTabsBaseReloc;
	HWND hListViewBaseRelocTable;
	HWND hEditFixupEntries;

	vector<PropertyPageHandler::TextAndData> BaseRelocInfo;
	vector<RTTI::GenericTooltip> BaseRelocTooltipInfo;

	tstring lstBaseRelocTable_OnGetTooltip(int Index);
	void lstBaseRelocTable_OnContextMenu(LONG x, LONG y, int Index);
	void tabsBaseRelocations_OnTabChanged(HWND hControl, int SelectedIndex);

public:
	PropertyPageHandler_BaseReloc(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter) {}

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Debug Data page
class PropertyPageHandler_Debug : public PropertyPageHandler
{
private:
	HWND hTabsDebugDirs;
	HWND hListViewDebugDir;
	HWND hEditDebugData;

	vector<PropertyPageHandler::TextAndData> DebugDirInfo;
	vector<RTTI::GenericTooltip> DebugDirTooltipInfo;

	tstring lstDebugDir_OnGetTooltip(int Index);
	void lstDebugDir_OnContextMenu(LONG x, LONG y, int Index);
	void tabsDebugDirs_OnTabChanged(HWND hControl, int SelectedIndex);

public:
	PropertyPageHandler_Debug(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter) {}

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Thread Local Storage page
class PropertyPageHandler_TLS : public PropertyPageHandler
{
private:
	void tabsTLSCallbacksIndexes_OnTabChanged(HWND hControl, int SelectedIndex);

public:
	PropertyPageHandler_TLS(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter) {}

	void OnInitDialog();
};

// For Load Configuration page
class PropertyPageHandler_LoadConfiguration : public PropertyPageHandler
{
private:
	HWND hListViewLoadConfig;
	HWND hListViewSEH;

	vector<RTTI::GenericTooltip> LoadConfigTooltipInfo;
	vector<PropertyPageHandler::TextAndData> LoadConfigItemsInfo;

	tstring lstLoadConfig_OnGetTooltip(int Index);
	void lstLoadConfig_OnContextMenu(LONG x, LONG y, int Index);

public:
	PropertyPageHandler_LoadConfiguration(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter) {}

	void OnInitDialog();
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
};

// For Common Language Runtime Data page
class PropertyPageHandler_CLR : public PropertyPageHandler
{
private:
	HWND hTabsCLRData;
	HWND hListViewCLRData;
	HWND hEditCLRData;

	void tabsCLRData_OnTabChanged(HWND hControl, int SelectedIndex);

public:
	PropertyPageHandler_CLR(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter) {}

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
		Rect R;

		SelectionRectData(DWORD rva, DWORD size, Rect& r)
			: RVA(rva), Size(size), R(r) {}
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

	static const int LEGENDS_X = MEMORYLAYOUT_X + MEMORYLAYOUT_WIDTH + SELECTION_RECT_ADDR_SPACING + 30;
	static const int LEGENDS_Y = MEMORYLAYOUT_Y;
	int LEGENDS_WIDTH;
	static const int LEGENDS_HEIGHT = 280;

	static const int CUSTOM_X = LEGENDS_X;
	static const int CUSTOM_Y = LEGENDS_HEIGHT + LEGENDS_Y + 4;

	static const int CAPTION_Y_DIFF = 15;

	static const int SELECTION_RECT_ADDR_X = MARGIN_X + MEMORYLAYOUT_X + MEMORYLAYOUT_WIDTH - 4;

	ULONG_PTR gdiplusToken;
	Bitmap *pbitmapMemoryMap;
	DWORD HighestRVA;
	vector<SelectionRectData> vectorSelectionRects;
	int SelectionRectIndex;
	HTREEITEM hTreeViewCustomItem;

	HWND hTreeViewLegends;
	HWND hStaticCustom;
	HWND hStaticCustomRVA;
	HWND hStaticCustomSize;
	HWND hEditCustomRVA;
	HWND hEditCustomSize;

	void tvwLegends_OnSelection(HWND hControl, NMTVITEMCHANGE *pItemChange);
	void txtCustomRVA_OnLostFocus(HWND hControl);
	void txtCustomSize_OnLostFocus(HWND hControl);

public:
	PropertyPageHandler_Overview(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter),
			gdiplusToken(0), pbitmapMemoryMap(NULL),
			HighestRVA(0), SelectionRectIndex(-1),
			hTreeViewCustomItem(NULL) {}

	void OnInitDialog();
	void OnShowWindow();
	void OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl);
	void OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam);
	void OnPaint(HDC hdc, const RECT& rectUpdate);
	void OnDestroy();
};

// For Tools page
class PropertyPageHandler_Tools : public PropertyPageHandler
{
private:
	HWND hComboConvertAddrFrom;
	HWND hEditConvertAddrFrom;
	HWND hComboConvertAddrTo;
	HWND hEditConvertAddrTo;
	HWND hBtnConvertAddr;
	HWND hEditSHA1Hash;
	HWND hEditMD5Hash;
	HWND hEditVerifyHash;

	HBRUSH hbrushGreen;
	HBRUSH hbrushRed;

	HANDLE hbitmapCorrect;
	HANDLE hbitmapIncorrect;

public:
	PropertyPageHandler_Tools(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter) {}

	void OnInitDialog();
	void OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl);

	void btnConvertAddr_OnClick();
	void txtVerifyHash_Changed();
};


typedef struct CompareFuncParam
{
	HWND hListView;
	int iColumn;
	SortOrder Order;
} *pCompareFuncParam;

int CALLBACK StringCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK NumberCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK HexNumberCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);