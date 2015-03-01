#include "stdafx.h"
#include "PropertyPageHandler.h"


PropertyPageHandler_Imports::PropertyPageHandler_Imports(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, std::ref(PEReaderWriter)),
	m_TabPage(TabPages::ImportsEntry),
	m_lstModulesSortOrder(SortOrder::None),
	m_lstImportsSortOrder(SortOrder::None)
{
	m_hListViewImportModules = GetDlgItem(m_hWnd, IDC_LISTIMPORTSMODULES);
	m_hTabsImports = GetDlgItem(m_hWnd, IDC_TABSIMPORTS);
	m_hListViewImportsAndDirTable = GetDlgItem(m_hWnd, IDC_LISTIMPORTSFUNCSANDDIRTABLE);
	m_hCheckBoxCPPNameUnmangle = GetDlgItem(m_hWnd, IDC_CHKCPPNAMEUNMANGLEIMPORTS);

	// Setup controls with layout manager
	m_LayoutManager.AddChildConstraint(IDC_TABSIMPORTS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_LayoutManager.AddChildConstraint(IDC_LISTIMPORTSFUNCSANDDIRTABLE, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_LayoutManager.AddChildConstraint(IDC_CHKCPPNAMEUNMANGLEIMPORTS, CWA_DEFAULT, CWA_BOTTOM);
	// Make ZOrder: Listview (On top) and Tab Control (On bottom)
	SetWindowPos(m_hTabsImports, m_hListViewImportsAndDirTable, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(m_hTabsImports, m_hCheckBoxCPPNameUnmangle, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	// Set full row selection style for listviews
	ListView_SetExtendedListViewStyleEx(m_hListViewImportModules, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	ListView_SetExtendedListViewStyleEx(m_hListViewImportsAndDirTable,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);
}

void PropertyPageHandler_Imports::OnInitDialog()
{
	// Fill controls with data
	static LPWSTR szModulesColumnText[] = { L"Name", L"Link Type" };
	static LPWSTR szModuleImportTypes[] = { L"Static", L"Delayed" };

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(column));

	for (size_t i = 0; i < ARRAYSIZE(szModulesColumnText); ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szModulesColumnText[i];
		ListView_InsertColumn(m_hListViewImportModules, i, &column);
	}

	// Insert ListView items for Modules list view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(item));

	// Add static imports first
	int err;
	if (m_PEReaderWriter.hasStaticImports())
	{
		const size_t NoOfStaticImportModules = m_PEReaderWriter.getNoOfStaticImportModules();

		for (size_t i = 0; i < NoOfStaticImportModules; ++i)
		{
			PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
			err = m_PEReaderWriter.getStaticModuleDescriptor(int(i), std::ref(pImportDesc));
			if (err)
			{
				LogError(L"Static import descriptor for index " + DWORD_toString(DWORD(i)) + L" is corrupted. File is not valid.", true);
				break;
			}

			item.iItem = int(i);
			item.iSubItem = 0;
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.pszText = LPWSTR(_wcsdup(m_PEReaderWriter.getStaticModuleName(std::cref(pImportDesc)).c_str()));
			m_listImportTypeAndDesc.push_back({ PEReadWrite::ImportType::Static, pImportDesc });
			item.lParam = LPARAM(m_listImportTypeAndDesc.size() - 1);
			ListView_InsertItem(m_hListViewImportModules, &item);
			free(item.pszText);

			item.iSubItem = 1;
			item.mask = LVIF_TEXT;
			item.pszText = LPWSTR(szModuleImportTypes[0]);
			item.lParam = NULL;
			ListView_SetItem(m_hListViewImportModules, &item);
		}
	}

	// Add delayed imports now, if any
	if (m_PEReaderWriter.hasDelayedImports())
	{
		const int NextItemIndex = item.iItem + 1;
		const size_t NoOfDelayedImportModules = m_PEReaderWriter.getNoOfDelayedImportModules();

		for (size_t i = 0; i < NoOfDelayedImportModules; ++i)
		{
			PImgDelayDescr pImportDesc;
			m_PEReaderWriter.getDelayedModuleDescriptor(int(i), std::ref(pImportDesc));

			item.iItem = NextItemIndex + int(i);
			item.iSubItem = 0;
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.pszText = LPWSTR(_wcsdup(m_PEReaderWriter.getDelayedModuleName(std::cref(pImportDesc)).c_str()));
			m_listImportTypeAndDesc.push_back({ PEReadWrite::ImportType::Delayed, pImportDesc });
			item.lParam = LPARAM(m_listImportTypeAndDesc.size() - 1);
			ListView_InsertItem(m_hListViewImportModules, &item);
			free(item.pszText);

			item.iSubItem = 1;
			item.mask = LVIF_TEXT;
			item.pszText = LPWSTR(szModuleImportTypes[1]);
			item.lParam = NULL;
			ListView_SetItem(m_hListViewImportModules, &item);
		}
	}

	// Resize columns
	for (size_t i = 0; i < ARRAYSIZE(szModulesColumnText); ++i)
		ListView_SetColumnWidth(m_hListViewImportModules,
								i,
								i == ARRAYSIZE(szModulesColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);

	static LPWSTR szImportsTabText[] = { L"Imports", L"Import Directory Table" };

	// Insert tab items
	for (size_t i = 0; i < ARRAYSIZE(szImportsTabText); ++i)
	{
		TCITEM item;
		item.mask = TCIF_TEXT;
		item.pszText = LPWSTR(szImportsTabText[i]);
		TabCtrl_InsertItem(m_hTabsImports, i, &item);
	}

	ListView_SetItemState(m_hListViewImportModules, 0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

	// FIXME: File offset is not computed correctly here
	RTTI::GetTooltipInfo(m_ImportsDirTooltipInfo, 0, RTTI::RTTI_IMPORT_DIRECTORY);	// Get tooltip data
}

void PropertyPageHandler_Imports::chkImportsUnmangleCPPNames_OnClick(HWND hControl, bool IsChecked)
{
	RECT ViewRect;

	// Save scrolled view rectangle
	ListView_GetViewRect(m_hListViewImportsAndDirTable, &ViewRect);

	ImportsPage_UpdateDisplay();

	// Scroll to the place where the user was earlier
	ListView_Scroll(m_hListViewImportsAndDirTable, abs(ViewRect.left), abs(ViewRect.top));
}

void PropertyPageHandler_Imports::lstImportsModules_OnSelection(HWND hControl, int SelectedIndex)
{
	ImportsPage_UpdateDisplay();
}

void PropertyPageHandler_Imports::lstImportsModules_OnColumnHeaderClick(int Index)
{
	HWND hHeader = ListView_GetHeader(m_hListViewImportModules);
	HDITEM hdrItem;
	ZeroMemory(&hdrItem, sizeof(hdrItem));
	hdrItem.mask = HDI_FORMAT;
	hdrItem.fmt = HDF_LEFT | HDF_STRING;

	// Restore header's property to normal header without arrow
	for (int i = 0; i < Header_GetItemCount(hHeader); ++i)
		Header_SetItem(hHeader, i, &hdrItem);

	switch (m_lstModulesSortOrder)
	{
		case SortOrder::None:
		case SortOrder::Descending:
			hdrItem.fmt |= HDF_SORTDOWN;
			m_lstModulesSortOrder = SortOrder::Ascending;
			break;

		case SortOrder::Ascending:
			hdrItem.fmt |= HDF_SORTUP;
			m_lstModulesSortOrder = SortOrder::Descending;
			break;
	}

	// Set header property of selected column with arrow
	Header_SetItem(hHeader, Index, &hdrItem);

	// Set sorting function
	CompareFuncParam Params = {m_hListViewImportModules, Index, m_lstModulesSortOrder};
	ListView_SortItemsEx(m_hListViewImportModules, funcStringSort, &Params);
}

void PropertyPageHandler_Imports::lstImportsAndDirTable_OnColumnHeaderClick(int Index)
{
	if (m_TabPage != TabPages::ImportsEntry)
		return;

	HWND hHeader = ListView_GetHeader(m_hListViewImportsAndDirTable);
	HDITEM hdrItem;
	ZeroMemory(&hdrItem, sizeof(hdrItem));

	hdrItem.mask = HDI_FORMAT;
	hdrItem.fmt = HDF_LEFT | HDF_STRING;

	// Restore header's property to normal header without arrow
	for (int i = 0; i < Header_GetItemCount(hHeader); ++i)
		Header_SetItem(hHeader, i, &hdrItem);

	switch (m_lstImportsSortOrder)
	{
		case SortOrder::None:
		case SortOrder::Descending:
			hdrItem.fmt |= HDF_SORTDOWN;
			m_lstImportsSortOrder = SortOrder::Ascending;
			break;

		case SortOrder::Ascending:
			hdrItem.fmt |= HDF_SORTUP;
			m_lstImportsSortOrder = SortOrder::Descending;
			break;
	}

	// Set header property of selected column with arrow
	Header_SetItem(hHeader, Index, &hdrItem);

	// Set sorting function
	CompareFuncParam Params = {m_hListViewImportsAndDirTable, Index, m_lstImportsSortOrder};

	switch (Index)
	{
	case 0:
	case 1:
		ListView_SortItemsEx(m_hListViewImportsAndDirTable, funcNumberSort, &Params);
		break;

	case 2:
		ListView_SortItemsEx(m_hListViewImportsAndDirTable, funcStringSort, &Params);
		break;
	}
}

void PropertyPageHandler_Imports::lstImportsAndDirTable_OnContextMenu(LONG x, LONG y, int Index)
{
	if (m_TabPage != TabPages::ImportDirTable)
		return;

	Generic_OnContextMenu(m_ImportsDirTooltipInfo, m_ImportsDirInfo, x, y, Index);
}

wstring PropertyPageHandler_Imports::lstImportsAndDirTable_OnGetTooltip(int Index)
{
	if (m_TabPage != TabPages::ImportDirTable)
		return L"";

	return Generic_OnGetTooltip(m_ImportsDirTooltipInfo, Index);
}

void PropertyPageHandler_Imports::tabsImports_OnTabChanged(HWND hControl, int SelectedIndex)
{
	switch (SelectedIndex)
	{
		case 0: m_TabPage = TabPages::ImportsEntry; break;
		case 1: m_TabPage = TabPages::ImportDirTable; break;
	}

	ImportsPage_UpdateDisplay();
}

void PropertyPageHandler_Imports::ImportsPage_UpdateDisplay()
{
	int lstImportModules_SelectedIndex = ListView_GetNextItem(m_hListViewImportModules, -1, LVNI_SELECTED);
	bool chkBoxUnmangle_Checked = (Button_GetCheck(m_hCheckBoxCPPNameUnmangle) == BST_CHECKED);

	// Delete all columns and all items in ListViewImportsFuncsAndDirTable
	ListView_Reset(m_hListViewImportsAndDirTable);

	// If no module is selected, do nothing
	if (lstImportModules_SelectedIndex < 0)
		return;

	// Get the 'LPARAM' of the selected item. It contains pointer to import directory structure
	size_t ImportTypeAndDescIndex; // Index pointing to item in 'm_listImportTypeAndDesc'
	LVITEM SelectedItem;

	ZeroMemory(&SelectedItem, sizeof(SelectedItem));
	SelectedItem.iItem = lstImportModules_SelectedIndex;
	SelectedItem.mask = LVIF_PARAM;
	SelectedItem.lParam = LPARAM(-1);
	ListView_GetItem(m_hListViewImportModules, &SelectedItem);

	if (SelectedItem.lParam == LPARAM(-1))
		return;

	ImportTypeAndDescIndex = size_t(SelectedItem.lParam);
	int err;

	switch (m_TabPage)
	{
		case TabPages::ImportsEntry:
		{
			m_lstImportsSortOrder = SortOrder::None;

			// Show 'Unmangle C++ names' checkbox
			ShowWindow(m_hCheckBoxCPPNameUnmangle, SW_SHOW);

			static LPWSTR szImportsColumnText[] = { L"Ordinal", L"Hint", L"Name" };

			// Insert ListView columns
			LV_COLUMN column;
			ZeroMemory(&column, sizeof(column));

			for (size_t i = 0; i < ARRAYSIZE(szImportsColumnText); ++i)
			{
				column.mask = LVCF_TEXT;
				column.pszText = szImportsColumnText[i];
				ListView_InsertColumn(m_hListViewImportsAndDirTable, i, &column);
			}

			// Add items to Import Functions listview
			LV_ITEM item;
			ZeroMemory(&item, sizeof(item));

			ImportTypeAndDesc SelectedImportTypeAndDesc = m_listImportTypeAndDesc[ImportTypeAndDescIndex];

			switch (SelectedImportTypeAndDesc.ImportType)
			{
				case PEReadWrite::ImportType::Static:
				{
					PIMAGE_IMPORT_DESCRIPTOR pImportDesc = PIMAGE_IMPORT_DESCRIPTOR(SelectedImportTypeAndDesc.pImportDesc);
					const int NoOfStaticModuleImports = m_PEReaderWriter.getNoOfStaticModuleImports(std::cref(pImportDesc));

					for (int i = 0; i < NoOfStaticModuleImports; ++i)
					{
						WORD Ordinal;
						WORD Hint;
						string Name;
						bool hasOrdinal;

						switch (m_PEReaderWriter.getPEType())
						{
							case PEReadWrite::PEType::PE32:
							{
								PIMAGE_THUNK_DATA32 pThunkData;
								err = m_PEReaderWriter.getStaticModuleImport(std::cref(pImportDesc),
																			 i,
																			 std::ref(pThunkData));
								if (err)
								{
									LogError(L"ERROR: Couldn't read import at index " + DWORD_toString(i) + L". File is not valid.", true);
									break;
								}

								err = m_PEReaderWriter.getModuleImportOrdinalOrName(std::cref(pThunkData),
																					std::ref(Ordinal),
																					std::ref(Hint),
																					std::ref(Name),
																					std::ref(hasOrdinal));
								if (err)
								{
									LogError(L"ERROR: Couldn't read import ordinal/name at index " + DWORD_toString(i) + L". File is not valid.", true);
									break;
								}
							}
							break;

							case PEReadWrite::PEType::PE64:
							{
								PIMAGE_THUNK_DATA64 pThunkData;
								err = m_PEReaderWriter.getStaticModuleImport(std::cref(pImportDesc),
																			 i,
																			 std::ref(pThunkData));
								if (err)
								{
									LogError(L"ERROR: Couldn't read import at index " + DWORD_toString(i) + L". File is not valid.", true);
									break;
								}

								err = m_PEReaderWriter.getModuleImportOrdinalOrName(std::cref(pThunkData),
																					std::ref(Ordinal),
																					std::ref(Hint),
																					std::ref(Name),
																					std::ref(hasOrdinal));
								if (err)
								{
									LogError(L"ERROR: Couldn't read import ordinal/name at index " + DWORD_toString(i) + L". File is not valid.", true);
									break;
								}
							}
							break;
						}

						// Ordinal
						item.iItem = i;
						item.iSubItem = 0;
						item.mask = LVIF_TEXT;
						item.pszText = LPWSTR(_wcsdup((hasOrdinal ? DWORD_toString(Ordinal).c_str() : L"n/a")));
						ListView_InsertItem(m_hListViewImportsAndDirTable, &item);
						free(item.pszText);

						// Hint
						item.iSubItem = 1;
						item.pszText = LPWSTR(_wcsdup((!hasOrdinal ? DWORD_toString(Hint).c_str() : L"n/a")));
						ListView_SetItem(m_hListViewImportsAndDirTable, &item);
						free(item.pszText);

						// Name
						item.iSubItem = 2;
						item.pszText = LPWSTR(_wcsdup(!hasOrdinal ? MultiByte_toString(chkBoxUnmangle_Checked ? PEReadWrite::unmangleCPPNames(Name, PEReadWrite::SymbolPart::Full) : Name).c_str() : L"n/a"));
						ListView_SetItem(m_hListViewImportsAndDirTable, &item);
						free(item.pszText);
					}
				}
				break;

				case PEReadWrite::ImportType::Delayed:
				{
					PImgDelayDescr pImportDesc = PImgDelayDescr(SelectedImportTypeAndDesc.pImportDesc);
					const int NoOfDelayedModuleImports = m_PEReaderWriter.getNoOfDelayedModuleImports(std::cref(pImportDesc));

					for (int i = 0; i < NoOfDelayedModuleImports; ++i)
					{
						WORD Ordinal;
						WORD Hint;
						string Name;
						bool hasOrdinal;

						switch (m_PEReaderWriter.getPEType())
						{
							case PEReadWrite::PEType::PE32:
							{
								PIMAGE_THUNK_DATA32 pThunkData;
								err = m_PEReaderWriter.getDelayedModuleImport(std::cref(pImportDesc),
																			  i,
																			  std::ref(pThunkData));
								if (err)
								{
									LogError(L"ERROR: Couldn't read import at index " + DWORD_toString(i) + L". File is not valid.", true);
									break;
								}

								err = m_PEReaderWriter.getModuleImportOrdinalOrName(std::cref(pThunkData),
																					std::ref(Ordinal),
																					std::ref(Hint),
																					std::ref(Name),
																					std::ref(hasOrdinal));
								if (err)
								{
									LogError(L"ERROR: Couldn't read import ordinal/name at index " + DWORD_toString(i) + L". File is not valid.", true);
									break;
								}
							}
							break;

							case PEReadWrite::PEType::PE64:
							{
								PIMAGE_THUNK_DATA64 pThunkData;
								err = m_PEReaderWriter.getDelayedModuleImport(std::cref(pImportDesc),
																			  i,
																			  std::ref(pThunkData));
								if (err)
								{
									LogError(L"ERROR: Couldn't read import at index " + DWORD_toString(i) + L". File is not valid.", true);
									break;
								}

								err = m_PEReaderWriter.getModuleImportOrdinalOrName(std::cref(pThunkData),
																					std::ref(Ordinal),
																					std::ref(Hint),
																					std::ref(Name),
																					std::ref(hasOrdinal));
								if (err)
								{
									LogError(L"ERROR: Couldn't read import ordinal/name at index " + DWORD_toString(i) + L". File is not valid.", true);
									break;
								}
							}
							break;
						}

						// Ordinal
						item.iItem = i;
						item.iSubItem = 0;
						item.mask = LVIF_TEXT;
						item.pszText = LPWSTR(_wcsdup((hasOrdinal ? DWORD_toString(Ordinal).c_str() : L"n/a")));
						ListView_InsertItem(m_hListViewImportsAndDirTable, &item);
						free(item.pszText);

						// Hint
						item.iSubItem = 1;
						item.pszText = LPWSTR(_wcsdup((!hasOrdinal ? DWORD_toString(Hint).c_str() : L"n/a")));
						ListView_SetItem(m_hListViewImportsAndDirTable, &item);
						free(item.pszText);

						// Name
						item.iSubItem = 2;
						item.pszText = LPWSTR(_wcsdup(!hasOrdinal ? MultiByte_toString(chkBoxUnmangle_Checked ? PEReadWrite::unmangleCPPNames(Name, PEReadWrite::SymbolPart::Full) : Name).c_str() : L"n/a"));
						ListView_SetItem(m_hListViewImportsAndDirTable, &item);
						free(item.pszText);
					}
				}
				break;
			}

			// Resize column
			for (int i = 0; i < ARRAYSIZE(szImportsColumnText); i++)
				ListView_SetColumnWidth(m_hListViewImportsAndDirTable, i, LVSCW_AUTOSIZE_USEHEADER);
		}
		break;

		case TabPages::ImportDirTable:
		{
			// Hide 'Unmangle C++ names' checkbox
			ShowWindow(m_hCheckBoxCPPNameUnmangle, SW_HIDE);

			// Insert ListView columns
			LV_COLUMN column;
			ZeroMemory(&column, sizeof(column));

			for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
			{
				column.mask = LVCF_TEXT;
				column.pszText = szGenericColumnText[i];
				ListView_InsertColumn(m_hListViewImportsAndDirTable, int(i), &column);
			}

			// Fill with data
			ImportTypeAndDesc SelectedImportTypeAndDesc = m_listImportTypeAndDesc[ImportTypeAndDescIndex];

			switch (SelectedImportTypeAndDesc.ImportType)
			{
				case PEReadWrite::ImportType::Static:
				{
					PIMAGE_IMPORT_DESCRIPTOR pImportDesc = PIMAGE_IMPORT_DESCRIPTOR(SelectedImportTypeAndDesc.pImportDesc);

					m_ImportsDirInfo =
					{
						{ L"Import Lookup Table RVA", DWORD_toString(pImportDesc->OriginalFirstThunk, Hexadecimal),
						  pImportDesc->OriginalFirstThunk == 0 ? L"Shouldn't be zero, probably faulty Borland linker" : L"" },
						{ L"Time/Date Stamp", DWORD_toString(pImportDesc->TimeDateStamp), m_PEReaderWriter.isImportsAlreadyBound() ?
						  L"Already bound, invalid stamp" : L"Meaningless until bound" },
						{ L"Forwarder Chain", Integer_toString(pImportDesc->ForwarderChain) },
						{ L"Name RVA", DWORD_toString(pImportDesc->Name, Hexadecimal),
						  L'\"' + m_PEReaderWriter.getStaticModuleName(std::cref(pImportDesc)) + L'\"' },
						{ L"Import Address Table RVA", DWORD_toString(pImportDesc->FirstThunk, Hexadecimal) }
					};
				}
				break;

				case PEReadWrite::ImportType::Delayed:
				{
					PImgDelayDescr pImportDesc = PImgDelayDescr(SelectedImportTypeAndDesc.pImportDesc);

					m_ImportsDirInfo =
					{
						{ L"Attributes", DWORD_toString(pImportDesc->grAttrs), L"Reserved, must be zero" },
						{ L"Name RVA", DWORD_toString(pImportDesc->rvaDLLName, Hexadecimal),
						  L'\"' + m_PEReaderWriter.getDelayedModuleName(std::cref(pImportDesc)) + L'\"' },
						{ L"Module Handle",
						  DWORD_toString(pImportDesc->rvaHmod, Hexadecimal), L"Meaningless until bound" },
						{ L"Delay Address Table RVA",
						  DWORD_toString(pImportDesc->rvaIAT, Hexadecimal), L"Meaningless until bound" },
						{ L"Name Table RVA", DWORD_toString(pImportDesc->rvaINT, Hexadecimal) },
						{ L"Bound Table RVA",
						  DWORD_toString(pImportDesc->rvaBoundIAT, Hexadecimal), L"Meaningless until bound" },
						{ L"Unload Table RVA", DWORD_toString(pImportDesc->rvaUnloadIAT, Hexadecimal) },
						{ L"Time Stamp", DWORD_toString(pImportDesc->dwTimeStamp), L"Meaningless until bound" }
					};
				}
				break;
			}

			// Add items to Import Descriptor Table listview
			for (size_t i = 0; i < m_ImportsDirInfo.size(); ++i)
			{
				LV_ITEM item;
				ZeroMemory(&item, sizeof(LV_ITEM));

				item.iItem = int(i);
				item.mask = LVIF_TEXT;
				item.pszText = LPWSTR(_wcsdup(m_ImportsDirInfo[i].Text.c_str()));
				ListView_InsertItem(m_hListViewImportsAndDirTable, &item);
				free(item.pszText);

				item.iSubItem = 1;
				item.pszText = LPWSTR(_wcsdup(m_ImportsDirInfo[i].Data.c_str()));
				ListView_SetItem(m_hListViewImportsAndDirTable, &item);
				free(item.pszText);

				item.iSubItem = 2;
				item.pszText = LPWSTR(_wcsdup(m_ImportsDirInfo[i].Comments.c_str()));
				ListView_SetItem(m_hListViewImportsAndDirTable, &item);
				free(item.pszText);
			}

			// Resize column
			for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
				ListView_SetColumnWidth(m_hListViewImportsAndDirTable,
				                        i,
										(i == ARRAYSIZE(szGenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));
		}
		break;
	}
}