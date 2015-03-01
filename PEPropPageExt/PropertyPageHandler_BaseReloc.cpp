#include "stdafx.h"
#include "PropertyPageHandler.h"


PropertyPageHandler_BaseReloc::PropertyPageHandler_BaseReloc(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, std::ref(PEReaderWriter))
{
	m_hTabsBaseReloc = GetDlgItem(m_hWnd, IDC_TABSBASERELOC);
	m_hListViewBaseRelocTable = GetDlgItem(m_hWnd, IDC_LISTBASERELOCTABLE);
	m_hEditFixupEntries = GetDlgItem(m_hWnd, IDC_EDITFIXUPENTRIES);

	// Setup controls
	m_LayoutManager.AddChildConstraint(IDC_TABSBASERELOC, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_LayoutManager.AddChildConstraint(IDC_LISTBASERELOCTABLE, CWA_LEFTRIGHT, CWA_TOP);
	m_LayoutManager.AddChildConstraint(IDC_EDITFIXUPENTRIES, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
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

	for (int i = 0; i < m_PEReaderWriter.getNoOfBaseRelocationTables(); ++i)
	{
		item.mask = TCIF_TEXT;
		item.pszText = LPWSTR(_wcsdup(wstring(L"Table " + DWORD_toString(i + 1)).c_str()));

		TabCtrl_InsertItem(m_hTabsBaseReloc, i, &item);
		free(item.pszText);
	}

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szGenericColumnText[i];

		ListView_InsertColumn(m_hListViewBaseRelocTable, i, &column);
	}

	tabsBaseRelocations_OnTabChanged(m_hWnd, 0);
}

wstring PropertyPageHandler_BaseReloc::lstBaseRelocTable_OnGetTooltip(int Index)
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
	PIMAGE_BASE_RELOCATION pBaseReloc;
	int err = m_PEReaderWriter.getBaseRelocationTable(SelectedIndex, std::ref(pBaseReloc));
	if (err)
	{
		LogError(L"ERROR: Couldn't read base relocation table at index " + DWORD_toString(SelectedIndex) + L". File is not valid.", true);
		return;
	}

	m_BaseRelocInfo.clear();
	m_BaseRelocInfo =
	{
		{ L"Page RVA", DWORD_toString(pBaseReloc->VirtualAddress, Hexadecimal) },
		{ L"Block size", DWORD_toString(pBaseReloc->SizeOfBlock), FormattedBytesSize(pBaseReloc->SizeOfBlock) }
	};

	// Insert ListView items for Debug Directory view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (size_t i = 0; i < m_BaseRelocInfo.size(); ++i)
	{
		item.iItem = int(i);
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = LPWSTR(_wcsdup(m_BaseRelocInfo[i].Text.c_str()));
		ListView_InsertItem(m_hListViewBaseRelocTable, &item);
		free(item.pszText);

		item.iSubItem = 1;
		item.pszText = LPWSTR(_wcsdup(m_BaseRelocInfo[i].Data.c_str()));
		ListView_SetItem(m_hListViewBaseRelocTable, &item);
		free(item.pszText);

		item.iSubItem = 2;
		item.pszText = LPWSTR(_wcsdup(m_BaseRelocInfo[i].Comments.c_str()));
		ListView_SetItem(m_hListViewBaseRelocTable, &item);
		free(item.pszText);
	}

	// Resize columns
	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
		ListView_SetColumnWidth(m_hListViewBaseRelocTable,
								i,
								(i == ARRAYSIZE(szGenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));


	wstring BaseRelocInfo;
	const int cFixupEntries = m_PEReaderWriter.getNoOfBaseRelocationEntries(std::cref(pBaseReloc));

	for (int i = 0; i < cFixupEntries; ++i)
	{
		PEReadWrite::PIMAGE_FIXUP_ENTRY pFixupEntry;
		err = m_PEReaderWriter.getBaseRelocationEntry(std::cref(pBaseReloc), i, std::ref(pFixupEntry));
		if (err)
		{
			LogError(L"ERROR: Couldn't read base relocation entry at index " +
					 DWORD_toString(i) + L" for base relocation directory at index " +
					 DWORD_toString(SelectedIndex) + L". File is not valid.", true);
			break;
		}

		static auto funcMakeBaseRelocEntryDesc = [](wstring& info, wstring label, wstring value) -> void
		{
			info.append(label + L": " + value + L'\n');
		};

		funcMakeBaseRelocEntryDesc(std::ref(BaseRelocInfo), L"Offset", DWORD_toString(pFixupEntry->offset, Hexadecimal));
		funcMakeBaseRelocEntryDesc(std::ref(BaseRelocInfo), L"Type", DWORD_toString(pFixupEntry->type, Hexadecimal)
								   + L" (" + BaseRelocType_toString(pFixupEntry->type) + L")\n");
	}

	if (BaseRelocInfo.size() > 2)
		BaseRelocInfo.resize(BaseRelocInfo.size() - 2); // Remove last two newline characters
	Edit_SetText(m_hEditFixupEntries, BaseRelocInfo.c_str());
}