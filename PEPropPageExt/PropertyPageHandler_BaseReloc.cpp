#include "PropertyPageHandler.h"


#define FillData2(stringobj, label, value)	(stringobj).append(label _T(": ") + tstring(value) + _T('\n'))


void PropertyPageHandler_BaseReloc::OnInitDialog()
{
	hTabsBaseReloc = GetDlgItem(m_hWnd, IDC_TABSBASERELOC);
	hListViewBaseRelocTable = GetDlgItem(m_hWnd, IDC_LISTBASERELOCTABLE);
	hEditFixupEntries = GetDlgItem(m_hWnd, IDC_EDITFIXUPENTRIES);

	// Setup controls
	m_pLayoutManager->AddChildConstraint(IDC_TABSBASERELOC, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_pLayoutManager->AddChildConstraint(IDC_LISTBASERELOCTABLE, CWA_LEFTRIGHT, CWA_TOP);
	m_pLayoutManager->AddChildConstraint(IDC_EDITFIXUPENTRIES, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	SetWindowPos(hTabsBaseReloc, hListViewBaseRelocTable, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(hTabsBaseReloc, hEditFixupEntries, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	// Set full row selection style for list view
	ListView_SetExtendedListViewStyleEx(hListViewBaseRelocTable,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

	// Get tooltip data
	RTTI::GetTooltipInfo(BaseRelocTooltipInfo, 0, RTTI::RTTI_BASE_RELOC);

	// Insert Tabs
	TCITEM item;
	for (int i = 0; i < m_PEReaderWriter.GetNoOfBaseRelocationTables(); i++)
	{
		tstring temp = _T("Table ") + DWORD_toString(i + 1);
		item.mask = TCIF_TEXT;
		item.pszText = (LPTSTR) temp.c_str();

		TabCtrl_InsertItem(hTabsBaseReloc, i, &item);
	}

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = GenericColumnText[i];

		ListView_InsertColumn(hListViewBaseRelocTable, i, &column);
	}

	tabsBaseRelocations_OnTabChanged(m_hWnd, 0);
}

tstring PropertyPageHandler_BaseReloc::lstBaseRelocTable_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(BaseRelocTooltipInfo, Index);
}

void PropertyPageHandler_BaseReloc::lstBaseRelocTable_OnContextMenu(LONG x, LONG y, int Index)
{
	return Generic_OnContextMenu(BaseRelocTooltipInfo, BaseRelocInfo, x, y, Index);
}

void PropertyPageHandler_BaseReloc::tabsBaseRelocations_OnTabChanged(HWND hControl, int SelectedIndex)
{
	ListView_DeleteAllItems(hListViewBaseRelocTable);

	// Fill with data
	BaseRelocInfo.clear();
	PIMAGE_BASE_RELOCATION pBaseReloc = m_PEReaderWriter.GetBaseRelocationTable(SelectedIndex);

	FillData(BaseRelocInfo, _T("Page RVA"), DWORD_toString(pBaseReloc->VirtualAddress, Hexadecimal));
	FillData(BaseRelocInfo, _T("Block size"), DWORD_toString(pBaseReloc->SizeOfBlock), FormattedBytesSize(pBaseReloc->SizeOfBlock));

	// Insert ListView items for Debug Directory view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < BaseRelocInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) BaseRelocInfo[i].szText.c_str();
		ListView_InsertItem(hListViewBaseRelocTable, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) BaseRelocInfo[i].szData.c_str();
		ListView_SetItem(hListViewBaseRelocTable, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) BaseRelocInfo[i].szComments.c_str();
		ListView_SetItem(hListViewBaseRelocTable, &item);
	}

	// Resize columns
	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
		ListView_SetColumnWidth(hListViewBaseRelocTable, i,
											i == GetArraySize(GenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);


	tstring buffer;
	int cFixupEntries = m_PEReaderWriter.GetNoOfBaseRelocationEntries(SelectedIndex);
	for (int i = 0; i < cFixupEntries; i++)
	{
		PIMAGE_FIXUP_ENTRY pFixupEntry = m_PEReaderWriter.GetBaseRelocationEntry(SelectedIndex, i);
		
		FillData2(buffer, _T("Offset"), DWORD_toString(pFixupEntry->offset, Hexadecimal));
		FillData2(buffer, _T("Type"), DWORD_toString(pFixupEntry->type, Hexadecimal) + _T(" (") + BaseRelocType_toString(pFixupEntry->type) + _T(")"));

		buffer.append(_T("\n"));
	}

	buffer.resize(buffer.size() - 2);
	Edit_SetText(hEditFixupEntries, buffer.c_str());
}