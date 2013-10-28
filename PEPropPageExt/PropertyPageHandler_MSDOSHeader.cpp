#include "PropertyPageHandler.h"


PropertyPageHandler_MSDOSHeader::PropertyPageHandler_MSDOSHeader(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter)
{
	m_hListViewMSDOSHeader = GetDlgItem(m_hWnd, IDC_LISTMSDOSHEADERDATA);
	m_hEditMSDOSstub = GetDlgItem(m_hWnd, IDC_EDITMSDOSDISASSEMBLY);

	// Setup controls with layout manager
	m_pLayoutManager->AddChildConstraint(IDC_LISTMSDOSHEADERDATA, CWA_LEFTRIGHT, CWA_TOP);
	m_pLayoutManager->AddChildConstraint(IDC_EDITMSDOSDISASSEMBLY, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Set full row selection style for list view
	ListView_SetExtendedListViewStyleEx(m_hListViewMSDOSHeader,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

	// Set font for rich edit
	CHARFORMAT CharFormat;
	static const LPTSTR szPreferredFont = _T("MS Shell Dlg");

	ZeroMemory(&CharFormat, sizeof(CHARFORMAT));
	CharFormat.cbSize = sizeof(CHARFORMAT);
	CharFormat.dwMask = CFM_FACE;
	CopyMemory(CharFormat.szFaceName, szPreferredFont, _tcslen(szPreferredFont) + sizeof(TCHAR));
	SendMessage(m_hEditMSDOSstub, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM) &CharFormat);

	// Set tab stop  for rich edit
	DWORD cTabs = 100;
	Edit_SetTabStops(m_hEditMSDOSstub, 1, &cTabs);
}

void PropertyPageHandler_MSDOSHeader::OnInitDialog()
{
	// Fill them with data
	// Determine the type of primary header of the file
	switch (m_PEReaderWriter.GetPrimaryHeaderType())
	{
	case PEReadWrite::UnknownHeader:
		LogError(_T("Unrecognized MSDOS header signature! Assuming corrupted \"MZ\" header."), true);

	case PEReadWrite::MZHeader:
		{
			PIMAGE_DOS_HEADER pDOSheader = m_PEReaderWriter.GetPrimaryHeader<PIMAGE_DOS_HEADER>();
			if (!m_PEReaderWriter.IsMemoryReadable(pDOSheader, sizeof(IMAGE_DOS_HEADER)))
			{
				LogError(_T("'IMAGE_DOS_HEADER' structure is incomplete! Cannot continue."), true);
				return;
			}

			// Prepare Tooltip for MSDOS header listview
			RTTI::GetTooltipInfo(m_TooltipInfo, (UINT_PTR) 0, RTTI::RTTI_IMAGE_DOS_HEADER);
			
			FillData(m_ItemsInfo, _T("MS-DOS Signature"), DWORD_toString(pDOSheader->e_magic, Hexadecimal), 
												pDOSheader->e_magic == IMAGE_DOS_SIGNATURE ? _T("\"MZ\"") : _T("Invalid Signature"));
			FillData(m_ItemsInfo, _T("Bytes on last page of file"), DWORD_toString(pDOSheader->e_cblp),
																							FormattedBytesSize(pDOSheader->e_cblp));
			FillData(m_ItemsInfo, _T("No. of pages"), DWORD_toString(pDOSheader->e_cp));
			FillData(m_ItemsInfo, _T("Relocations"), DWORD_toString(pDOSheader->e_crlc));
			FillData(m_ItemsInfo, _T("Size of header in paragraphs"), DWORD_toString(pDOSheader->e_cparhdr));
			FillData(m_ItemsInfo, _T("Min extra paragraphs needed"), DWORD_toString(pDOSheader->e_minalloc));
			FillData(m_ItemsInfo, _T("Max extra paragraphs needed"), DWORD_toString(pDOSheader->e_maxalloc));
			FillData(m_ItemsInfo, _T("Initial (relative) SS value"), DWORD_toString(pDOSheader->e_ss, Hexadecimal));
			FillData(m_ItemsInfo, _T("Initial SP value"), DWORD_toString(pDOSheader->e_sp, Hexadecimal));
			FillData(m_ItemsInfo, _T("Checksum"), DWORD_toString(pDOSheader->e_csum));
			FillData(m_ItemsInfo, _T("Initial IP value"), DWORD_toString(pDOSheader->e_ip, Hexadecimal));
			FillData(m_ItemsInfo, _T("Initial (relative) CS value"), DWORD_toString(pDOSheader->e_cs, Hexadecimal));
			FillData(m_ItemsInfo, _T("File offset of relocation table"), DWORD_toString(pDOSheader->e_lfarlc, Hexadecimal));
			FillData(m_ItemsInfo, _T("Overlay number"), DWORD_toString(pDOSheader->e_ovno));
			FillData(m_ItemsInfo, _T("Reserved 4 words"), QWORD_toString(*((ULONGLONG *) pDOSheader->e_res), Hexadecimal),
														_T("Reserved, must be zero"));
			FillData(m_ItemsInfo, _T("OEM identifier"), DWORD_toString(pDOSheader->e_oemid));
			FillData(m_ItemsInfo, _T("OEM information"), DWORD_toString(pDOSheader->e_oeminfo));
			FillData(m_ItemsInfo, _T("Reserved 4 words"), QWORD_toString(*((ULONGLONG *) pDOSheader->e_res2), Hexadecimal),
														_T("Reserved, must be zero"));
			FillData(m_ItemsInfo, _T("Reserved 4 words"), QWORD_toString(*((ULONGLONG *) &pDOSheader->e_res2[4]), Hexadecimal),
														_T("Reserved, must be zero"));
			FillData(m_ItemsInfo, _T("Reserved 2 words"), DWORD_toString(*((ULONG *) &pDOSheader->e_res2[8]), Hexadecimal),
														_T("Reserved, must be zero"));
			FillData(m_ItemsInfo, _T("File offset of PE header"), DWORD_toString(pDOSheader->e_lfanew, Hexadecimal),
														(pDOSheader->e_lfanew == 0 ? _T("Invalid value, cannot be zero") : _T("")));

			// Disassemble MSDOS stub code using 'libudis86'
			ud_t Disassembler;
			ud_init(&Disassembler);						// Initialize disassembler
			ud_set_mode(&Disassembler, 16);				// Use 16-bit disassembly
			ud_set_pc(&Disassembler,					// Set EIP
						(UINT_PTR) m_PEReaderWriter.GetVA(pDOSheader->e_ip, true));
			ud_set_syntax(&Disassembler, UD_SYN_INTEL);	// Use Microsoft-Intel opcode mnemonics style

			if (pDOSheader->e_lfanew <= sizeof(IMAGE_DOS_HEADER))
				break;

			ud_set_input_buffer(&Disassembler, (uint8_t *) m_PEReaderWriter.GetVA(sizeof(IMAGE_DOS_HEADER), true),
								pDOSheader->e_lfanew - sizeof(IMAGE_DOS_HEADER));

			tstring Disassembly;						// Contains the final disassembly as output
			Disassembly = _T("Opcode\tMnemonic\n")
							_T("---------------------------------------------------------------\n");

			while (ud_disassemble(&Disassembler))		// Disassembly each block and produce in text form
			{
				// Format for each line of disassembly will be
				//	<Hexadecimal assembly> <Tab space> <Instruction Mnemonic>
				// eg: 00 01 02		mov eax, 1
				char *pBuffer = ud_insn_hex(&Disassembler);	// Get hexadecimal notation of opcode
				Disassembly += (pBuffer ? _T("0x") + MultiByte_toString(pBuffer) + _T('\t') : _T("<Invalid>\t"));

				pBuffer = ud_insn_asm(&Disassembler);		// Get mnemonic
				Disassembly += (pBuffer ? MultiByte_toString(pBuffer) : _T("<Invalid>"));
				Disassembly += _T('\n');
			}

			// Set text for edit box
			Edit_SetText(m_hEditMSDOSstub, (LPTSTR) Disassembly.c_str());
		}
	}
	
	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (int i = 0; i < GetArraySize(GenericColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = GenericColumnText[i];
		ListView_InsertColumn(m_hListViewMSDOSHeader, i, &column);
	}

	// Insert ListView items for MS-DOS data list view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for(int i = 0; i < m_ItemsInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) m_ItemsInfo[i].szText.c_str();
		ListView_InsertItem(m_hListViewMSDOSHeader, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) m_ItemsInfo[i].szData.c_str();
		ListView_SetItem(m_hListViewMSDOSHeader, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) m_ItemsInfo[i].szComments.c_str();
		ListView_SetItem(m_hListViewMSDOSHeader, &item);
	}

	// Resize column
	for(int i = 0; i < GetArraySize(GenericColumnText); i++)
		ListView_SetColumnWidth(m_hListViewMSDOSHeader, i, 
										i == GetArraySize(GenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);
}

tstring PropertyPageHandler_MSDOSHeader::lstMSDOSHeader_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(m_TooltipInfo, Index);
}

void PropertyPageHandler_MSDOSHeader::lstMSDOSHeader_OnContextMenu(LONG x, LONG y, int Index)
{
	Generic_OnContextMenu(m_TooltipInfo, m_ItemsInfo, x, y, Index);
}