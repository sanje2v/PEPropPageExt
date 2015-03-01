#include "PropertyPageHandler.h"


PropertyPageHandler_MSDOSHeader::PropertyPageHandler_MSDOSHeader(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, std::ref(PEReaderWriter))
{
	m_hListViewMSDOSHeader = GetDlgItem(m_hWnd, IDC_LISTMSDOSHEADERDATA);
	m_hEditMSDOSstub = GetDlgItem(m_hWnd, IDC_EDITMSDOSDISASSEMBLY);

	// Setup controls with layout manager
	m_LayoutManager.AddChildConstraint(IDC_LISTMSDOSHEADERDATA, CWA_LEFTRIGHT, CWA_TOP);
	m_LayoutManager.AddChildConstraint(IDC_EDITMSDOSDISASSEMBLY, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Set full row selection style for list view
	ListView_SetExtendedListViewStyleEx(m_hListViewMSDOSHeader,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

	// Set font for rich edit
	CHARFORMAT CharFormat;
	ZeroMemory(&CharFormat, sizeof(CharFormat));

	CharFormat.cbSize = sizeof(CharFormat);
	CharFormat.dwMask = CFM_FACE;
	CopyMemory(CharFormat.szFaceName, szPreferredFont, wcslen(szPreferredFont) + sizeof(WCHAR));
	SendMessage(m_hEditMSDOSstub, EM_SETCHARFORMAT, SCF_DEFAULT, LPARAM(&CharFormat));

	// Set tab stop  for rich edit
	DWORD _cTabs = cTabs;
	Edit_SetTabStops(m_hEditMSDOSstub, 1, &_cTabs);
}

void PropertyPageHandler_MSDOSHeader::OnInitDialog()
{
	// Fill them with data
	m_ItemsInfo.reserve(21);
	m_TooltipInfo.reserve(21);

	// Determine the type of primary header of the file
	int err;
	switch (m_PEReaderWriter.getPrimaryHeaderType())
	{
		case PEReadWrite::HeaderType::Unknown:
			LogError(L"ERROR: MSDOS header signature not found. File is not valid.", true);
			return;

		case PEReadWrite::HeaderType::MZ:
		{
			PIMAGE_DOS_HEADER pDOSheader;
			err = m_PEReaderWriter.getPrimaryHeader<PIMAGE_DOS_HEADER> (std::ref(pDOSheader));
			if (err)
			{
				LogError(L"ERROR: Incomplete 'IMAGE_DOS_HEADER' data. File is not valid.", true);
				return;
			}

			// Prepare Tooltip for MSDOS header listview
			RTTI::GetTooltipInfo(m_TooltipInfo, UINT_PTR(NULL), RTTI::RTTI_IMAGE_DOS_HEADER);

			m_ItemsInfo = 
			{
				{
					L"MS-DOS Signature",
					DWORD_toString(pDOSheader->e_magic, Hexadecimal),
					(pDOSheader->e_magic == IMAGE_DOS_SIGNATURE ? L"\"MZ\"" : L"Invalid Signature")
				},
				{
					L"Bytes on last page of file",
					DWORD_toString(pDOSheader->e_cblp),
					FormattedBytesSize(pDOSheader->e_cblp)
				},
				{ L"No. of pages", DWORD_toString(pDOSheader->e_cp) },
				{ L"Relocations", DWORD_toString(pDOSheader->e_crlc) },
				{ L"Size of header in paragraphs", DWORD_toString(pDOSheader->e_cparhdr) },
				{ L"Min extra paragraphs needed", DWORD_toString(pDOSheader->e_minalloc) },
				{ L"Max extra paragraphs needed", DWORD_toString(pDOSheader->e_maxalloc) },
				{ L"Initial (relative) SS value", DWORD_toString(pDOSheader->e_ss, Hexadecimal) },
				{ L"Initial SP value", DWORD_toString(pDOSheader->e_sp, Hexadecimal) },
				{ L"Checksum", DWORD_toString(pDOSheader->e_csum) },
				{ L"Initial IP value", DWORD_toString(pDOSheader->e_ip, Hexadecimal) },
				{ L"Initial (relative) CS value", DWORD_toString(pDOSheader->e_cs, Hexadecimal) },
				{ L"File offset of relocation table", DWORD_toString(pDOSheader->e_lfarlc, Hexadecimal) },
				{ L"Overlay number", DWORD_toString(pDOSheader->e_ovno) },
				{
					L"Reserved 4 words",
					QWORD_toString(*PULONGLONG(pDOSheader->e_res), Hexadecimal),
					L"Reserved, must be zero"
				},
				{ L"OEM identifier", DWORD_toString(pDOSheader->e_oemid) },
				{ L"OEM information", DWORD_toString(pDOSheader->e_oeminfo) },
				{
					L"Reserved 4 words",
					QWORD_toString(*PULONGLONG(pDOSheader->e_res2), Hexadecimal),
					L"Reserved, must be zero"
				},
				{
					L"Reserved 4 words",
					QWORD_toString(*PULONGLONG(&pDOSheader->e_res2[4]), Hexadecimal),
					L"Reserved, must be zero"
				},
				{
					L"Reserved 2 words",
					DWORD_toString(*PULONG(&pDOSheader->e_res2[8]), Hexadecimal),
					L"Reserved, must be zero"
				},
				{
					L"File offset of PE header",
					DWORD_toString(pDOSheader->e_lfanew, Hexadecimal),
					(pDOSheader->e_lfanew == 0 ? L"Invalid value, cannot be zero" : L"")
				}
			};

			// Check whether there is a 'Rich' data between MSDOS and PE header
			DWORD nSignDwords;

			if (m_PEReaderWriter.hasRichSignature(std::ref(nSignDwords)))
			{
				vector<PEReadWrite::RichSigVCCompilerInfo> RichData;
				err = m_PEReaderWriter.getRichVCToolsData(nSignDwords, std::ref(RichData));
				if (err)
				{
					LogError(L"ERROR: Rich data is incomplete. File is not valid!", true);
					break;
				}

				wstring RichMsg = L"Found 'Rich' data between IMAGE_DOS_HEADER and IMAGE_PE_HEADERS.\n"
								  L"These are normally produced by Visual C++ compilers. Data held:\n\n";

				for (size_t i = 0; i < RichData.size(); ++i)
					RichMsg += L"ID: " + DWORD_toString(RichData[i].id) + L", "
							+ L"Compiler min version: " + DWORD_toString(RichData[i].minver) + L", "
							+ L"Times: " + DWORD_toString(RichData[i].vnum) + L'\n';

				RichMsg += L"\nPlease refer to 'http://www.ntcore.com/files/richsign.htm' for more information about this data.";

				LogError(RichMsg);
			}

			// Disassemble MSDOS stub code using 'libudis86'
			wstring Disassembly;
			err = m_PEReaderWriter.disassembleMSDOSstub(std::ref(Disassembly));
			if (err)
			{
				LogError(L"ERROR: MSDOS stub code couldn't be disassembled. File is not valid.", true);
				return;
			}

			Disassembly = L"Opcode\tMnemonic\n"
						  L"---------------------------------------------------------------\n"
						  + Disassembly;

			// Set text for edit box
			Edit_SetText(m_hEditMSDOSstub, Disassembly.c_str());
		}
	}
	
	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szGenericColumnText[i];
		ListView_InsertColumn(m_hListViewMSDOSHeader, i, &column);
	}

	// Insert ListView items for MS-DOS data list view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for(size_t i = 0; i < m_ItemsInfo.size(); ++i)
	{
		item.iItem = int(i);
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = LPWSTR(_wcsdup(m_ItemsInfo[i].Text.c_str()));
		ListView_InsertItem(m_hListViewMSDOSHeader, &item);
		free(item.pszText);

		item.iSubItem = 1;
		item.pszText = LPWSTR(_wcsdup(m_ItemsInfo[i].Data.c_str()));
		ListView_SetItem(m_hListViewMSDOSHeader, &item);
		free(item.pszText);

		item.iSubItem = 2;
		item.pszText = LPWSTR(_wcsdup(m_ItemsInfo[i].Comments.c_str()));
		ListView_SetItem(m_hListViewMSDOSHeader, &item);
		free(item.pszText);
	}

	// Resize column
	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
		ListView_SetColumnWidth(m_hListViewMSDOSHeader,
								i,
								(i == ARRAYSIZE(szGenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));
}

wstring PropertyPageHandler_MSDOSHeader::lstMSDOSHeader_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(m_TooltipInfo, Index);
}

void PropertyPageHandler_MSDOSHeader::lstMSDOSHeader_OnContextMenu(LONG x, LONG y, int Index)
{
	Generic_OnContextMenu(m_TooltipInfo, m_ItemsInfo, x, y, Index);
}