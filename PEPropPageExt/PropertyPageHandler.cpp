#include "PropertyPageHandler.h"
#include "CommonDefs.h"


// Definition of listview column header texts for generic listviews
LPTSTR const PropertyPageHandler::GenericColumnText[3] = {_T("Field"), _T("Data"), _T("Annotation")};

// Structure pairing dialog resource id and its title
const PropertyPageHandler::PropertyPageData PropertyPageHandler::PropertyPagesData[NUM_OF_PAGES] = 
{
	{IDD_PROPPAGE_MSDOSHEADER,			_T("MS-DOS Header")},
	{IDD_PROPPAGE_PEHEADERS,			_T("PE Headers")},
	{IDD_PROPPAGE_SECTIONS,				_T("Sections")},
	{IDD_PROPPAGE_MANIFEST,				_T("Manifest")},
	{IDD_PROPPAGE_IMPORTS,				_T("Imports")},
	{IDD_PROPPAGE_EXPORTS,				_T("Exports")},
	{IDD_PROPPAGE_RESOURCES,			_T("Resources")},
	{IDD_PROPPAGE_EXCEPTION,			_T("Exception Handling")},
	{IDD_PROPPAGE_BASERELOC,			_T("Base Reloc Data")},
	{IDD_PROPPAGE_DEBUG,				_T("Debug Info")},
	{IDD_PROPPAGE_TLS,					_T("TLS Data")},
	{IDD_PROPPAGE_LOADCONFIG,			_T("Load Config")},
	{IDD_PROPPAGE_CLR,					_T("CLR Data")},
	{IDD_PROPPAGE_OVERVIEW,				_T("Overview")},
	{IDD_PROPPAGE_TOOLS,				_T("Tools")}
};

tstring PropertyPageHandler::Generic_OnGetTooltip(vector<RTTI::GenericTooltip>& TooltipInfo, int Index)
{
	return TooltipInfo[Index].FullName +
			_T("\nType: ") + TooltipInfo[Index].Type +
			_T("\nSize: ") + TooltipInfo[Index].Size +
			_T("\nFile offset: ") + TooltipInfo[Index].FileOffset;
}

void PropertyPageHandler::Generic_OnContextMenu(vector<RTTI::GenericTooltip>& TooltipInfo,
												vector<PropertyPageHandler::TextAndData>& ItemsInfo,
												LONG x, LONG y, int Index)
{
	if (!OpenClipboard(m_hWnd))
		return;

	HMENU hContextMenu = GetSubMenu(hGenericContextMenu, 0);	// Must select first subitem for context menu to work
	tstring Data;

	switch (TrackPopupMenu(hContextMenu, TPM_NONOTIFY | TPM_RETURNCMD, x, y, 0, m_hWnd, NULL))
	{
	case ID_COPYVALUE:
		Data = ItemsInfo[Index].szData;

		break;

	case ID_COPYANNOTATION:
		Data = ItemsInfo[Index].szComments;

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
	HGLOBAL hGlobalData = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR) * (Data.size() + 1));
	LPVOID pGlobalData;
	if (hGlobalData)
	{
		if ((pGlobalData = GlobalLock(hGlobalData)) == 0)
		{
			GlobalFree(hGlobalData);
			CloseClipboard();

			return;
		}

		CopyMemory(pGlobalData, Data.c_str(), sizeof(TCHAR) * (Data.size() + 1));
		GlobalUnlock(hGlobalData);

		EmptyClipboard();
#ifdef UNICODE
		SetClipboardData(CF_UNICODETEXT, hGlobalData);
#else
		SetClipboardData(CF_TEXT, pGlobalData);
#endif
	}

	CloseClipboard();
}

void PropertyPageHandler::OnSize(WPARAM wParam, LPARAM lParam)
{
	// When notified about page resize ask layout manager to do so
	m_pLayoutManager->DoLayout(wParam, lParam);
}

