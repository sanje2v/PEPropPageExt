#include "PropertyPageHandler.h"
#include <Uxtheme.h>

// Initialization of static variables for 'PropertyPageHandler'
// Set preferred font used for Rich Edit Control
const LPWSTR PropertyPageHandler::szPreferredFont = L"MS Shell Dlg";

// Definition of listview column header texts for generic listviews
LPWSTR const PropertyPageHandler::szGenericColumnText[3] = { L"Field", L"Data", L"Annotation" };

// Structure pairing dialog resource id and its title
const PropertyPageHandler::PropertyPageData PropertyPageHandler::PropertyPagesData[NUM_OF_PAGES] =
{
	{ IDD_PROPPAGE_MSDOSHEADER, L"MS-DOS Header" },
	{ IDD_PROPPAGE_PEHEADERS, L"PE Headers" },
	{ IDD_PROPPAGE_SECTIONS, L"Sections" },
	{ IDD_PROPPAGE_MANIFEST, L"Manifest" },
	{ IDD_PROPPAGE_IMPORTS, L"Imports" },
	{ IDD_PROPPAGE_EXPORTS, L"Exports" },
	{ IDD_PROPPAGE_RESOURCES, L"Resources" },
	{ IDD_PROPPAGE_EXCEPTION, L"Exception Handling" },
	{ IDD_PROPPAGE_BASERELOC, L"Base Reloc Data" },
	{ IDD_PROPPAGE_DEBUG, L"Debug Info" },
	{ IDD_PROPPAGE_TLS, L"TLS Data" },
	{ IDD_PROPPAGE_LOADCONFIG, L"Load Config" },
	{ IDD_PROPPAGE_CLR, L"CLR Data" },
	{ IDD_PROPPAGE_OVERVIEW, L"Overview" },
	{ IDD_PROPPAGE_TOOLS, L"Tools" }
};

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

HBRUSH PropertyPageHandler_Manifest::OnControlColorStatic(HDC hDC, HWND hControl)
{
	if (hControl == m_hImgInfo)
		return NULL;
	else
		return PropertyPageHandler::OnControlColorStatic(hDC, hControl);
}

void PropertyPageHandler_Imports::OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl)
{
	switch (ControlID)
	{
		case IDC_CHKCPPNAMEUNMANGLEIMPORTS:	// Import page's Unmangle checkbox
			if (NotificationCode == BN_CLICKED)
				chkImportsUnmangleCPPNames_OnClick(hControl, (Button_GetCheck(hControl) == BST_CHECKED));

			break;
	}
}

HBRUSH PropertyPageHandler_Imports::OnControlColorStatic(HDC hDC, HWND hControl)
{
	// NOTE: We want the default window procedure to handle this message.
	// Returning 'NULL' here will make 'CPEPropPageExt::PropertyPagesProc' do so.
	if (hControl == m_hCheckBoxCPPNameUnmangle)
	{
		SetBkMode(hDC, TRANSPARENT);
		HBRUSH hParentBackgroundBrush = HBRUSH(GetClassLongPtr(m_hTabsImports, GCLP_HBRBACKGROUND));

		return hParentBackgroundBrush;
	}
	else
		return NULL;
}

HBRUSH PropertyPageHandler_Imports::OnControlColorButton(HDC hDC, HWND hControl)
{
	if (hControl == m_hCheckBoxCPPNameUnmangle)
	{
		SetBkMode(hDC, TRANSPARENT);
		HBRUSH hParentBackgroundBrush = HBRUSH(GetClassLongPtr(m_hTabsImports, GCLP_HBRBACKGROUND));

		return hParentBackgroundBrush;
	}
	else
		return NULL;
}

