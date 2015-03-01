#include "stdafx.h"
#include "PropertyPageHandler.h"


PropertyPageHandler_Sections::PropertyPageHandler_Sections(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, std::ref(PEReaderWriter))
{
	m_hTabsSections = GetDlgItem(m_hWnd, IDC_TABSSECTIONS);
	m_hListViewSections = GetDlgItem(m_hWnd, IDC_LISTSECTIONS);

	// Setup controls with layout manager
	m_LayoutManager.AddChildConstraint(IDC_TABSSECTIONS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_LayoutManager.AddChildConstraint(IDC_LISTSECTIONS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Make ZOrder: Listview (On top) and Tab Control (On bottom)
	SetWindowPos(m_hTabsSections, m_hListViewSections, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	// Set full row selection style for listviews
	ListView_SetExtendedListViewStyleEx(m_hListViewSections,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);
}

void PropertyPageHandler_Sections::OnInitDialog()
{
	// Fill controls with data
	m_SectionInfo.reserve(10);
	m_SectionTooltipInfo.reserve(10);

	WORD cSections = m_PEReaderWriter.getNoOfSections();
	if (cSections == 0)
	{
		LogError(L"ERROR: The number of sections is zero. File is not valid.", true);
		return;
	}

	// Insert Tabs
	TCITEM item;
	for (WORD i = 0; i < cSections; ++i)
	{
		item.mask = TCIF_TEXT;
		item.pszText = LPWSTR(_wcsdup(wstring(L"Section " + DWORD_toString(i + 1)).c_str()));

		TabCtrl_InsertItem(m_hTabsSections, i, &item);
		free(item.pszText);
	}

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szGenericColumnText[i];

		ListView_InsertColumn(m_hListViewSections, i, &column);
	}

	// Emulate first tab being selected
	tabsSections_OnTabChanged(m_hWnd, 0);
}

void PropertyPageHandler_Sections::tabsSections_OnTabChanged(HWND hControl, int SelectedIndex)
{
	PIMAGE_SECTION_HEADER pSection;
	int err = m_PEReaderWriter.getSectionHeader(SelectedIndex, pSection);
	if (err)
	{
		LogError(L"ERROR: Couldn't read section header at index " + DWORD_toString(SelectedIndex) + L". File is not valid.");
		return;
	}

	// Prepare Tooltip for Sections listview
	HINSTANCE hMod = 0;
	m_PEReaderWriter.getPrimaryHeader(std::ref(hMod));
	UINT_PTR fileOffset = UINT_PTR(pSection) - UINT_PTR(hMod);
	RTTI::GetTooltipInfo(m_SectionTooltipInfo, fileOffset, RTTI::RTTI_SECTION_HEADER);

	// Fill with data
	m_SectionInfo.clear();
	m_SectionInfo =
	{
		{ L"Name", ProperSectionName(pSection->Name), StandardSectionNameAnnotation(ProperSectionName(pSection->Name)) },
		{ L"Virtual Size", DWORD_toString(pSection->Misc.VirtualSize), FormattedBytesSize(pSection->Misc.VirtualSize) },
		{ L"Virtual Address", DWORD_toString(pSection->VirtualAddress, Hexadecimal) },
		{ L"Raw Data size", DWORD_toString(pSection->SizeOfRawData), FormattedBytesSize(pSection->SizeOfRawData) },
		{ L"Ptr to Raw Data", DWORD_toString(pSection->PointerToRawData, Hexadecimal) },
		{ L"Ptr to Relocations", DWORD_toString(pSection->PointerToRelocations, Hexadecimal) },
		{ L"Ptr to Line nos.", DWORD_toString(pSection->PointerToLinenumbers, Hexadecimal), L"Deprecated" },
		{ L"No. of Relocations", DWORD_toString(pSection->NumberOfRelocations) },
		{ L"No. of Line nos.", DWORD_toString(pSection->NumberOfLinenumbers), L"Deprecated" },
		{ L"Characteristics", DWORD_toString(pSection->Characteristics, Hexadecimal), SectionCharacteristics_toString(pSection->Characteristics) }
	};

	ListView_DeleteAllItems(m_hListViewSections);

	// Insert ListView items
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (size_t i = 0; i < m_SectionInfo.size(); ++i)
	{
		item.iItem = int(i);
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = LPWSTR(_wcsdup(m_SectionInfo[i].Text.c_str()));
		ListView_InsertItem(m_hListViewSections, &item);
		free(item.pszText);

		item.iSubItem = 1;
		item.pszText = LPWSTR(_wcsdup(m_SectionInfo[i].Data.c_str()));
		ListView_SetItem(m_hListViewSections, &item);
		free(item.pszText);

		item.iSubItem = 2;
		item.pszText = LPWSTR(_wcsdup(m_SectionInfo[i].Comments.c_str()));
		ListView_SetItem(m_hListViewSections, &item);
		free(item.pszText);
	}

	// Resize columns
	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
		ListView_SetColumnWidth(m_hListViewSections,
								i,
								(i == ARRAYSIZE(szGenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));
}

wstring PropertyPageHandler_Sections::lstSections_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(m_SectionTooltipInfo, Index);
}

void PropertyPageHandler_Sections::lstSections_OnContextMenu(LONG x, LONG y, int Index)
{
	return Generic_OnContextMenu(m_SectionTooltipInfo, m_SectionInfo, x, y, Index);
}