void PropertyPageHandler_Manifest::OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl)
{
	switch (ControlID)
	{
	case IDC_CMBMANIFESTNAME:			// Manifest page's Name Combobox
		if (NotificationCode == CBN_SELCHANGE)
		{
			int SelectedIndex = ComboBox_GetCurSel(hControl);
			if (SelectedIndex >= 0)
				cmbManifestName_OnSelectionChanged(hControl, SelectedIndex);
		}

		break;

	case IDC_CMBMANIFESTLANG:			// Manifest page's Language Combobox
		if (NotificationCode == CBN_SELCHANGE)
		{
			int SelectedIndex = ComboBox_GetCurSel(hControl);
			if (SelectedIndex >= 0)
				cmbManifestLang_OnSelectionChanged(hControl, SelectedIndex);
		}

		break;
	}
}

void PropertyPageHandler_Imports::OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl)
{
	switch (ControlID)
	{
	case IDC_CHKCPPNAMEUNMANGLEIMPORTS:	// Import page's Unmangle checkbox
		if (NotificationCode == BN_CLICKED)
			chkImportsUnmangleCPPNames_OnClick(hControl, Button_GetCheck(hControl) == BST_CHECKED);

		break;
	}
}

void PropertyPageHandler_Exports::OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl)
{
	switch (ControlID)
	{
	case IDC_CHKCPPNAMEUNMANGLEEXPORTS:	// Export page's Unmangle checkbox
		if (NotificationCode == BN_CLICKED)
			chkExportsUnmangleCPPNames_OnClick(hControl, Button_GetCheck(hControl) == BST_CHECKED);

		break;
	}
}

void PropertyPageHandler_Overview::OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl)
{
	switch (ControlID)
	{
	case IDC_EDITCUSTOMRVA:
		if (NotificationCode == EN_KILLFOCUS)
			txtCustomRVA_OnLostFocus(hControl);

		break;

	case IDC_EDITCUSTOMSIZE:
		if (NotificationCode == EN_KILLFOCUS)
			txtCustomSize_OnLostFocus(hControl);

		break;
	}
}

void PropertyPageHandler_Tools::OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl)
{
	switch (ControlID)
	{
	case IDC_BTNCONVERTADDR:
		if (NotificationCode == BN_CLICKED)
			btnConvertAddr_OnClick();

		break;

	case IDC_EDITVERIFYHASH:
		if (NotificationCode == EN_CHANGE)
			txtVerifyHash_Changed();
	}
}

void PropertyPageHandler_MSDOSHeader::OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam)
{
	switch (ControlID)
	{
	case IDC_LISTMSDOSHEADERDATA:
		switch (NotificationCode)
		{
		case LVN_GETINFOTIP:
			{
				LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP) lParam;

				if (pGetInfoTip != NULL)
				{
					if (pGetInfoTip->iItem < 0)
						break;

					tstring TooltipText = lstMSDOSHeader_OnGetTooltip(pGetInfoTip->iItem);
					int cTooltipText = (int) (TooltipText.size() + 1 <= pGetInfoTip->cchTextMax ?
												TooltipText.size() + 1 : pGetInfoTip->cchTextMax);
				
					CopyMemory(pGetInfoTip->pszText,
								TooltipText.c_str(),
								cTooltipText * sizeof(TCHAR));
				}
			}

			break;

		case NM_RCLICK:
			{
				LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) lParam;

				if (pItem != NULL)
				{
					if (pItem->iItem < 0)
						break;

					ClientToScreen(hControl, &pItem->ptAction);
					lstMSDOSHeader_OnContextMenu(pItem->ptAction.x, pItem->ptAction.y, pItem->iItem);
				}
			}
		}

		break;
	}
}