void PropertyPageHandler_Exports::OnCommand(WORD NotificationCode, WORD ControlID, HWND hControl)
{
	switch (ControlID)
	{
		case IDC_CHKCPPNAMEUNMANGLEEXPORTS:	// Export page's Unmangle checkbox
			if (NotificationCode == BN_CLICKED)
				chkExportsUnmangleCPPNames_OnClick(hControl, (Button_GetCheck(hControl) == BST_CHECKED));

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

			break;

		case IDC_EDITHEXVIEWADDR:
			if (NotificationCode == EN_CHANGE)
				txtHexViewerAddress_Changed();

			break;

		case IDC_CHKHEXVIEWDISASSEMBLE:
			if (NotificationCode == BN_CLICKED)
				chkHexViewDisassemble_OnClick(hControl, (Button_GetCheck(hControl) == BST_CHECKED));

			break;

		case IDC_CMBHEXVIEWADDRTYPE:
			if (NotificationCode == CBN_SELCHANGE)
			{
				int SelectedIndex = ComboBox_GetCurSel(hControl);
				if (SelectedIndex >= 0)
					cmbHexViewAddrType_OnSelectionChanged(hControl, SelectedIndex);
			}

			break;

		case IDC_CMBHEXVIEWDATASIZE:
			if (NotificationCode == CBN_SELCHANGE)
			{
				int SelectedIndex = ComboBox_GetCurSel(hControl);
				if (SelectedIndex >= 0)
					cmbHexViewDataSize_OnSelectionChanged(hControl, SelectedIndex);
			}

			break;
	}
}

HBRUSH PropertyPageHandler_Tools::OnControlColorStatic(HDC hDC, HWND hControl)
{
	if (hControl == m_hEditSHA1Hash || hControl == m_hEditMD5Hash || hControl == m_hStaticVerify)
		return NULL;

	return PropertyPageHandler::OnControlColorStatic(hDC, hControl);
}

void PropertyPageHandler_MSDOSHeader::OnNotify(UINT NotificationCode,
											   UINT_PTR ControlID,
											   HWND hControl,
											   LPARAM lParam)
{
	switch (ControlID)
	{
		case IDC_LISTMSDOSHEADERDATA:
			switch (NotificationCode)
			{
				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP pGetInfoTip = LPNMLVGETINFOTIP(lParam);

					if (pGetInfoTip != NULL)
					{
						if (pGetInfoTip->iItem < 0)
							break;

						wstring TooltipText = lstMSDOSHeader_OnGetTooltip(pGetInfoTip->iItem);
						int cTooltipText = (int(TooltipText.size() + 1) <= pGetInfoTip->cchTextMax ?
											int(TooltipText.size() + 1) : pGetInfoTip->cchTextMax);

						CopyMemory(pGetInfoTip->pszText,
								   TooltipText.c_str(),
								   cTooltipText * sizeof(WCHAR));
					}
				}

				break;

				case NM_RCLICK:
				{
					LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE)lParam;

					if (pItem != NULL)
					{
						if (pItem->iItem < 0)
							break;

						ClientToScreen(hControl, &pItem->ptAction);
						lstMSDOSHeader_OnContextMenu(pItem->ptAction.x, pItem->ptAction.y, pItem->iItem);
					}
				}

				break;
			}

			break;
	}
}

