#include "PropertyPageHandler.h"


void PropertyPageHandler_Imports::OnInitDialog()
{
	hListViewImportsModules = GetDlgItem(m_hWnd, IDC_LISTIMPORTSMODULES);
	hTabsImports = GetDlgItem(m_hWnd, IDC_TABSIMPORTS);
	hListViewImportsAndDirTable = GetDlgItem(m_hWnd, IDC_LISTIMPORTSFUNCSANDDIRTABLE);
	hCheckBoxCPPNameUnmangle = GetDlgItem(m_hWnd, IDC_CHKCPPNAMEUNMANGLEIMPORTS);

	// Setup controls with layout manager
	m_pLayoutManager->AddChildConstraint(IDC_TABSIMPORTS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_pLayoutManager->AddChildConstraint(IDC_LISTIMPORTSFUNCSANDDIRTABLE, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_pLayoutManager->AddChildConstraint(IDC_CHKCPPNAMEUNMANGLEIMPORTS, CWA_DEFAULT, CWA_BOTTOM);
	// Make ZOrder: Listview (On top) and Tab Control (On bottom)
	SetWindowPos(hTabsImports, hListViewImportsAndDirTable, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	// Set full row selection style for listviews
	ListView_SetExtendedListViewStyleEx(hListViewImportsModules, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	ListView_SetExtendedListViewStyleEx(hListViewImportsAndDirTable,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

	// Fill controls with data
	static LPTSTR szModulesColumnText[] = {_T("Name"), _T("Link Type")};
	static LPTSTR szImportsTabText[] = {_T("Imports"), _T("Import Directory Table")};
	RTTI::GetTooltipInfo(ImportsDirTooltipInfo, 0, RTTI::RTTI_IMPORT_DIRECTORY);	// Get tooltip data
	vector<TextAndData> ModulesItemsInfo;
	lstModules = m_PEReaderWriter.GetImportsModules(&NoOfStaticImportModules);

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (unsigned int i = 0; i < GetArraySize(szModulesColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szModulesColumnText[i];
		ListView_InsertColumn(hListViewImportsModules, i, &column);
	}

	// Insert ListView items for Modules list view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < lstModules.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.pszText = (LPTSTR) lstModules[i].Name.c_str();
		item.lParam = (LPARAM) (i - (lstModules[i].Type == PEReadWrite::Delayed ? NoOfStaticImportModules : 0));
		ListView_InsertItem(hListViewImportsModules, &item);

		item.iSubItem = 1;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) (lstModules[i].Type == PEReadWrite::Static ? _T("Static") : _T("Delayed"));
		item.lParam = NULL;
		ListView_SetItem(hListViewImportsModules, &item);
	}

	// Resize columns
	for (unsigned int i = 0; i < GetArraySize(szModulesColumnText); i++)
		ListView_SetColumnWidth(hListViewImportsModules, i,
								i == GetArraySize(szModulesColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);

	// Insert tab items
	for(unsigned int i = 0; i < GetArraySize(szImportsTabText); i++)
	{
		TCITEM item;
		item.mask = TCIF_TEXT;
		item.pszText = (LPTSTR) szImportsTabText[i];
		TabCtrl_InsertItem(hTabsImports, i, &item);
	}

	ListView_SetItemState(hListViewImportsModules, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
}

void PropertyPageHandler_Imports::chkImportsUnmangleCPPNames_OnClick(HWND hControl, bool IsChecked)
{
	RECT ViewRect;

	// Save scrolled view rectangle
	ListView_GetViewRect(hListViewImportsAndDirTable, &ViewRect);

	ImportsPage_UpdateDisplay(false, false, true);

	// Scroll to the place where the user was earlier
	ListView_Scroll(hListViewImportsAndDirTable, abs(ViewRect.left), abs(ViewRect.top));
}

void PropertyPageHandler_Imports::lstImportsModules_OnSelection(HWND hControl, int SelectedIndex)
{
	ImportsPage_UpdateDisplay(true, false, false);
}

void PropertyPageHandler_Imports::lstImportsModules_OnColumnHeaderClick(int Index)
{
	HWND hHeader = ListView_GetHeader(hListViewImportsModules);
	HDITEM hdrItem;
	ZeroMemory(&hdrItem, sizeof(HDITEM));
	hdrItem.mask = HDI_FORMAT;
	hdrItem.fmt = HDF_LEFT | HDF_STRING;

	// Restore header's property to normal header without arrow
	for (int i = 0; i < Header_GetItemCount(hHeader); i++)
		Header_SetItem(hHeader, i, &hdrItem);

	switch (lstModulesSortOrder)
	{
	case None:
	case Descending:
		hdrItem.fmt |= HDF_SORTDOWN;
		lstModulesSortOrder = Ascending;

		break;

	case Ascending:
		hdrItem.fmt |= HDF_SORTUP;
		lstModulesSortOrder = Descending;
	}

	// Set header property of selected column with arrow
	Header_SetItem(hHeader, Index, &hdrItem);

	// Set sorting function
	CompareFuncParam Params = {hListViewImportsModules, Index, lstModulesSortOrder};
	ListView_SortItemsEx(hListViewImportsModules, StringCompareFunc, &Params);
}

void PropertyPageHandler_Imports::lstImportsAndDirTable_OnColumnHeaderClick(int Index)
{
	if (TabPage != TabImports)
		return;

	HWND hHeader = ListView_GetHeader(hListViewImportsAndDirTable);
	HDITEM hdrItem;
	ZeroMemory(&hdrItem, sizeof(HDITEM));
	hdrItem.mask = HDI_FORMAT;
	hdrItem.fmt = HDF_LEFT | HDF_STRING;

	// Restore header's property to normal header without arrow
	for (int i = 0; i < Header_GetItemCount(hHeader); i++)
		Header_SetItem(hHeader, i, &hdrItem);

	switch (lstImportsSortOrder)
	{
	case None:
	case Descending:
		hdrItem.fmt |= HDF_SORTDOWN;
		lstImportsSortOrder = Ascending;

		break;

	case Ascending:
		hdrItem.fmt |= HDF_SORTUP;
		lstImportsSortOrder = Descending;
	}

	// Set header property of selected column with arrow
	Header_SetItem(hHeader, Index, &hdrItem);

	// Set sorting function
	CompareFuncParam Params = {hListViewImportsAndDirTable, Index, lstImportsSortOrder};

	switch (Index)
	{
	case 0:
	case 1:
		ListView_SortItemsEx(hListViewImportsAndDirTable, NumberCompareFunc, &Params);

		break;

	case 2:
		ListView_SortItemsEx(hListViewImportsAndDirTable, StringCompareFunc, &Params);
	}
}

void PropertyPageHandler_Imports::lstImportsAndDirTable_OnContextMenu(LONG x, LONG y, int Index)
{
	if (TabPage != TabDirTable)
		return;

	return Generic_OnContextMenu(ImportsDirTooltipInfo, ImportsDirInfo, x, y, Index);
}

tstring PropertyPageHandler_Imports::lstImportsAndDirTable_OnGetTooltip(int Index)
{
	if (TabPage != TabDirTable)
		return _T("");

	return Generic_OnGetTooltip(ImportsDirTooltipInfo, Index);
}

void PropertyPageHandler_Imports::tabsImports_OnTabChanged(HWND hControl, int SelectedIndex)
{
	ImportsPage_UpdateDisplay(false, true, false);
}

void PropertyPageHandler_Imports::ImportsPage_UpdateDisplay(bool ListViewImportsModules_Changed, bool TabsImports_Changed, bool ChkBoxUnmangle_Changed)
{
	static LPTSTR szImportsColumnText[] = {_T("Ordinal"), _T("Hint"), _T("Name")};
	static const int TAB_IMPORTFUNCS = 0;
	static const int TAB_IMPORTDIR = 1;

	int ListViewImportsModules_SelectedIndex = ListView_GetNextItem(hListViewImportsModules, -1, LVNI_SELECTED);
	int TabsImports_SelectedIndex = TabCtrl_GetCurSel(GetDlgItem(m_hWnd, IDC_TABSIMPORTS));
	bool ChkBoxUnmangle_Checked = Button_GetCheck(hCheckBoxCPPNameUnmangle) == BST_CHECKED;
	LVITEM SelectedItem;
	DWORD Index;

	// Delete all columns and all items in ListViewImportsFuncsAndDirTable
	while (ListView_DeleteColumn(hListViewImportsAndDirTable, 0));
	ListView_DeleteAllItems(hListViewImportsAndDirTable);

	if (ListViewImportsModules_SelectedIndex < 0)
		return;

	ZeroMemory(&SelectedItem, sizeof(LVITEM));
	SelectedItem.iItem = ListViewImportsModules_SelectedIndex;
	SelectedItem.mask = LVIF_PARAM;
	SelectedItem.lParam = -1;
	ListView_GetItem(hListViewImportsModules, &SelectedItem);

	if ((Index = (DWORD) SelectedItem.lParam) < 0)
		return;

	if (ListViewImportsModules_Changed || TabsImports_Changed)
	{
		switch (TabsImports_SelectedIndex)
		{
		case TAB_IMPORTFUNCS:
			{
				TabPage = TabImports;
				lstImportsSortOrder = None;
				lstFunctions = m_PEReaderWriter.GetImportsFunctions(Index,
																	lstModules[ListViewImportsModules_SelectedIndex].Type);

				// Show 'Unmangle C++ names' checkbox
				ShowWindow(hCheckBoxCPPNameUnmangle, SW_SHOW);

				ImportsPage_UpdateDisplay(false, false, true);
			}

			break;

		case TAB_IMPORTDIR:
			{
				TabPage = TabDirTable;

				// Insert ListView columns
				LV_COLUMN column;
				ZeroMemory(&column, sizeof(LV_COLUMN));

				for (int i = 0; i < GetArraySize(GenericColumnText); i++)
				{
					column.mask = LVCF_TEXT;
					column.pszText = GenericColumnText[i];
					ListView_InsertColumn(hListViewImportsAndDirTable, i, &column);
				}

				// Fill with data
				ImportsDirInfo.clear();

				switch (lstModules[Index].Type)
				{
				case PEReadWrite::Static:
					{
						IMAGE_IMPORT_DESCRIPTOR &ImportDescriptor = *m_PEReaderWriter.GetImportDirectory(Index);			

						FillData(ImportsDirInfo, _T("Import Lookup Table RVA"), DWORD_toString(ImportDescriptor.OriginalFirstThunk, Hexadecimal),
																														ImportDescriptor.OriginalFirstThunk == 0 ?
																							_T("Shouldn't be zero, probably faulty Borland linker") : _T(""));
						FillData(ImportsDirInfo, _T("Time/Date Stamp"), DWORD_toString(ImportDescriptor.TimeDateStamp), m_PEReaderWriter.IsImportsAlreadyBound() ?
																								_T("Already bound, invalid stamp") : _T("Meaningless until bound"));
						FillData(ImportsDirInfo, _T("Forwarder Chain"), Integer_toString(ImportDescriptor.ForwarderChain));
						FillData(ImportsDirInfo, _T("Name RVA"), DWORD_toString(ImportDescriptor.Name, Hexadecimal), 
													_T("\"") + MultiByte_toString((char *) m_PEReaderWriter.GetVA(ImportDescriptor.Name)) + _T("\""));
						FillData(ImportsDirInfo, _T("Import Address Table RVA"),  DWORD_toString(ImportDescriptor.FirstThunk, Hexadecimal));
					}

					break;

				case PEReadWrite::Delayed:
					{
						ImgDelayDescr &ImportDescriptor = *m_PEReaderWriter.GetDelayImportDirectory(Index - NoOfStaticImportModules);

						FillData(ImportsDirInfo, _T("Attributes"), DWORD_toString(ImportDescriptor.grAttrs), _T("Reserved, must be zero"));
						FillData(ImportsDirInfo, _T("Name RVA"), DWORD_toString(ImportDescriptor.rvaDLLName, Hexadecimal), 
												_T("\"") + MultiByte_toString((char *) m_PEReaderWriter.GetVA(ImportDescriptor.rvaDLLName)) + _T("\""));
						FillData(ImportsDirInfo, _T("Module Handle"),
																DWORD_toString(ImportDescriptor.rvaHmod, Hexadecimal), _T("Meaningless until bound"));
						FillData(ImportsDirInfo, _T("Delay Import Address Table RVA"),
																	DWORD_toString(ImportDescriptor.rvaIAT, Hexadecimal), _T("Meaningless until bound"));
						FillData(ImportsDirInfo, _T("Name Table RVA"), DWORD_toString(ImportDescriptor.rvaINT, Hexadecimal));
						FillData(ImportsDirInfo, _T("Bound Table RVA"),
															DWORD_toString(ImportDescriptor.rvaBoundIAT, Hexadecimal), _T("Meaningless until bound"));
						FillData(ImportsDirInfo, _T("Unload Table RVA"), DWORD_toString(ImportDescriptor.rvaUnloadIAT, Hexadecimal));
						FillData(ImportsDirInfo, _T("Time Stamp"), DWORD_toString(ImportDescriptor.dwTimeStamp), _T("Meaningless until bound"));
					}
				}

				// Add items to Import Descriptor Table listview
				for(int i = 0; i < ImportsDirInfo.size(); i++)
				{
					LV_ITEM item;
					ZeroMemory(&item, sizeof(LV_ITEM));

					item.iItem = i;
					item.mask = LVIF_TEXT;
					item.pszText = (LPTSTR) ImportsDirInfo[i].szText.c_str();
					ListView_InsertItem(hListViewImportsAndDirTable, &item);

					item.iSubItem = 1;
					item.pszText = (LPTSTR) ImportsDirInfo[i].szData.c_str();
					ListView_SetItem(hListViewImportsAndDirTable, &item);

					item.iSubItem = 2;
					item.pszText = (LPTSTR) ImportsDirInfo[i].szComments.c_str();
					ListView_SetItem(hListViewImportsAndDirTable, &item);
				}

				// Resize column
				for(int i = 0; i < GetArraySize(GenericColumnText); i++)
					ListView_SetColumnWidth(hListViewImportsAndDirTable, i, i == GetArraySize(GenericColumnText) - 1 ?
																										LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);

				// Hide 'Unmangle C++ names' checkbox
				ShowWindow(hCheckBoxCPPNameUnmangle, SW_HIDE);
			}
		}
	}
	else
	{
		if (lstFunctions.size() == 0)
			return;

		// Insert ListView columns
		LV_COLUMN column;
		ZeroMemory(&column, sizeof(LV_COLUMN));

		for (int i = 0; i < GetArraySize(szImportsColumnText); i++)
		{
			column.mask = LVCF_TEXT;
			column.pszText = szImportsColumnText[i];
			ListView_InsertColumn(hListViewImportsAndDirTable, i, &column);
		}

		// Add items to Import Functions listview
		LV_ITEM item;
		ZeroMemory(&item, sizeof(LV_ITEM));

		for (int i = 0; i < lstFunctions.size(); i++)
		{
			tstring Buffer;

			// Ordinal
			item.iItem = i;
			item.iSubItem = 0;
			item.mask = LVIF_TEXT;
			Buffer = lstFunctions[i].Type == PEReadWrite::ByOrdinal ? DWORD_toString(lstFunctions[i].Ordinal) : _T("");
			item.pszText = (LPTSTR) Buffer.c_str();
			ListView_InsertItem(hListViewImportsAndDirTable, &item);

			// Hint
			item.iSubItem = 1;
			Buffer = lstFunctions[i].Type == PEReadWrite::ByName ? DWORD_toString(lstFunctions[i].Hint) : _T("");
			item.pszText = (LPTSTR) Buffer.c_str();
			ListView_SetItem(hListViewImportsAndDirTable, &item);

			// Name
			item.iSubItem = 2;
			Buffer = MultiByte_toString(lstFunctions[i].Type == PEReadWrite::ByName ? (ChkBoxUnmangle_Checked ?
											PEReadWrite::UnmangleCPPNames(lstFunctions[i].Name, PEReadWrite::Full).c_str() :
											lstFunctions[i].Name.c_str()) : "n/a");

			item.pszText = (LPTSTR) Buffer.c_str();
			ListView_SetItem(hListViewImportsAndDirTable, &item);
		}

		// Resize column
		for (int i = 0; i < GetArraySize(szImportsColumnText); i++)
			ListView_SetColumnWidth(hListViewImportsAndDirTable, i, LVSCW_AUTOSIZE_USEHEADER);
	}
}