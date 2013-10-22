#include "PropertyPageHandler.h"


#define FillData2(vectorobj, ...)		(vectorobj).push_back(RVANameOrdinalForwarder(__VA_ARGS__))


void PropertyPageHandler_Exports::OnInitDialog()
{
	hListViewExportDir = GetDlgItem(m_hWnd, IDC_LISTEXPORTDIRECTORYTABLE);
	hListViewExports = GetDlgItem(m_hWnd, IDC_LISTEXPORTS);
	hCheckBoxCPPNameUnmangle = GetDlgItem(m_hWnd, IDC_CHKCPPNAMEUNMANGLEEXPORTS);

	// Setup controls with layout manager
	m_pLayoutManager->AddChildConstraint(IDC_LISTEXPORTDIRECTORYTABLE, CWA_LEFTRIGHT, CWA_TOP);
	m_pLayoutManager->AddChildConstraint(IDC_LISTEXPORTS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_pLayoutManager->AddChildConstraint(IDC_CHKCPPNAMEUNMANGLEEXPORTS, CWA_DEFAULT, CWA_BOTTOM);

	// Set full row selection style for listviews
	ListView_SetExtendedListViewStyleEx(hListViewExportDir,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);
	ListView_SetExtendedListViewStyleEx(hListViewExports, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	// Fill them with data
	IMAGE_EXPORT_DIRECTORY &ExportDir = *m_PEReaderWriter.GetExportDirectory();

	// Prepare Tooltip for Export Directory listview
	RTTI::GetTooltipInfo(ExportDirTooltipInfo, (UINT_PTR) 0, RTTI::RTTI_EXPORT_DIRECTORY);

	// Data for Export Directory Table
	FillData(ExportDirInfo, _T("Export Flags"), DWORD_toString(ExportDir.Characteristics, Hexadecimal), _T("Reserved, must be zero"));
	FillData(ExportDirInfo, _T("Time/Date Stamp"), DWORD_toString(ExportDir.TimeDateStamp), TimeDateStamp_toString(ExportDir.TimeDateStamp));
	FillData(ExportDirInfo, _T("Version"), VersionNums_toString(ExportDir.MajorVersion, ExportDir.MinorVersion));
	FillData(ExportDirInfo, _T("Name RVA"), DWORD_toString(ExportDir.Name, Hexadecimal), _T("\"") +
																		MultiByte_toString((char *) m_PEReaderWriter.GetVA(ExportDir.Name)) +
																		_T("\""));
	FillData(ExportDirInfo, _T("Ordinal Base"), DWORD_toString(ExportDir.Base));
	FillData(ExportDirInfo, _T("Addr Table Entries"), DWORD_toString(ExportDir.NumberOfFunctions));
	FillData(ExportDirInfo, _T("No. of Name Ptrs"), DWORD_toString(ExportDir.NumberOfNames));
	FillData(ExportDirInfo, _T("Export Addr Table RVA"), DWORD_toString(ExportDir.AddressOfFunctions, Hexadecimal));
	FillData(ExportDirInfo, _T("Name Pointer RVA"), DWORD_toString(ExportDir.AddressOfNames, Hexadecimal));
	FillData(ExportDirInfo, _T("Ordinal Table RVA"), DWORD_toString(ExportDir.AddressOfNameOrdinals, Hexadecimal));

	// Data for Exports
	lstOrdinalsAndNames = m_PEReaderWriter.GetExportOrdinalsAndNames();
	lstExportRVAs = m_PEReaderWriter.GetExportRVAs();
	
	// Select each export address and find its corresponding 'Name', 'Ordinal' and 'Forwarder String' if any
	ExportsPage_UpdateDisplay(false);

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = GenericColumnText[i];
		ListView_InsertColumn(hListViewExportDir, i, &column);
	}

	// Insert ListView items for Export Directory view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < ExportDirInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) ExportDirInfo[i].szText.c_str();
		ListView_InsertItem(hListViewExportDir, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) ExportDirInfo[i].szData.c_str();
		ListView_SetItem(hListViewExportDir, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) ExportDirInfo[i].szComments.c_str();
		ListView_SetItem(hListViewExportDir, &item);
	}

	// Resize columns
	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
		ListView_SetColumnWidth(hListViewExportDir, i,
											i == GetArraySize(GenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);
}

tstring PropertyPageHandler_Exports::lstExportDir_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(ExportDirTooltipInfo, Index);
}

void PropertyPageHandler_Exports::lstExportDir_OnContextMenu(LONG x, LONG y, int Index)
{
	return Generic_OnContextMenu(ExportDirTooltipInfo, ExportDirInfo, x, y, Index);
}