void PropertyPageHandler_PEHeaders::OnNotify(UINT NotificationCode,
											 UINT_PTR ControlID,
											 HWND hControl,
											 LPARAM lParam)
{
	switch (ControlID)
	{
		case IDC_LISTCOFFHEADERDATA:
			switch (NotificationCode)
			{
				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP pGetInfoTip = LPNMLVGETINFOTIP(lParam);

					if (pGetInfoTip != NULL)
					{
						if (pGetInfoTip->iItem < 0)
							break;

						wstring TooltipText = lstCOFFHeader_OnGetTooltip(pGetInfoTip->iItem);
						int cTooltipText = (int(TooltipText.size() + 1) <= pGetInfoTip->cchTextMax ?
											int(TooltipText.size() + 1) : pGetInfoTip->cchTextMax);

						CopyMemory(pGetInfoTip->pszText,
								   TooltipText.c_str(),
								   cTooltipText * sizeof(WCHAR));
					}
				}

				break;

				case NM_RCLICK:
				{
					LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE)lParam;

					if (pItem != NULL)
					{
						if (pItem->iItem < 0)
							break;

						ClientToScreen(hControl, &pItem->ptAction);
						lstCOFFHeader_OnContextMenu(pItem->ptAction.x, pItem->ptAction.y, pItem->iItem);
					}
				}

				break;
			}

			break;

		case IDC_LISTOPTIONALHEADERDATA:
			switch (NotificationCode)
			{
				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP pGetInfoTip = LPNMLVGETINFOTIP(lParam);

					if (pGetInfoTip != NULL)
					{
						if (pGetInfoTip->iItem < 0)
							break;

						wstring TooltipText = lstOptionalHeader_OnGetTooltip(pGetInfoTip->iItem);
						int cTooltipText = (int(TooltipText.size() + 1) <= pGetInfoTip->cchTextMax ?
											int(TooltipText.size() + 1) : pGetInfoTip->cchTextMax);

						CopyMemory(pGetInfoTip->pszText,
								   TooltipText.c_str(),
								   cTooltipText * sizeof(WCHAR));
					}
				}

				break;

				case NM_RCLICK:
				{
					LPNMITEMACTIVATE pItem = LPNMITEMACTIVATE(lParam);

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

void PropertyPageHandler_Sections::OnNotify(UINT NotificationCode,
											UINT_PTR ControlID,
											HWND hControl,
											LPARAM lParam)
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
					LPNMLVGETINFOTIP pGetInfoTip = LPNMLVGETINFOTIP(lParam);

					if (pGetInfoTip != NULL)
					{
						if (pGetInfoTip->iItem < 0)
							break;

						wstring TooltipText = lstSections_OnGetTooltip(pGetInfoTip->iItem);
						int cTooltipText = (int(TooltipText.size() + 1) <= pGetInfoTip->cchTextMax ?
											int(TooltipText.size() + 1) : pGetInfoTip->cchTextMax);

						CopyMemory(pGetInfoTip->pszText,
								   TooltipText.c_str(),
								   cTooltipText * sizeof(WCHAR));
					}
				}

				break;

				case NM_RCLICK:
				{
					LPNMITEMACTIVATE pItem = LPNMITEMACTIVATE(lParam);

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
					LPNMLISTVIEW pItemState = LPNMLISTVIEW(lParam);

					if (pItemState->iItem < 0)
						break;

					if (TestFlag(pItemState->uNewState, LVIS_SELECTED))
						lstImportsModules_OnSelection(hControl, pItemState->iItem);
				}

				break;

				case LVN_COLUMNCLICK:
				{
					LPNMLISTVIEW pItem = LPNMLISTVIEW(lParam);

					lstImportsModules_OnColumnHeaderClick(pItem->iSubItem);
				}

				break;
			}

			break;

		case IDC_LISTIMPORTSFUNCSANDDIRTABLE:
			switch (NotificationCode)
			{
				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP pGetInfoTip = LPNMLVGETINFOTIP(lParam);

					if (pGetInfoTip != NULL)
					{
						if (pGetInfoTip->iItem < 0)
							break;

						wstring TooltipText = lstImportsAndDirTable_OnGetTooltip(pGetInfoTip->iItem);
						int cTooltipText = (int(TooltipText.size() + 1) <= pGetInfoTip->cchTextMax ?
											int(TooltipText.size() + 1) : pGetInfoTip->cchTextMax);

						CopyMemory(pGetInfoTip->pszText,
								   TooltipText.c_str(),
								   cTooltipText * sizeof(WCHAR));
					}
				}

				break;

				case NM_RCLICK:
				{
					LPNMITEMACTIVATE pItem = LPNMITEMACTIVATE(lParam);

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
					LPNMLISTVIEW pItem = LPNMLISTVIEW(lParam);

					lstImportsAndDirTable_OnColumnHeaderClick(pItem->iSubItem);
				}
			}

			break;
	}
}

