#include "PropertyPageHandler.h"


PropertyPageHandler_Sections::PropertyPageHandler_Sections(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter)
{
	m_hTabsSections = GetDlgItem(m_hWnd, IDC_TABSSECTIONS);
	m_hListViewSections = GetDlgItem(m_hWnd, IDC_LISTSECTIONS);

	// Setup controls with layout manager
	m_pLayoutManager->AddChildConstraint(IDC_TABSSECTIONS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_pLayoutManager->AddChildConstraint(IDC_LISTSECTIONS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

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
	unsigned short cSections = m_PEReaderWriter.GetNoOfSections();
	if (cSections == 0)
	{
		LogError(_T("Invalid number of sections in PE header!"), true);
		return;
	}

	// Insert Tabs
	TCITEM item;
	for (unsigned short i = 0; i < cSections; i++)
	{
		tstring temp = _T("Section ") + DWORD_toString(i + 1);
		item.mask = TCIF_TEXT;
		item.pszText = (LPTSTR) temp.c_str();

		TabCtrl_InsertItem(m_hTabsSections, i, &item);
	}

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = GenericColumnText[i];

		ListView_InsertColumn(m_hListViewSections, i, &column);
	}

	// Emulate first tab being selected
	tabsSections_OnTabChanged(m_hWnd, 0);
}

void PropertyPageHandler_Sections::tabsSections_OnTabChanged(HWND hControl, int SelectedIndex)
{
	PIMAGE_SECTION_HEADER pSection = m_PEReaderWriter.GetSectionHeader(SelectedIndex);

	if (!pSection)
		throw std::exception();

	// Prepare Tooltip for Sections listview
	RTTI::GetTooltipInfo(m_SectionTooltipInfo, 0, RTTI::RTTI_SECTION_HEADER);

	// Fill with data
	m_SectionInfo.clear();
	FillData(m_SectionInfo, _T("Name"), ProperSectionName(pSection->Name),
															StandardSectionNameAnnotation(ProperSectionName(pSection->Name)));
	FillData(m_SectionInfo, _T("Virtual Size"), DWORD_toString(pSection->Misc.VirtualSize),
																				FormattedBytesSize(pSection->Misc.VirtualSize));
	FillData(m_SectionInfo, _T("Virtual Address"), DWORD_toString(pSection->VirtualAddress, Hexadecimal));
	FillData(m_SectionInfo, _T("Raw Data size"), DWORD_toString(pSection->SizeOfRawData),
																				FormattedBytesSize(pSection->SizeOfRawData));
	FillData(m_SectionInfo, _T("Ptr to Raw Data"), DWORD_toString(pSection->PointerToRawData, Hexadecimal));
	FillData(m_SectionInfo, _T("Ptr to Relocations"), DWORD_toString(pSection->PointerToRelocations, Hexadecimal));
	FillData(m_SectionInfo, _T("Ptr to Line nos."), DWORD_toString(pSection->PointerToLinenumbers, Hexadecimal), _T("Deprecated"));
	FillData(m_SectionInfo, _T("No. of Relocations"), DWORD_toString(pSection->NumberOfRelocations));
	FillData(m_SectionInfo, _T("No. of Line nos."), DWORD_toString(pSection->NumberOfLinenumbers), _T("Deprecated"));
	FillData(m_SectionInfo, _T("Characteristics"), DWORD_toString(pSection->Characteristics, Hexadecimal),
																	SectionCharacteristics_toString(pSection->Characteristics));

	ListView_DeleteAllItems(m_hListViewSections);

	// Insert ListView items
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < m_SectionInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) m_SectionInfo[i].szText.c_str();

		ListView_InsertItem(m_hListViewSections, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) m_SectionInfo[i].szData.c_str();

		ListView_SetItem(m_hListViewSections, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) m_SectionInfo[i].szComments.c_str();

		ListView_SetItem(m_hListViewSections, &item);
	}

	// Resize columns
	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
		ListView_SetColumnWidth(m_hListViewSections, i,
										i == GetArraySize(GenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);
}

tstring PropertyPageHandler_Sections::lstSections_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(m_SectionTooltipInfo, Index);
}

void PropertyPageHandler_Sections::lstSections_OnContextMenu(LONG x, LONG y, int Index)
{
	return Generic_OnContextMenu(m_SectionTooltipInfo, m_SectionInfo, x, y, Index);
}