void PropertyPageHandler_PEHeaders::OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam)
{
	switch (ControlID)
	{
	case IDC_LISTCOFFHEADERDATA:
		switch (NotificationCode)
		{
		case LVN_GETINFOTIP:
			{
				LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP) lParam;

				if (pGetInfoTip != NULL)
				{
					if (pGetInfoTip->iItem < 0)
						break;

					tstring TooltipText = lstCOFFHeader_OnGetTooltip(pGetInfoTip->iItem);
					int cTooltipText = (int) (TooltipText.size() + 1 <= pGetInfoTip->cchTextMax ?
												TooltipText.size() + 1 : pGetInfoTip->cchTextMax);
				
					CopyMemory(pGetInfoTip->pszText,
								TooltipText.c_str(),
								cTooltipText * sizeof(TCHAR));
				}
			}

			break;

		case NM_RCLICK:
			{
				LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) lParam;

				if (pItem != NULL)
				{
					if (pItem->iItem < 0)
						break;

					ClientToScreen(hControl, &pItem->ptAction);
					lstCOFFHeader_OnContextMenu(pItem->ptAction.x, pItem->ptAction.y, pItem->iItem);
				}
			}
		}

		break;

	case IDC_LISTOPTIONALHEADERDATA:
		switch (NotificationCode)
		{
		case LVN_GETINFOTIP:
			{
				LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP) lParam;

				if (pGetInfoTip != NULL)
				{
					if (pGetInfoTip->iItem < 0)
						break;

					tstring TooltipText = lstOptionalHeader_OnGetTooltip(pGetInfoTip->iItem);
					int cTooltipText = (int) (TooltipText.size() + 1 <= pGetInfoTip->cchTextMax ?
												TooltipText.size() + 1 : pGetInfoTip->cchTextMax);
				
					CopyMemory(pGetInfoTip->pszText,
								TooltipText.c_str(),
								cTooltipText * sizeof(TCHAR));
				}
			}

			break;

		case NM_RCLICK:
			{
				LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) lParam;

				if (pItem != NULL)
				{
					if (pItem->iItem < 0)
						break;

					ClientToScreen(hControl, &pItem->ptAction);
					lstOptionalHeader_OnContextMenu(pItem->ptAction.x, pItem->ptAction.y, pItem->iItem);
				}
			}
		}

		break;
	}
}

void PropertyPageHandler_Sections::OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam)
{
	switch (ControlID)
	{
	case IDC_TABSSECTIONS:	// Section page's sections tabs
		if (NotificationCode == TCN_SELCHANGE)
		{
			int SelectedIndex = TabCtrl_GetCurSel(hControl);
			if (SelectedIndex < 0)
				break;

			tabsSections_OnTabChanged(hControl, SelectedIndex);
		}

		break;

	case IDC_LISTSECTIONS:
		switch (NotificationCode)
		{
		case LVN_GETINFOTIP:
			{
				LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP) lParam;

				if (pGetInfoTip != NULL)
				{
					if (pGetInfoTip->iItem < 0)
						break;

					tstring TooltipText = lstSections_OnGetTooltip(pGetInfoTip->iItem);
					int cTooltipText = (int) (TooltipText.size() + 1 <= pGetInfoTip->cchTextMax ?
												TooltipText.size() + 1 : pGetInfoTip->cchTextMax);
				
					CopyMemory(pGetInfoTip->pszText,
								TooltipText.c_str(),
								cTooltipText * sizeof(TCHAR));
				}
			}

			break;

		case NM_RCLICK:
			{
				LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) lParam;

				if (pItem != NULL)
				{
					if (pItem->iItem < 0)
						break;

					ClientToScreen(hControl, &pItem->ptAction);
					lstSections_OnContextMenu(pItem->ptAction.x, pItem->ptAction.y, pItem->iItem);
				}
			}
		}

		break;
	}
}

