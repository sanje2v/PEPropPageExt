#include "stdafx.h"
#include "PropertyPageHandler.h"


PropertyPageHandler_Exports::PropertyPageHandler_Exports(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, std::ref(PEReaderWriter)),
	  m_ExportsSortOrder(SortOrder::None)
{
	m_hListViewExportDir = GetDlgItem(m_hWnd, IDC_LISTEXPORTDIRECTORYTABLE);
	m_hListViewExports = GetDlgItem(m_hWnd, IDC_LISTEXPORTS);
	m_hCheckBoxCPPNameUnmangle = GetDlgItem(m_hWnd, IDC_CHKCPPNAMEUNMANGLEEXPORTS);

	// Setup controls with layout manager
	m_LayoutManager.AddChildConstraint(IDC_LISTEXPORTDIRECTORYTABLE, CWA_LEFTRIGHT, CWA_TOP);
	m_LayoutManager.AddChildConstraint(IDC_LISTEXPORTS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_LayoutManager.AddChildConstraint(IDC_CHKCPPNAMEUNMANGLEEXPORTS, CWA_DEFAULT, CWA_BOTTOM);

	// Set full row selection style for listviews
	ListView_SetExtendedListViewStyleEx(m_hListViewExportDir,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);
	ListView_SetExtendedListViewStyleEx(m_hListViewExports, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
}

void PropertyPageHandler_Exports::OnInitDialog()
{
	// Fill them with data
	PIMAGE_EXPORT_DIRECTORY pExportDir;
	int err = m_PEReaderWriter.getExportDirectory(std::ref(pExportDir));
	if (err)
	{
		LogError(L"ERROR: Export directory is incomplete. File is not valid.", true);
		return;
	}

	// Prepare Tooltip for Export Directory listview
	RTTI::GetTooltipInfo(m_ExportDirTooltipInfo, UINT_PTR(NULL), RTTI::RTTI_EXPORT_DIRECTORY);

	// Data for Export Directory Table
	wstring ExportName;
	err = m_PEReaderWriter.getExportImageName(std::cref(pExportDir), std::ref(ExportName));
	if (err)
	{
		LogError(L"ERROR: Export name is incomplete. File is not valid.", true);
		return;
	}

	m_ExportDirInfo =
	{
		{ L"Export Flags", DWORD_toString(pExportDir->Characteristics, Hexadecimal), L"Reserved, must be zero" },
		{ L"Time/Date Stamp", DWORD_toString(pExportDir->TimeDateStamp), TimeDateStamp_toString(pExportDir->TimeDateStamp) },
		{ L"Version", VersionNums_toString(pExportDir->MajorVersion, pExportDir->MinorVersion) },
		{
			L"Name RVA",
			DWORD_toString(pExportDir->Name, Hexadecimal),
			L'\"' + ExportName + L'\"'
		},
		{ L"Ordinal Base", DWORD_toString(pExportDir->Base) },
		{ L"Addr Table Entries", DWORD_toString(pExportDir->NumberOfFunctions) },
		{ L"No. of Name Ptrs", DWORD_toString(pExportDir->NumberOfNames) },
		{ L"Export Addr Table RVA", DWORD_toString(pExportDir->AddressOfFunctions, Hexadecimal) },
		{ L"Name Pointer RVA", DWORD_toString(pExportDir->AddressOfNames, Hexadecimal) },
		{ L"Ordinal Table RVA", DWORD_toString(pExportDir->AddressOfNameOrdinals, Hexadecimal) }
	};

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szGenericColumnText[i];
		ListView_InsertColumn(m_hListViewExportDir, i, &column);
	}

	// Insert ListView items for Export Directory view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (size_t i = 0; i < m_ExportDirInfo.size(); ++i)
	{
		item.iItem = int(i);
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = LPWSTR(_wcsdup(m_ExportDirInfo[i].Text.c_str()));
		ListView_InsertItem(m_hListViewExportDir, &item);
		free(item.pszText);

		item.iSubItem = 1;
		item.pszText = LPWSTR(_wcsdup(m_ExportDirInfo[i].Data.c_str()));
		ListView_SetItem(m_hListViewExportDir, &item);
		free(item.pszText);

		item.iSubItem = 2;
		item.pszText = LPWSTR(_wcsdup(m_ExportDirInfo[i].Comments.c_str()));
		ListView_SetItem(m_hListViewExportDir, &item);
		free(item.pszText);
	}

	// Resize columns
	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
		ListView_SetColumnWidth(m_hListViewExportDir,
								i,
								(i == ARRAYSIZE(szGenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));

	ExportsPage_UpdateDisplay(Button_GetCheck(m_hCheckBoxCPPNameUnmangle) == BST_CHECKED);
}

wstring PropertyPageHandler_Exports::lstExportDir_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(m_ExportDirTooltipInfo, Index);
}

void PropertyPageHandler_Exports::lstExportDir_OnContextMenu(LONG x, LONG y, int Index)
{
	return Generic_OnContextMenu(m_ExportDirTooltipInfo, m_ExportDirInfo, x, y, Index);
}

void PropertyPageHandler_Exports::chkExportsUnmangleCPPNames_OnClick(HWND hControl, bool IsChecked)
{
	RECT ViewRect;

	// Save scrolled view rectangle
	ListView_GetViewRect(m_hListViewExports, &ViewRect);

	ExportsPage_UpdateDisplay(IsChecked);

	// Scroll to the place where the user was earlier
	ListView_Scroll(m_hListViewExports, abs(ViewRect.left), abs(ViewRect.top));
}

void PropertyPageHandler_Exports::lstExports_OnColumnHeaderClick(int Index)
{
	HWND hHeader = ListView_GetHeader(m_hListViewExports);
	HDITEM hdrItem;
	ZeroMemory(&hdrItem, sizeof(HDITEM));
	hdrItem.mask = HDI_FORMAT;
	hdrItem.fmt = HDF_LEFT | HDF_STRING;

	// Restore header's property to normal header without arrow
	int HeaderItemsCount = Header_GetItemCount(hHeader);
	for (int i = 0; i < HeaderItemsCount; ++i)
		Header_SetItem(hHeader, i, &hdrItem);

	switch (m_ExportsSortOrder)
	{
		case SortOrder::None:
		case SortOrder::Descending:
			hdrItem.fmt |= HDF_SORTDOWN;
			m_ExportsSortOrder = SortOrder::Ascending;
			break;
		
		case SortOrder::Ascending:
			hdrItem.fmt |= HDF_SORTUP;
			m_ExportsSortOrder = SortOrder::Descending;
			break;
	}

	// Set header property of selected column with arrow
	Header_SetItem(hHeader, Index, &hdrItem);

	// Set sorting function
	CompareFuncParam Params = { m_hListViewExports, Index, m_ExportsSortOrder };

	switch (Index)
	{
		case 0:
			ListView_SortItemsEx(m_hListViewExports, funcHexNumberSort, &Params);
			break;
		
		case 1:
		case 3:
			ListView_SortItemsEx(m_hListViewExports, funcStringSort, &Params);
			break;

		case 2:
			ListView_SortItemsEx(m_hListViewExports, funcNumberSort, &Params);
			break;
	}
}

void PropertyPageHandler_Exports::ExportsPage_UpdateDisplay(bool ChkBoxUnmangle_Checked)
{
	PIMAGE_EXPORT_DIRECTORY pExportDir;
	int err = m_PEReaderWriter.getExportDirectory(std::ref(pExportDir));
	if (err)
	{
		LogError(L"ERROR: Export directory is incomplete. File is not valid.", true);
		return;
	}

	LPWORD pOrdinalsTable;
	err = m_PEReaderWriter.getExportOrdinalsTable(std::cref(pExportDir), std::ref(pOrdinalsTable));
	if (err)
	{
		LogError(L"ERROR: Export ordinals table is incomplete. File is not valid.", true);
		return;
	}

	LPDWORD pNamesTable;
	err = m_PEReaderWriter.getExportNamesTable(std::cref(pExportDir), std::ref(pNamesTable));
	if (err)
	{
		LogError(L"ERROR: Export names table is incomplete. File is not valid.", true);
		return;
	}

	LPDWORD pRVAs;
	err = m_PEReaderWriter.getExportRVAs(std::cref(pExportDir), std::ref(pRVAs));
	if (err)
	{
		LogError(L"ERROR: Export RVA table is incomplete. File is not valid.", true);
		return;
	}

	vector<RVANameOrdinalForwarder> ExportsInfo;

	for (DWORD i = 0; i < pExportDir->NumberOfFunctions; ++i)
	{
		bool hasBothNameAndOrdinal = false;

		for (DWORD j = 0; j < pExportDir->NumberOfNames; ++j)
		{
			if (WORD(i) == m_PEReaderWriter.getExportOrdinal(std::cref(pOrdinalsTable), j))
			{
				string ExportName = m_PEReaderWriter.getExportName(std::cref(pNamesTable), j);
				DWORD ExportOrdinal = m_PEReaderWriter.getExportOrdinal(pOrdinalsTable, j) + pExportDir->Base;

				hasBothNameAndOrdinal = true;	// Address has a name and ordinal

				ExportsInfo.insert(ExportsInfo.cend(),
				{
					DWORD_toString(pRVAs[i], Hexadecimal),
					MultiByte_toString(ChkBoxUnmangle_Checked ?
					 PEReadWrite::unmangleCPPNames(ExportName, PEReadWrite::SymbolPart::Full) : ExportName),
					DWORD_toString(ExportOrdinal),
					(m_PEReaderWriter.isExportForwarderName(pRVAs[i]) ?
					 L'\"' + m_PEReaderWriter.getExportForwarderName(pRVAs[i]) + L'\"' : L"")
				});
			}
		}

		if (!hasBothNameAndOrdinal)		// Address only has a ordinal
			ExportsInfo.insert(ExportsInfo.cend(),
			{
				DWORD_toString(pRVAs[i], Hexadecimal),
				L"n/a",
				DWORD_toString(i + pExportDir->Base),
				(m_PEReaderWriter.isExportForwarderName(pRVAs[i]) ?
				    L'\"' + m_PEReaderWriter.getExportForwarderName(pRVAs[i]) + L'\"' : L"")
			});
	}

	// Delete all columns and all items in ListViewImportsFuncsAndDirTable
	ListView_Reset(m_hListViewExports);

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));
	
	static LPWSTR szExportsColumnText[] = { L"Function", L"Name", L"Ordinal", L"Forwarder" };
	for (size_t i = 0; i < ARRAYSIZE(szExportsColumnText); ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szExportsColumnText[i];
		ListView_InsertColumn(m_hListViewExports, i, &column);
	}

	//Insert ListView items for Exports
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (size_t i = 0; i < ExportsInfo.size(); ++i)
	{
		item.iItem = int(i);
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = LPWSTR(_wcsdup(ExportsInfo[i].RVA.c_str()));
		ListView_InsertItem(m_hListViewExports, &item);
		free(item.pszText);

		item.iSubItem = 1;
		item.pszText = LPWSTR(_wcsdup(ExportsInfo[i].Name.c_str()));
		ListView_SetItem(m_hListViewExports, &item);
		free(item.pszText);

		item.iSubItem = 2;
		item.pszText = LPWSTR(_wcsdup(ExportsInfo[i].Ordinal.c_str()));
		ListView_SetItem(m_hListViewExports, &item);
		free(item.pszText);

		item.iSubItem = 3;
		item.pszText = LPWSTR(_wcsdup(ExportsInfo[i].Forwarder.c_str()));
		ListView_SetItem(m_hListViewExports, &item);
		free(item.pszText);
	}

	// Resize columns
	for (size_t i = 0; i < ARRAYSIZE(szExportsColumnText); ++i)
		ListView_SetColumnWidth(m_hListViewExports, i, LVSCW_AUTOSIZE_USEHEADER);
}