#include "PropertyPageHandler.h"


#define FillData2(stringobj, label, value)	(stringobj).append(label _T(": ") + tstring(value) + _T('\n'))


PropertyPageHandler_BaseReloc::PropertyPageHandler_BaseReloc(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter)
{
	m_hTabsBaseReloc = GetDlgItem(m_hWnd, IDC_TABSBASERELOC);
	m_hListViewBaseRelocTable = GetDlgItem(m_hWnd, IDC_LISTBASERELOCTABLE);
	m_hEditFixupEntries = GetDlgItem(m_hWnd, IDC_EDITFIXUPENTRIES);

	// Setup controls
	m_pLayoutManager->AddChildConstraint(IDC_TABSBASERELOC, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_pLayoutManager->AddChildConstraint(IDC_LISTBASERELOCTABLE, CWA_LEFTRIGHT, CWA_TOP);
	m_pLayoutManager->AddChildConstraint(IDC_EDITFIXUPENTRIES, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	SetWindowPos(m_hTabsBaseReloc, m_hListViewBaseRelocTable, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(m_hTabsBaseReloc, m_hEditFixupEntries, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	// Set full row selection style for list view
	ListView_SetExtendedListViewStyleEx(m_hListViewBaseRelocTable,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

	// Get tooltip data
	RTTI::GetTooltipInfo(m_BaseRelocTooltipInfo, 0, RTTI::RTTI_BASE_RELOC);
}

void PropertyPageHandler_BaseReloc::OnInitDialog()
{
	// Insert Tabs
	TCITEM item;
	for (int i = 0; i < m_PEReaderWriter.GetNoOfBaseRelocationTables(); i++)
	{
		tstring temp = _T("Table ") + DWORD_toString(i + 1);
		item.mask = TCIF_TEXT;
		item.pszText = (LPTSTR) temp.c_str();

		TabCtrl_InsertItem(m_hTabsBaseReloc, i, &item);
	}

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = GenericColumnText[i];

		ListView_InsertColumn(m_hListViewBaseRelocTable, i, &column);
	}

	tabsBaseRelocations_OnTabChanged(m_hWnd, 0);
}

tstring PropertyPageHandler_BaseReloc::lstBaseRelocTable_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(m_BaseRelocTooltipInfo, Index);
}

void PropertyPageHandler_BaseReloc::lstBaseRelocTable_OnContextMenu(LONG x, LONG y, int Index)
{
	return Generic_OnContextMenu(m_BaseRelocTooltipInfo, m_BaseRelocInfo, x, y, Index);
}

void PropertyPageHandler_BaseReloc::tabsBaseRelocations_OnTabChanged(HWND hControl, int SelectedIndex)
{
	ListView_DeleteAllItems(m_hListViewBaseRelocTable);

	// Fill with data
	m_BaseRelocInfo.clear();
	PIMAGE_BASE_RELOCATION pBaseReloc = m_PEReaderWriter.GetBaseRelocationTable(SelectedIndex);

	FillData(m_BaseRelocInfo, _T("Page RVA"), DWORD_toString(pBaseReloc->VirtualAddress, Hexadecimal));
	FillData(m_BaseRelocInfo, _T("Block size"), DWORD_toString(pBaseReloc->SizeOfBlock), FormattedBytesSize(pBaseReloc->SizeOfBlock));

	// Insert ListView items for Debug Directory view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < m_BaseRelocInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) m_BaseRelocInfo[i].szText.c_str();
		ListView_InsertItem(m_hListViewBaseRelocTable, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) m_BaseRelocInfo[i].szData.c_str();
		ListView_SetItem(m_hListViewBaseRelocTable, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) m_BaseRelocInfo[i].szComments.c_str();
		ListView_SetItem(m_hListViewBaseRelocTable, &item);
	}

	// Resize columns
	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
		ListView_SetColumnWidth(m_hListViewBaseRelocTable, i,
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
	Edit_SetText(m_hEditFixupEntries, buffer.c_str());
}