void PropertyPageHandler_Imports::OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam)
{
	switch (ControlID)
	{
	case IDC_TABSIMPORTS:	// Import page's imports tabs
		if (NotificationCode == TCN_SELCHANGE)
		{
			int SelectedIndex = TabCtrl_GetCurSel(hControl);
			if (SelectedIndex < 0)
				break;

			tabsImports_OnTabChanged(hControl, SelectedIndex);
		}

		break;

	case IDC_LISTIMPORTSMODULES:		// Import page's list box
		switch (NotificationCode)
		{
		case LVN_ITEMCHANGED:
			{
				LPNMLISTVIEW pItemState = (LPNMLISTVIEW) lParam;

				if (pItemState->iItem < 0)
					break;

				if (TestFlag(pItemState->uNewState, LVIS_SELECTED))
					lstImportsModules_OnSelection(hControl, pItemState->iItem);
			}

			break;

		case LVN_COLUMNCLICK:
			{
				LPNMLISTVIEW pItem = (LPNMLISTVIEW) lParam;

				lstImportsModules_OnColumnHeaderClick(pItem->iSubItem);
			}
		}

		break;

	case IDC_LISTIMPORTSFUNCSANDDIRTABLE:
		switch (NotificationCode)
		{
		case LVN_GETINFOTIP:
			{
				LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP) lParam;

				if (pGetInfoTip != NULL)
				{
					if (pGetInfoTip->iItem < 0)
						break;

					tstring TooltipText = lstImportsAndDirTable_OnGetTooltip(pGetInfoTip->iItem);
					int cTooltipText = (int) (TooltipText.size() + 1 <= pGetInfoTip->cchTextMax ?
												TooltipText.size() + 1 : pGetInfoTip->cchTextMax);
				
					CopyMemory(pGetInfoTip->pszText,
								TooltipText.c_str(),
								cTooltipText * sizeof(TCHAR));
				}
			}

			break;

		case NM_RCLICK:
			{
				LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) lParam;

				if (pItem != NULL)
				{
					if (pItem->iItem < 0)
						break;

					ClientToScreen(hControl, &pItem->ptAction);
					lstImportsAndDirTable_OnContextMenu(pItem->ptAction.x, pItem->ptAction.y, pItem->iItem);
				}
			}

			break;

		case LVN_COLUMNCLICK:
			{
				LPNMLISTVIEW pItem = (LPNMLISTVIEW) lParam;

				lstImportsAndDirTable_OnColumnHeaderClick(pItem->iSubItem);
			}
		}

		break;
	}
}

void PropertyPageHandler_Exports::OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam)
{
	switch (ControlID)
	{
	case IDC_LISTEXPORTDIRECTORYTABLE:
		switch (NotificationCode)
		{
		case LVN_GETINFOTIP:
			{
				LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP) lParam;

				if (pGetInfoTip != NULL)
				{
					if (pGetInfoTip->iItem < 0)
						break;

					tstring TooltipText = lstExportDir_OnGetTooltip(pGetInfoTip->iItem);
					int cTooltipText = (int) (TooltipText.size() + 1 <= pGetInfoTip->cchTextMax ?
												TooltipText.size() + 1 : pGetInfoTip->cchTextMax);
				
					CopyMemory(pGetInfoTip->pszText,
								TooltipText.c_str(),
								cTooltipText * sizeof(TCHAR));
				}
			}

			break;

		case NM_RCLICK:
			{
				LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) lParam;

				if (pItem != NULL)
				{
					if (pItem->iItem < 0)
						break;

					ClientToScreen(hControl, &pItem->ptAction);
					lstExportDir_OnContextMenu(pItem->ptAction.x, pItem->ptAction.y, pItem->iItem);
				}
			}
		}

		break;

	case IDC_LISTEXPORTS:
		switch (NotificationCode)
		{
		case LVN_COLUMNCLICK:
			{
				LPNMLISTVIEW pItem = (LPNMLISTVIEW) lParam;

				lstExports_OnColumnHeaderClick(pItem->iSubItem);
			}

			break;

		case LVN_GETINFOTIP:
			{
				LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP) lParam;

				if (pGetInfoTip != NULL)
				{
					if (pGetInfoTip->iItem < 0)
						break;

					tstring TooltipText = lstExportDir_OnGetTooltip(pGetInfoTip->iItem);
					int cTooltipText = (int) (TooltipText.size() + 1 <= pGetInfoTip->cchTextMax ?
												TooltipText.size() + 1 : pGetInfoTip->cchTextMax);
				
					CopyMemory(pGetInfoTip->pszText,
								TooltipText.c_str(),
								cTooltipText * sizeof(TCHAR));
				}
			}

			break;

		case NM_RCLICK:
			{
				LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) lParam;

				if (pItem != NULL)
				{
					if (pItem->iItem < 0)
						break;

					ClientToScreen(hControl, &pItem->ptAction);
					lstExportDir_OnContextMenu(pItem->ptAction.x, pItem->ptAction.y, pItem->iItem);
				}
			}
		}

		break;
	}
}

