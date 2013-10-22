#include "PropertyPageHandler.h"


void PropertyPageHandler_MSDOSHeader::OnInitDialog()
{
	hListViewMSDOSHeader = GetDlgItem(m_hWnd, IDC_LISTMSDOSHEADERDATA);
	hEditMSDOSstub = GetDlgItem(m_hWnd, IDC_EDITMSDOSDISASSEMBLY);

	// Setup controls with layout manager
	m_pLayoutManager->AddChildConstraint(IDC_LISTMSDOSHEADERDATA, CWA_LEFTRIGHT, CWA_TOP);
	m_pLayoutManager->AddChildConstraint(IDC_EDITMSDOSDISASSEMBLY, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Set full row selection style for list view
	ListView_SetExtendedListViewStyleEx(hListViewMSDOSHeader,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

	// Set font for rich edit
	CHARFORMAT CharFormat;
	static const LPTSTR szPreferredFont = _T("MS Shell Dlg");

	ZeroMemory(&CharFormat, sizeof(CHARFORMAT));
	CharFormat.cbSize = sizeof(CHARFORMAT);
	CharFormat.dwMask = CFM_FACE;
	CopyMemory(CharFormat.szFaceName, szPreferredFont, _tcslen(szPreferredFont) + sizeof(TCHAR));
	SendMessage(hEditMSDOSstub, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM) &CharFormat);

	// Set tab stop  for rich edit
	DWORD cTabs = 100;
	Edit_SetTabStops(hEditMSDOSstub, 1, &cTabs);

	// Fill them with data
	// Determine the type of primary header of the file
	switch (m_PEReaderWriter.GetPrimaryHeaderType())
	{
	case PEReadWrite::UnknownHeader:
		LogError(_T("Unrecognized MSDOS header signature! Assuming corrupted \"MZ\" header."), true);

	case PEReadWrite::MZHeader:
		{
			PIMAGE_DOS_HEADER pDOSheader = m_PEReaderWriter.GetPrimaryHeader<PIMAGE_DOS_HEADER>();

			// Prepare Tooltip for MSDOS header listview
			RTTI::GetTooltipInfo(TooltipInfo, (UINT_PTR) 0, RTTI::RTTI_IMAGE_DOS_HEADER);
			
			FillData(ItemsInfo, _T("MS-DOS Signature"), DWORD_toString(pDOSheader->e_magic, Hexadecimal), 
												pDOSheader->e_magic == IMAGE_DOS_SIGNATURE ? _T("\"MZ\"") : _T("Invalid Signature"));
			FillData(ItemsInfo, _T("Bytes on last page of file"), DWORD_toString(pDOSheader->e_cblp),
																							FormattedBytesSize(pDOSheader->e_cblp));
			FillData(ItemsInfo, _T("No. of pages"), DWORD_toString(pDOSheader->e_cp));
			FillData(ItemsInfo, _T("Relocations"), DWORD_toString(pDOSheader->e_crlc));
			FillData(ItemsInfo, _T("Size of header in paragraphs"), DWORD_toString(pDOSheader->e_cparhdr));
			FillData(ItemsInfo, _T("Min extra paragraphs needed"), DWORD_toString(pDOSheader->e_minalloc));
			FillData(ItemsInfo, _T("Max extra paragraphs needed"), DWORD_toString(pDOSheader->e_maxalloc));
			FillData(ItemsInfo, _T("Initial (relative) SS value"), DWORD_toString(pDOSheader->e_ss, Hexadecimal));
			FillData(ItemsInfo, _T("Initial SP value"), DWORD_toString(pDOSheader->e_sp, Hexadecimal));
			FillData(ItemsInfo, _T("Checksum"), DWORD_toString(pDOSheader->e_csum));
			FillData(ItemsInfo, _T("Initial IP value"), DWORD_toString(pDOSheader->e_ip, Hexadecimal));
			FillData(ItemsInfo, _T("Initial (relative) CS value"), DWORD_toString(pDOSheader->e_cs, Hexadecimal));
			FillData(ItemsInfo, _T("File offset of relocation table"), DWORD_toString(pDOSheader->e_lfarlc, Hexadecimal));
			FillData(ItemsInfo, _T("Overlay number"), DWORD_toString(pDOSheader->e_ovno));
			FillData(ItemsInfo, _T("Reserved 4 words"), QWORD_toString(*((ULONGLONG *) pDOSheader->e_res), Hexadecimal),
														_T("Reserved, must be zero"));
			FillData(ItemsInfo, _T("OEM identifier"), DWORD_toString(pDOSheader->e_oemid));
			FillData(ItemsInfo, _T("OEM information"), DWORD_toString(pDOSheader->e_oeminfo));
			FillData(ItemsInfo, _T("Reserved 4 words"), QWORD_toString(*((ULONGLONG *) pDOSheader->e_res2), Hexadecimal),
														_T("Reserved, must be zero"));
			FillData(ItemsInfo, _T("Reserved 4 words"), QWORD_toString(*((ULONGLONG *) &pDOSheader->e_res2[4]), Hexadecimal),
														_T("Reserved, must be zero"));
			FillData(ItemsInfo, _T("Reserved 2 words"), DWORD_toString(*((ULONG *) &pDOSheader->e_res2[8]), Hexadecimal),
														_T("Reserved, must be zero"));
			FillData(ItemsInfo, _T("File offset of PE header"), DWORD_toString(pDOSheader->e_lfanew, Hexadecimal),
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

			wstring Disassembly;						// Contains the final disassembly as output
			Disassembly = _T("Opcode\tMnemonic\n---------------------------------------------------------------\n");

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
			Edit_SetText(hEditMSDOSstub, (LPTSTR) Disassembly.c_str());
		}
	}
	
	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = GenericColumnText[i];
		ListView_InsertColumn(hListViewMSDOSHeader, i, &column);
	}

	// Insert ListView items for MS-DOS data list view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for(unsigned int i = 0; i < ItemsInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) ItemsInfo[i].szText.c_str();
		ListView_InsertItem(hListViewMSDOSHeader, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) ItemsInfo[i].szData.c_str();
		ListView_SetItem(hListViewMSDOSHeader, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) ItemsInfo[i].szComments.c_str();
		ListView_SetItem(hListViewMSDOSHeader, &item);
	}

	// Resize column
	for(unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
		ListView_SetColumnWidth(hListViewMSDOSHeader, i, 
										i == GetArraySize(GenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);
}

tstring PropertyPageHandler_MSDOSHeader::lstMSDOSHeader_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(TooltipInfo, Index);
}

void PropertyPageHandler_MSDOSHeader::lstMSDOSHeader_OnContextMenu(LONG x, LONG y, int Index)
{
	Generic_OnContextMenu(TooltipInfo, ItemsInfo, x, y, Index);
}