void PropertyPageHandler_Exports::chkExportsUnmangleCPPNames_OnClick(HWND hControl, bool IsChecked)
{
	RECT ViewRect;

	// Save scrolled view rectangle
	ListView_GetViewRect(hListViewExports, &ViewRect);

	ExportsPage_UpdateDisplay(IsChecked);

	// Scroll to the place where the user was earlier
	ListView_Scroll(hListViewExports, abs(ViewRect.left), abs(ViewRect.top));
}

void PropertyPageHandler_Exports::lstExports_OnColumnHeaderClick(int Index)
{
	HWND hHeader = ListView_GetHeader(hListViewExports);
	HDITEM hdrItem;
	ZeroMemory(&hdrItem, sizeof(HDITEM));
	hdrItem.mask = HDI_FORMAT;
	hdrItem.fmt = HDF_LEFT | HDF_STRING;

	// Restore header's property to normal header without arrow
	for (int i = 0; i < Header_GetItemCount(hHeader); i++)
		Header_SetItem(hHeader, i, &hdrItem);

	switch (lstExportsSortOrder)
	{
	case None:
	case Descending:
		hdrItem.fmt |= HDF_SORTDOWN;
		lstExportsSortOrder = Ascending;

		break;

	case Ascending:
		hdrItem.fmt |= HDF_SORTUP;
		lstExportsSortOrder = Descending;
	}

	// Set header property of selected column with arrow
	Header_SetItem(hHeader, Index, &hdrItem);

	// Set sorting function
	CompareFuncParam Params = {hListViewExports, Index, lstExportsSortOrder};

	switch (Index)
	{
	case 0:
		ListView_SortItemsEx(hListViewExports, HexNumberCompareFunc, &Params);

		break;

	case 1:
	case 3:
		ListView_SortItemsEx(hListViewExports, StringCompareFunc, &Params);

		break;

	case 2:
		ListView_SortItemsEx(hListViewExports, NumberCompareFunc, &Params);
	}
}

void PropertyPageHandler_Exports::ExportsPage_UpdateDisplay(bool ChkBoxUnmangle_Checked)
{
	vector<RVANameOrdinalForwarder> ExportsInfo;
	IMAGE_EXPORT_DIRECTORY &ExportDir = *m_PEReaderWriter.GetExportDirectory();

	for (int i = 0; i < lstExportRVAs.size(); i++)
	{
		bool bFound = false;

		for (int j = 0; j < lstOrdinalsAndNames.size(); j++)
		{
			if (i == lstOrdinalsAndNames[j].first)
			{
				bFound = true;	// Address has a name and ordinal

				FillData2(ExportsInfo,
							DWORD_toString(lstExportRVAs[i], Hexadecimal),
							MultiByte_toString(ChkBoxUnmangle_Checked ?
												PEReadWrite::UnmangleCPPNames(lstOrdinalsAndNames[j].second, PEReadWrite::Full) :
												lstOrdinalsAndNames[j].second),
							DWORD_toString(lstOrdinalsAndNames[j].first + ExportDir.Base),
							m_PEReaderWriter.IsExportForwarderName(lstExportRVAs[i]) ?
								_T("\"") + m_PEReaderWriter.GetExportForwarderName(lstExportRVAs[i]) + _T("\"") : _T(""));
			}
		}

		if (!bFound)		// Address only has a ordinal
				FillData2(ExportsInfo,
							DWORD_toString(lstExportRVAs[i], Hexadecimal),
							_T("n/a"),
							DWORD_toString(i + ExportDir.Base),
							m_PEReaderWriter.IsExportForwarderName(lstExportRVAs[i]) ?
								_T("\"") + m_PEReaderWriter.GetExportForwarderName(lstExportRVAs[i]) + _T("\"") : _T(""));
	}

	// Delete all columns and all items in ListViewImportsFuncsAndDirTable
	while (ListView_DeleteColumn(hListViewExports, 0));
	ListView_DeleteAllItems(hListViewExports);

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));
	
	static LPTSTR szExportsColumnText[] = {_T("Function"), _T("Name"), _T("Ordinal"), _T("Forwarder")};
	for (unsigned int i = 0; i < GetArraySize(szExportsColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szExportsColumnText[i];
		ListView_InsertColumn(hListViewExports, i, &column);
	}

	//Insert ListView items for Exports
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < ExportsInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) ExportsInfo[i].RVA.c_str();
		ListView_InsertItem(hListViewExports, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) ExportsInfo[i].Name.c_str();
		ListView_SetItem(hListViewExports, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) ExportsInfo[i].Ordinal.c_str();
		ListView_SetItem(hListViewExports, &item);

		item.iSubItem = 3;
		item.pszText = (LPTSTR) ExportsInfo[i].Forwarder.c_str();
		ListView_SetItem(hListViewExports, &item);
	}

	// Resize columns
	for (unsigned int i = 0; i < GetArraySize(szExportsColumnText); i++)
		ListView_SetColumnWidth(hListViewExports, i, LVSCW_AUTOSIZE_USEHEADER);
}