void PropertyPageHandler_BaseReloc::OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam)
{
	switch (ControlID)
	{
	case IDC_TABSBASERELOC:	// Base relocation page's tabs
		if (NotificationCode == TCN_SELCHANGE)
		{
			int SelectedIndex = TabCtrl_GetCurSel(hControl);
			if (SelectedIndex < 0)
				break;

			tabsBaseRelocations_OnTabChanged(hControl, SelectedIndex);
		}

		break;

	case IDC_LISTBASERELOCTABLE:
		switch (NotificationCode)
		{
		case LVN_GETINFOTIP:
			{
				LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP) lParam;

				if (pGetInfoTip != NULL)
				{
					if (pGetInfoTip->iItem < 0)
						break;

					tstring TooltipText = lstBaseRelocTable_OnGetTooltip(pGetInfoTip->iItem);
					int cTooltipText = (int) (TooltipText.size() + 1 <= pGetInfoTip->cchTextMax ?
												TooltipText.size() + 1 : pGetInfoTip->cchTextMax);
				
					CopyMemory(pGetInfoTip->pszText,
								TooltipText.c_str(),
								cTooltipText * sizeof(TCHAR));
				}
			}

			break;

		case NM_RCLICK:
			{
				LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) lParam;

				if (pItem != NULL)
				{
					if (pItem->iItem < 0)
						break;

					ClientToScreen(hControl, &pItem->ptAction);
					lstBaseRelocTable_OnContextMenu(pItem->ptAction.x, pItem->ptAction.y, pItem->iItem);
				}
			}
		}

		break;
	}
}

void PropertyPageHandler_Debug::OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam)
{
	switch (ControlID)
	{
	case IDC_TABSDEBUGDIRS:	// Debug data page's tabs
		if (NotificationCode == TCN_SELCHANGE)
		{
			int SelectedIndex = TabCtrl_GetCurSel(hControl);
			if (SelectedIndex < 0)
				break;

			tabsDebugDirs_OnTabChanged(hControl, SelectedIndex);
		}

		break;

	case IDC_LISTDEBUGDIR:
		switch (NotificationCode)
		{
		case LVN_GETINFOTIP:
			{
				LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP) lParam;

				if (pGetInfoTip != NULL)
				{
					if (pGetInfoTip->iItem < 0)
						break;

					tstring TooltipText = lstDebugDir_OnGetTooltip(pGetInfoTip->iItem);
					int cTooltipText = (int) (TooltipText.size() + 1 <= pGetInfoTip->cchTextMax ?
												TooltipText.size() + 1 : pGetInfoTip->cchTextMax);
				
					CopyMemory(pGetInfoTip->pszText,
								TooltipText.c_str(),
								cTooltipText * sizeof(TCHAR));
				}
			}

			break;

		case NM_RCLICK:
			{
				LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) lParam;

				if (pItem != NULL)
				{
					if (pItem->iItem < 0)
						break;

					ClientToScreen(hControl, &pItem->ptAction);
					lstDebugDir_OnContextMenu(pItem->ptAction.x, pItem->ptAction.y, pItem->iItem);
				}
			}
		}

		break;
	}
}

void PropertyPageHandler_LoadConfiguration::OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam)
{
	switch (ControlID)
	{
	case IDC_LISTLOADCONFIG:
		switch (NotificationCode)
		{
		case LVN_GETINFOTIP:
			{
				LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP) lParam;

				if (pGetInfoTip != NULL)
				{
					if (pGetInfoTip->iItem < 0)
						break;

					tstring TooltipText = lstLoadConfig_OnGetTooltip(pGetInfoTip->iItem);
					int cTooltipText = (int) (TooltipText.size() + 1 <= pGetInfoTip->cchTextMax ?
												TooltipText.size() + 1 : pGetInfoTip->cchTextMax);
				
					CopyMemory(pGetInfoTip->pszText,
								TooltipText.c_str(),
								cTooltipText * sizeof(TCHAR));
				}
			}

			break;

		case NM_RCLICK:
			{
				LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE) lParam;

				if (pItem != NULL)
				{
					if (pItem->iItem < 0)
						break;

					ClientToScreen(hControl, &pItem->ptAction);
					lstLoadConfig_OnContextMenu(pItem->ptAction.x, pItem->ptAction.y, pItem->iItem);
				}
			}
		}

		break;
	}
}