void PropertyPageHandler_Exports::OnNotify(UINT NotificationCode,
										   UINT_PTR ControlID,
										   HWND hControl,
										   LPARAM lParam)
{
	switch (ControlID)
	{
		case IDC_LISTEXPORTDIRECTORYTABLE:
			switch (NotificationCode)
			{
				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP pGetInfoTip = LPNMLVGETINFOTIP(lParam);

					if (pGetInfoTip != NULL)
					{
						if (pGetInfoTip->iItem < 0)
							break;

						wstring TooltipText = lstExportDir_OnGetTooltip(pGetInfoTip->iItem);
						int cTooltipText = (int(TooltipText.size() + 1) <= pGetInfoTip->cchTextMax ?
											int(TooltipText.size() + 1) : pGetInfoTip->cchTextMax);

						CopyMemory(pGetInfoTip->pszText,
								   TooltipText.c_str(),
								   cTooltipText * sizeof(WCHAR));
					}
				}

				break;

				case NM_RCLICK:
				{
					LPNMITEMACTIVATE pItem = LPNMITEMACTIVATE(lParam);

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
					LPNMLISTVIEW pItem = LPNMLISTVIEW(lParam);

					lstExports_OnColumnHeaderClick(pItem->iSubItem);
				}

				break;

				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP pGetInfoTip = LPNMLVGETINFOTIP(lParam);

					if (pGetInfoTip != NULL)
					{
						if (pGetInfoTip->iItem < 0)
							break;

						wstring TooltipText = lstExportDir_OnGetTooltip(pGetInfoTip->iItem);
						int cTooltipText = (int(TooltipText.size() + 1) <= pGetInfoTip->cchTextMax ?
											int(TooltipText.size() + 1) : pGetInfoTip->cchTextMax);

						CopyMemory(pGetInfoTip->pszText,
								   TooltipText.c_str(),
								   cTooltipText * sizeof(WCHAR));
					}
				}

				break;

				case NM_RCLICK:
				{
					LPNMITEMACTIVATE pItem = LPNMITEMACTIVATE(lParam);

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

void PropertyPageHandler_BaseReloc::OnNotify(UINT NotificationCode,
											 UINT_PTR ControlID,
											 HWND hControl,
											 LPARAM lParam)
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
					LPNMLVGETINFOTIP pGetInfoTip = LPNMLVGETINFOTIP(lParam);

					if (pGetInfoTip != NULL)
					{
						if (pGetInfoTip->iItem < 0)
							break;

						wstring TooltipText = lstBaseRelocTable_OnGetTooltip(pGetInfoTip->iItem);
						int cTooltipText = (int(TooltipText.size() + 1) <= pGetInfoTip->cchTextMax ?
											int(TooltipText.size() + 1) : pGetInfoTip->cchTextMax);

						CopyMemory(pGetInfoTip->pszText,
								   TooltipText.c_str(),
								   cTooltipText * sizeof(WCHAR));
					}
				}

				break;

				case NM_RCLICK:
				{
					LPNMITEMACTIVATE pItem = LPNMITEMACTIVATE(lParam);

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

void PropertyPageHandler_Debug::OnNotify(UINT NotificationCode,
										 UINT_PTR ControlID,
										 HWND hControl,
										 LPARAM lParam)
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
					LPNMLVGETINFOTIP pGetInfoTip = LPNMLVGETINFOTIP(lParam);

					if (pGetInfoTip != NULL)
					{
						if (pGetInfoTip->iItem < 0)
							break;

						wstring TooltipText = lstDebugDir_OnGetTooltip(pGetInfoTip->iItem);
						int cTooltipText = (int(TooltipText.size() + 1) <= pGetInfoTip->cchTextMax ?
											int(TooltipText.size() + 1) : pGetInfoTip->cchTextMax);

						CopyMemory(pGetInfoTip->pszText,
								   TooltipText.c_str(),
								   cTooltipText * sizeof(WCHAR));
					}
				}

				break;

				case NM_RCLICK:
				{
					LPNMITEMACTIVATE pItem = LPNMITEMACTIVATE(lParam);

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

void PropertyPageHandler_LoadConfiguration::OnNotify(UINT NotificationCode,
													 UINT_PTR ControlID,
													 HWND hControl,
													 LPARAM lParam)
{
	switch (ControlID)
	{
		case IDC_LISTLOADCONFIG:
			switch (NotificationCode)
			{
				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP pGetInfoTip = LPNMLVGETINFOTIP(lParam);

					if (pGetInfoTip != NULL)
					{
						if (pGetInfoTip->iItem < 0)
							break;

						wstring TooltipText = lstLoadConfig_OnGetTooltip(pGetInfoTip->iItem);
						int cTooltipText = (int(TooltipText.size() + 1) <= pGetInfoTip->cchTextMax ?
											int(TooltipText.size() + 1) : pGetInfoTip->cchTextMax);

						CopyMemory(pGetInfoTip->pszText,
								   TooltipText.c_str(),
								   cTooltipText * sizeof(WCHAR));
					}
				}

				break;

				case NM_RCLICK:
				{
					LPNMITEMACTIVATE pItem = LPNMITEMACTIVATE(lParam);

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

void PropertyPageHandler_Resources::OnNotify(UINT NotificationCode,
											 UINT_PTR ControlID,
											 HWND hControl,
											 LPARAM lParam)
{
	switch (ControlID)
	{
		case IDC_TREERESOURCEENTRIES:
			if (NotificationCode == TVN_ITEMCHANGED)
			{
				NMTVITEMCHANGE *pItemState = (NMTVITEMCHANGE *)(lParam);

				if (!pItemState->hItem)
					break;

				if (TestFlag(pItemState->uStateNew, TVIS_SELECTED))
					tvwResourceInfo_OnSelection(hControl, pItemState);
			}

			break;

		case IDC_LISTRESOURCEINFO:
			if (NotificationCode == LVN_GETINFOTIP)
			{
				LPNMLVGETINFOTIP pGetInfoTip = LPNMLVGETINFOTIP(lParam);

				if (pGetInfoTip != NULL)
				{
					if (pGetInfoTip->iItem < 0)
						break;

					wstring TooltipText = lstResourceInfo_OnGetTooltip(pGetInfoTip->iItem);
					int cTooltipText = (int(TooltipText.size() + 1) <= pGetInfoTip->cchTextMax ?
										int(TooltipText.size() + 1) : pGetInfoTip->cchTextMax);

					CopyMemory(pGetInfoTip->pszText,
							   TooltipText.c_str(),
							   cTooltipText * sizeof(WCHAR));
				}
			}

			break;
	}
}

void PropertyPageHandler_CLR::OnNotify(UINT NotificationCode,
									   UINT_PTR ControlID,
									   HWND hControl,
									   LPARAM lParam)
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

void PropertyPageHandler_Overview::OnNotify(UINT NotificationCode,
											UINT_PTR ControlID,
											HWND hControl,
											LPARAM lParam)
{
	switch (ControlID)
	{
		case IDC_TREELEGENDS:
			if (NotificationCode == TVN_ITEMCHANGED)
			{
				NMTVITEMCHANGE *pItemState = (NMTVITEMCHANGE *)(lParam);

				if (!pItemState->hItem)
					break;

				if (TestFlag(pItemState->uStateNew, TVIS_SELECTED))
					tvwLegends_OnSelection(hControl, pItemState);
			}

			break;
	}
}

HBRUSH PropertyPageHandler_Overview::OnControlColorStatic(HDC hDC, HWND hControl)
{
	return m_hBackgroundBrush.get();
}