void PropertyPageHandler_CLR::OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam)
{
	switch (ControlID)
	{
	case IDC_TABSCLRDATA:			// CLR Data tabs
		if (NotificationCode == TCN_SELCHANGE)
		{
			int SelectedIndex = TabCtrl_GetCurSel(hControl);
			if (SelectedIndex < 0)
				break;

			tabsCLRData_OnTabChanged(hControl, SelectedIndex);
		}

		break;
	}
}

void PropertyPageHandler_Overview::OnNotify(UINT NotificationCode, UINT_PTR ControlID, HWND hControl, LPARAM lParam)
{
	switch (ControlID)
	{
	case IDC_TREELEGENDS:
		if (NotificationCode == TVN_ITEMCHANGED)
		{
			NMTVITEMCHANGE *pItemState = (NMTVITEMCHANGE *) lParam;

			if (!pItemState->hItem)
				break;

			if (TestFlag(pItemState->uStateNew, TVIS_SELECTED))
				tvwLegends_OnSelection(hControl, pItemState);
		}

		break;
	}
}


int CALLBACK StringCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	pCompareFuncParam pParams = (pCompareFuncParam) lParamSort;
	TCHAR szBuffer1[1024], szBuffer2[1024];

	ListView_GetItemText(pParams->hListView, lParam1, pParams->iColumn, szBuffer1, GetArraySize(szBuffer1));
	ListView_GetItemText(pParams->hListView, lParam2, pParams->iColumn, szBuffer2, GetArraySize(szBuffer2));

	if (pParams->Order == Ascending)
		return _tcsicmp(szBuffer1, szBuffer2);
	else
		return _tcsicmp(szBuffer2, szBuffer1);
}

int CALLBACK NumberCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	pCompareFuncParam pParams = (pCompareFuncParam) lParamSort;
	TCHAR szBuffer1[1024], szBuffer2[1024];

	ListView_GetItemText(pParams->hListView, lParam1, pParams->iColumn, szBuffer1, GetArraySize(szBuffer1));
	ListView_GetItemText(pParams->hListView, lParam2, pParams->iColumn, szBuffer2, GetArraySize(szBuffer2));

	if (_tcslen(szBuffer1) == 0)
		return -1;
	else if (_tcslen(szBuffer2) == 0)
		return 1;

	int iBuffer1 = _tstoi(szBuffer1), iBuffer2 = _tstoi(szBuffer2);

	if (pParams->Order == Ascending)
		return iBuffer1 == iBuffer2 ? 0 : (iBuffer1 < iBuffer2 ? -1 : 1);
	else
		return iBuffer1 == iBuffer2 ? 0 : (iBuffer1 > iBuffer2 ? -1 : 1);
}

int CALLBACK HexNumberCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	pCompareFuncParam pParams = (pCompareFuncParam) lParamSort;
	TCHAR szBuffer1[1024], szBuffer2[1024];

	ListView_GetItemText(pParams->hListView, lParam1, pParams->iColumn, szBuffer1, GetArraySize(szBuffer1));
	ListView_GetItemText(pParams->hListView, lParam2, pParams->iColumn, szBuffer2, GetArraySize(szBuffer2));

	if (_tcslen(szBuffer1) == 0)
		return -1;
	else if (_tcslen(szBuffer2) == 0)
		return 1;

	int iBuffer1 = _tcstol(szBuffer1, NULL, 16), iBuffer2 = _tcstol(szBuffer2, NULL, 16);

	if (pParams->Order == Ascending)
		return iBuffer1 == iBuffer2 ? 0 : (iBuffer1 < iBuffer2 ? -1 : 1);
	else
		return iBuffer1 == iBuffer2 ? 0 : (iBuffer1 > iBuffer2 ? -1 : 1);
}