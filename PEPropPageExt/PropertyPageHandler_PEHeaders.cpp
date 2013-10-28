#include "PropertyPageHandler.h"


PropertyPageHandler_PEHeaders::PropertyPageHandler_PEHeaders(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter)
{
	m_hListViewCOFFHeader = GetDlgItem(m_hWnd, IDC_LISTCOFFHEADERDATA);
	m_hListViewOptionalHeader = GetDlgItem(m_hWnd, IDC_LISTOPTIONALHEADERDATA);

	// Setup controls with layout manager
	m_pLayoutManager->AddChildConstraint(IDC_LISTCOFFHEADERDATA, CWA_LEFTRIGHT, CWA_TOP);
	m_pLayoutManager->AddChildConstraint(IDC_LISTOPTIONALHEADERDATA, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Set full row selection style for list views
	ListView_SetExtendedListViewStyleEx(m_hListViewCOFFHeader,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);
	ListView_SetExtendedListViewStyleEx(m_hListViewOptionalHeader,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);
}

void PropertyPageHandler_PEHeaders::OnInitDialog()
{
	// Fill them with data
	switch (m_PEReaderWriter.GetSecondaryHeaderType())
	{
	case PEReadWrite::NEHeader:
		{
			PIMAGE_OS2_HEADER pNEheader = m_PEReaderWriter.GetSecondaryHeader<PIMAGE_OS2_HEADER>();

			FillData(m_COFFItemsInfo, _T("OS/2 Signature"), DWORD_toString(pNEheader->ne_magic, Hexadecimal), _T("\"NE\""));
			FillData(m_COFFItemsInfo, _T("Version no."), VersionNums_toString(pNEheader->ne_ver, pNEheader->ne_rev));
			FillData(m_COFFItemsInfo, _T("Entry Table offset"), DWORD_toString(pNEheader->ne_enttab, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Entry Table Size"), DWORD_toString(pNEheader->ne_cbenttab),
																					FormattedBytesSize(pNEheader->ne_cbenttab));
			FillData(m_COFFItemsInfo, _T("Checksum"), DWORD_toString(pNEheader->ne_crc));
			FillData(m_COFFItemsInfo, _T("Flags"), DWORD_toString(pNEheader->ne_flags, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Automatic data segment no."), DWORD_toString(pNEheader->ne_autodata, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Initial heap allocation"), DWORD_toString(pNEheader->ne_heap, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Initial stack allocation"), DWORD_toString(pNEheader->ne_stack));
			FillData(m_COFFItemsInfo, _T("Initial CS:IP setting"), DWORD_toString(pNEheader->ne_csip));
			FillData(m_COFFItemsInfo, _T("Initial SS:SP setting"), DWORD_toString(pNEheader->ne_sssp));
			FillData(m_COFFItemsInfo, _T("File segments"), DWORD_toString(pNEheader->ne_cseg));
			FillData(m_COFFItemsInfo, _T("Reference Table entries"), DWORD_toString(pNEheader->ne_cmod));
			FillData(m_COFFItemsInfo, _T("Non-resident name Size"), DWORD_toString(pNEheader->ne_cbnrestab));
			FillData(m_COFFItemsInfo, _T("Segment Offset"), DWORD_toString(pNEheader->ne_segtab));
			FillData(m_COFFItemsInfo, _T("Resource Offset"), DWORD_toString(pNEheader->ne_rsrctab));
			FillData(m_COFFItemsInfo, _T("Resident Name Offset"), DWORD_toString(pNEheader->ne_restab));
			FillData(m_COFFItemsInfo, _T("Module Reference Offset"), DWORD_toString(pNEheader->ne_modtab));
			FillData(m_COFFItemsInfo, _T("Imported Names Offset"), DWORD_toString(pNEheader->ne_imptab));
			FillData(m_COFFItemsInfo, _T("Non-resident Names Offset"), DWORD_toString(pNEheader->ne_nrestab));
			FillData(m_COFFItemsInfo, _T("Movable entries"), DWORD_toString(pNEheader->ne_cmovent));
			FillData(m_COFFItemsInfo, _T("No. of Segment alignment shift"), DWORD_toString(pNEheader->ne_align));
			FillData(m_COFFItemsInfo, _T("Resource segments"), DWORD_toString(pNEheader->ne_cres));
			FillData(m_COFFItemsInfo, _T("Target OS"), DWORD_toString(pNEheader->ne_exetyp));
			FillData(m_COFFItemsInfo, _T("Other EXE flags"), DWORD_toString(pNEheader->ne_flagsothers));
			FillData(m_COFFItemsInfo, _T("Return thunks Offset"), DWORD_toString(pNEheader->ne_pretthunks));
			FillData(m_COFFItemsInfo, _T("Segment ref. bytes Offset"), DWORD_toString(pNEheader->ne_psegrefbytes));
			FillData(m_COFFItemsInfo, _T("Min code swap area size"), DWORD_toString(pNEheader->ne_swaparea));
			FillData(m_COFFItemsInfo, _T("Expected Windows version"), DWORD_toString(pNEheader->ne_expver));
		}
		break;

	case PEReadWrite::LEHeader:
		{
			PIMAGE_VXD_HEADER pLEheader = m_PEReaderWriter.GetSecondaryHeader<PIMAGE_VXD_HEADER>();

			FillData(m_COFFItemsInfo, _T("Magic number"), DWORD_toString(pLEheader->e32_magic, Hexadecimal), _T("\"LE\""));
			FillData(m_COFFItemsInfo, _T("Byte ordering"), DWORD_toString(pLEheader->e32_border, Hexadecimal),
																						LEOrdering_toString(pLEheader->e32_border));
			FillData(m_COFFItemsInfo, _T("Word ordering"), DWORD_toString(pLEheader->e32_worder, Hexadecimal),
																						LEOrdering_toString(pLEheader->e32_worder));
			FillData(m_COFFItemsInfo, _T("EXE format level"), DWORD_toString(pLEheader->e32_level), _T("Only defined value is 0"));
			FillData(m_COFFItemsInfo, _T("CPU type"), DWORD_toString(pLEheader->e32_cpu, Hexadecimal),
																							LECPUtype_toString(pLEheader->e32_cpu));
			FillData(m_COFFItemsInfo, _T("OS type"), DWORD_toString(pLEheader->e32_os, Hexadecimal),
																							LEOStype_toString(pLEheader->e32_os));
			FillData(m_COFFItemsInfo, _T("Version"), DWORD_toString(pLEheader->e32_ver));
			FillData(m_COFFItemsInfo, _T("Flags"), DWORD_toString(pLEheader->e32_mflags, Hexadecimal),
																				LEModuletypeflags_toString(pLEheader->e32_mflags));
			FillData(m_COFFItemsInfo, _T("No. of pages"), DWORD_toString(pLEheader->e32_mpages));
			FillData(m_COFFItemsInfo, _T("Object no. for EIP"), DWORD_toString(pLEheader->e32_startobj));
			FillData(m_COFFItemsInfo, _T("EIP value"), DWORD_toString(pLEheader->e32_eip, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Object no. for ESP"), DWORD_toString(pLEheader->e32_stackobj));
			FillData(m_COFFItemsInfo, _T("ESP value"), DWORD_toString(pLEheader->e32_esp, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Page size"), DWORD_toString(pLEheader->e32_pagesize),
																						FormattedBytesSize(pLEheader->e32_pagesize));
			FillData(m_COFFItemsInfo, _T("Last page size"), DWORD_toString(pLEheader->e32_lastpagesize),
																					FormattedBytesSize(pLEheader->e32_lastpagesize));
			FillData(m_COFFItemsInfo, _T("Fixup size"), DWORD_toString(pLEheader->e32_fixupsize),
																						FormattedBytesSize(pLEheader->e32_fixupsize));
			FillData(m_COFFItemsInfo, _T("Fixup checksum"), DWORD_toString(pLEheader->e32_fixupsum, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Loader size"), DWORD_toString(pLEheader->e32_ldrsize),
																						FormattedBytesSize(pLEheader->e32_ldrsize));
			FillData(m_COFFItemsInfo, _T("Loader checksum"), DWORD_toString(pLEheader->e32_ldrsum, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Obj table offset"), DWORD_toString(pLEheader->e32_objtab, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("No. of objs"), DWORD_toString(pLEheader->e32_objcnt));
			FillData(m_COFFItemsInfo, _T("Obj pagemap offset"), DWORD_toString(pLEheader->e32_objmap, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Obj iter data map offset"), DWORD_toString(pLEheader->e32_itermap, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Resource offset"), DWORD_toString(pLEheader->e32_rsrctab, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Resource entries"), DWORD_toString(pLEheader->e32_rsrccnt));
			FillData(m_COFFItemsInfo, _T("Resident Name offset"), DWORD_toString(pLEheader->e32_restab, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Entry Table offset"), DWORD_toString(pLEheader->e32_enttab, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Module Directive offset"), DWORD_toString(pLEheader->e32_dirtab, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Directive entries"), DWORD_toString(pLEheader->e32_dircnt));
			FillData(m_COFFItemsInfo, _T("Fixup Page offset"), DWORD_toString(pLEheader->e32_fpagetab, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Fixup Record offset"), DWORD_toString(pLEheader->e32_frectab, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Import Offset"), DWORD_toString(pLEheader->e32_impmod, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Import entries"), DWORD_toString(pLEheader->e32_impmodcnt));
			FillData(m_COFFItemsInfo, _T("Import Proc offset"), DWORD_toString(pLEheader->e32_impproc, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Per-Page Checksum offset"), DWORD_toString(pLEheader->e32_pagesum, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Enum Data Pages offset"), DWORD_toString(pLEheader->e32_datapage, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("No. of preload pages"), DWORD_toString(pLEheader->e32_preload));
			FillData(m_COFFItemsInfo, _T("Non-resident Names offset"), DWORD_toString(pLEheader->e32_nrestab, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Non-resident Names Size"), DWORD_toString(pLEheader->e32_cbnrestab),
																						FormattedBytesSize(pLEheader->e32_cbnrestab));
			FillData(m_COFFItemsInfo, _T("Non-resident Name checksum"), DWORD_toString(pLEheader->e32_nressum, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Obj no. for automatic data obj"), DWORD_toString(pLEheader->e32_autodata));
			FillData(m_COFFItemsInfo, _T("Debugging info offset"), DWORD_toString(pLEheader->e32_debuginfo, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Debugging info Size"), DWORD_toString(pLEheader->e32_debuglen),
																						FormattedBytesSize(pLEheader->e32_debuglen));
			FillData(m_COFFItemsInfo, _T("Preload instance pages"), DWORD_toString(pLEheader->e32_instpreload));
			FillData(m_COFFItemsInfo, _T("Demandload instance pages"), DWORD_toString(pLEheader->e32_instdemand));
			FillData(m_COFFItemsInfo, _T("Size of heap"), DWORD_toString(pLEheader->e32_heapsize),
													FormattedBytesSize(pLEheader->e32_heapsize) + _T(", for 16-bit applications"));
			FillData(m_COFFItemsInfo, _T("Reserved 4 words"), QWORD_toString(*reinterpret_cast<ULONGLONG *>(pLEheader->e32_res3),
																						Hexadecimal), _T("Reserved, must be zero"));
			FillData(m_COFFItemsInfo, _T("Reserved 2 words"), DWORD_toString(*reinterpret_cast<DWORD *>(&pLEheader->e32_res3[8]),
																						Hexadecimal), _T("Reserved, must be zero"));
			FillData(m_COFFItemsInfo, _T("Windows resource offset"), DWORD_toString(pLEheader->e32_winresoff, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("Windows resource length"), DWORD_toString(pLEheader->e32_winreslen),
																						FormattedBytesSize(pLEheader->e32_winreslen));
			FillData(m_COFFItemsInfo, _T("Device ID"), DWORD_toString(pLEheader->e32_devid, Hexadecimal));
			FillData(m_COFFItemsInfo, _T("DDK version"), DWORD_toString(pLEheader->e32_ddkver));
		}
		break;

	default:
		LogError(_T("Unknown header found! Assuming corrupt PE header."));

	case PEReadWrite::PEHeader:
		{
			PIMAGE_NT_HEADERS pNTheaders = m_PEReaderWriter.GetSecondaryHeader<PIMAGE_NT_HEADERS>();
			PIMAGE_NT_HEADERS32 pNTheaders32 = (PIMAGE_NT_HEADERS32) pNTheaders;
			PIMAGE_NT_HEADERS64 pNTheaders64 = (PIMAGE_NT_HEADERS64) pNTheaders;
			PEReadWrite::PEType PEType = m_PEReaderWriter.GetPEType();

			// Prepare Tooltip for COFF listview
			switch (PEType)
			{
			case PEReadWrite::UnknownPEType:
			case PEReadWrite::PE32:
			case PEReadWrite::PE64:
				RTTI::GetTooltipInfo(m_COFFTooltipInfo, m_PEReaderWriter.GetPrimaryHeader<PIMAGE_DOS_HEADER>()->e_lfanew, RTTI::RTTI_COFF_HEADER);

				break;
				
			case PEReadWrite::ROM:
				RTTI::GetTooltipInfo(m_COFFTooltipInfo, m_PEReaderWriter.GetPrimaryHeader<PIMAGE_DOS_HEADER>()->e_lfanew, RTTI::RTTI_ROM_HEADER);
			}


			// Data in COFF header
			switch (PEType)
			{
			case PEReadWrite::UnknownPEType:
			case PEReadWrite::PE32:
			case PEReadWrite::PE64:
				FillData(m_COFFItemsInfo, _T("PE Signature"), DWORD_toString(pNTheaders->Signature, Hexadecimal), 
						(pNTheaders->Signature == IMAGE_NT_SIGNATURE ? _T("\"PE\\0\\0\"") : _T("Invalid signature, must be 'PE\\0\\0'")));

			case PEReadWrite::ROM:
				FillData(m_COFFItemsInfo, _T("Machine"), DWORD_toString(pNTheaders->FileHeader.Machine, Hexadecimal),
															MachineType_toString(pNTheaders->FileHeader.Machine));
				FillData(m_COFFItemsInfo, _T("No. of Sections"), DWORD_toString(pNTheaders->FileHeader.NumberOfSections),
										(pNTheaders->FileHeader.NumberOfSections == 0 ? _T("Invalid value, cannot be zero") : _T("")));
				FillData(m_COFFItemsInfo, _T("Time and Date"), DWORD_toString(pNTheaders->FileHeader.TimeDateStamp),
															TimeDateStamp_toString(pNTheaders->FileHeader.TimeDateStamp));
				FillData(m_COFFItemsInfo, _T("Symbol Table ptr"),
															DWORD_toString(pNTheaders->FileHeader.PointerToSymbolTable, Hexadecimal),
																									_T("Deprecated, must be zero"));
				FillData(m_COFFItemsInfo, _T("No. of Symbols"), DWORD_toString(pNTheaders->FileHeader.NumberOfSymbols),
																									_T("Deprecated, must be zero"));
				FillData(m_COFFItemsInfo, _T("Optional Header size"), DWORD_toString(pNTheaders->FileHeader.SizeOfOptionalHeader), 
																		FormattedBytesSize(pNTheaders->FileHeader.SizeOfOptionalHeader));
				FillData(m_COFFItemsInfo, _T("Characteristics"), DWORD_toString(pNTheaders->FileHeader.Characteristics, Hexadecimal), 
																ImageCharacteristics_toString(pNTheaders->FileHeader.Characteristics));
			}

			// Prepare Tooltip for Optional header
			switch (PEType)
			{
			case PEReadWrite::UnknownPEType:
			case PEReadWrite::PE32:
				RTTI::GetTooltipInfo(m_OptionalTooltipInfo, 0, RTTI::RTTI_IMAGE_OPTIONAL_HEADER32);

				break;

			case PEReadWrite::PE64:
				RTTI::GetTooltipInfo(m_OptionalTooltipInfo, 0, RTTI::RTTI_IMAGE_OPTIONAL_HEADER64);

				break;
				
			case PEReadWrite::ROM:
				RTTI::GetTooltipInfo(m_OptionalTooltipInfo, 0, RTTI::RTTI_IMAGE_ROM_OPTIONAL_HEADER);
			}

			// Data in Optional header
			FillData(m_OptionalItemsInfo, _T("Magic Number"), DWORD_toString(pNTheaders->OptionalHeader.Magic, Hexadecimal),
																					MagicNum_toString(pNTheaders->OptionalHeader.Magic));
			FillData(m_OptionalItemsInfo, _T("Linker Version"), VersionNums_toString(pNTheaders->OptionalHeader.MajorLinkerVersion, 
																						pNTheaders->OptionalHeader.MinorLinkerVersion));
			FillData(m_OptionalItemsInfo, _T("Code Section size"), DWORD_toString(pNTheaders->OptionalHeader.SizeOfCode),
																			FormattedBytesSize(pNTheaders->OptionalHeader.SizeOfCode));
			FillData(m_OptionalItemsInfo, _T("Initialized Data size"), DWORD_toString(pNTheaders->OptionalHeader.SizeOfInitializedData),
																FormattedBytesSize(pNTheaders->OptionalHeader.SizeOfInitializedData));
			FillData(m_OptionalItemsInfo, _T("Uninitialized Data size"),
																DWORD_toString(pNTheaders->OptionalHeader.SizeOfUninitializedData), 
																FormattedBytesSize(pNTheaders->OptionalHeader.SizeOfUninitializedData));
			FillData(m_OptionalItemsInfo, _T("Addr of Entry Point"),
															DWORD_toString(pNTheaders->OptionalHeader.AddressOfEntryPoint, Hexadecimal));
			FillData(m_OptionalItemsInfo, _T("Code Section Base Addr"),
																	DWORD_toString(pNTheaders->OptionalHeader.BaseOfCode, Hexadecimal));

			tstring DataDirectoryNames[] = { _T("Export Table"), _T("Import Table"), _T("Resource Table"),
											_T("Exception Table"), _T("Certificate Table"), _T("Base Reloc Table"),
											_T("Debug Data"), _T("Architecture Data"), _T("Global Ptr"), _T("TLS Table"),
											_T("Load Config Table"), _T("Bound Table"), _T("Import Addr Table"),
											_T("Delay Import Desc"), _T("CLR Runtime Header"), _T("Reserved Data") };
			DWORD Calculatedsum;

			switch (PEType)
			{
				case PEReadWrite::ROM:
					{
						PIMAGE_ROM_OPTIONAL_HEADER pROMheader = 
							(PIMAGE_ROM_OPTIONAL_HEADER) (m_PEReaderWriter.GetSecondaryHeader<UINT_PTR>() + sizeof(pNTheaders->FileHeader));

						FillData(m_OptionalItemsInfo, _T("Bss Section Base Addr"), QWORD_toString(pROMheader->BaseOfBss, Hexadecimal));
						FillData(m_OptionalItemsInfo, _T("Gpr mask"), DWORD_toString(pROMheader->GprMask, Hexadecimal));
						FillData(m_OptionalItemsInfo, _T("Cpr mask[0]"), DWORD_toString(pROMheader->CprMask[0], Hexadecimal));
						FillData(m_OptionalItemsInfo, _T("Cpr mask[1]"), DWORD_toString(pROMheader->CprMask[1], Hexadecimal));
						FillData(m_OptionalItemsInfo, _T("Cpr mask[2]"), DWORD_toString(pROMheader->CprMask[2], Hexadecimal));
						FillData(m_OptionalItemsInfo, _T("Cpr mask[3]"), DWORD_toString(pROMheader->CprMask[3], Hexadecimal));
						FillData(m_OptionalItemsInfo, _T("Gpr value"), DWORD_toString(pROMheader->GpValue));
					}

				break;

			case PEReadWrite::UnknownPEType:
			case PEReadWrite::PE32:
				FillData(m_OptionalItemsInfo, _T("Data Section Base Addr"),
															DWORD_toString(pNTheaders32->OptionalHeader.BaseOfData, Hexadecimal));

			case PEReadWrite::PE64:
				FillData(m_OptionalItemsInfo, _T("Image Base Addr"), PEType == PEReadWrite::PE32 ?
															DWORD_toString(pNTheaders32->OptionalHeader.ImageBase, Hexadecimal) :
															QWORD_toString(pNTheaders64->OptionalHeader.ImageBase, Hexadecimal));
			
				FillData(m_OptionalItemsInfo, _T("Section Alignment"),
														DWORD_toString(pNTheaders->OptionalHeader.SectionAlignment, Hexadecimal));
				FillData(m_OptionalItemsInfo, _T("File Alignment"), DWORD_toString(pNTheaders->OptionalHeader.FileAlignment, Hexadecimal));
				FillData(m_OptionalItemsInfo, _T("Min OS Version"), 
													VersionNums_toString(pNTheaders->OptionalHeader.MajorOperatingSystemVersion,
																		pNTheaders->OptionalHeader.MinorOperatingSystemVersion), 
													OSIdString(pNTheaders->OptionalHeader.MajorOperatingSystemVersion,
																		pNTheaders->OptionalHeader.MinorOperatingSystemVersion));
				FillData(m_OptionalItemsInfo, _T("Image Version"), VersionNums_toString(pNTheaders->OptionalHeader.MajorImageVersion,
																					pNTheaders->OptionalHeader.MinorImageVersion));
				FillData(m_OptionalItemsInfo, _T("Subsystem Version"),
															VersionNums_toString(pNTheaders->OptionalHeader.MajorSubsystemVersion,
																				pNTheaders->OptionalHeader.MinorSubsystemVersion));
				FillData(m_OptionalItemsInfo, _T("Win32 Version Value"), DWORD_toString(pNTheaders->OptionalHeader.Win32VersionValue),
																									_T("Reserved, must be zero"));
				FillData(m_OptionalItemsInfo, _T("Size of Image"), DWORD_toString(pNTheaders->OptionalHeader.SizeOfImage),
																		FormattedBytesSize(pNTheaders->OptionalHeader.SizeOfImage));
				FillData(m_OptionalItemsInfo, _T("Size of Headers"),DWORD_toString(pNTheaders->OptionalHeader.SizeOfHeaders),
																		FormattedBytesSize(pNTheaders->OptionalHeader.SizeOfHeaders));
				FillData(m_OptionalItemsInfo, _T("Checksum"), DWORD_toString(pNTheaders->OptionalHeader.CheckSum, Hexadecimal),
																(m_PEReaderWriter.VerifyPEChecksum(&Calculatedsum) ? _T("OK") :
													_T("Zero/Invalid, should be ") + DWORD_toString(Calculatedsum, Hexadecimal)));
				FillData(m_OptionalItemsInfo, _T("Required Subsystem"), DWORD_toString(pNTheaders->OptionalHeader.Subsystem),
																		SubsystemID_toString(pNTheaders->OptionalHeader.Subsystem));
				FillData(m_OptionalItemsInfo, _T("DLL Characteristics"),
														DWORD_toString(pNTheaders->OptionalHeader.DllCharacteristics, Hexadecimal), 
														DllCharacteristics_toString(pNTheaders->OptionalHeader.DllCharacteristics));

				switch (PEType)
				{
				case PEReadWrite::UnknownPEType:
				case PEReadWrite::PE32:
					FillData(m_OptionalItemsInfo, _T("Reserved Stack size"),
																	DWORD_toString(pNTheaders32->OptionalHeader.SizeOfStackReserve),
																	FormattedBytesSize(pNTheaders32->OptionalHeader.SizeOfStackReserve));
					FillData(m_OptionalItemsInfo, _T("Committed Stack size"),
																	DWORD_toString(pNTheaders32->OptionalHeader.SizeOfStackCommit),
																	FormattedBytesSize(pNTheaders32->OptionalHeader.SizeOfStackCommit));
					FillData(m_OptionalItemsInfo, _T("Reserved Heap size"),
																	DWORD_toString(pNTheaders32->OptionalHeader.SizeOfHeapReserve),
																	FormattedBytesSize(pNTheaders32->OptionalHeader.SizeOfHeapReserve));
					FillData(m_OptionalItemsInfo, _T("Committed Heap size"),
																	DWORD_toString(pNTheaders32->OptionalHeader.SizeOfHeapCommit),
																	FormattedBytesSize(pNTheaders32->OptionalHeader.SizeOfHeapCommit));
					FillData(m_OptionalItemsInfo, _T("Loader Flags"),
																DWORD_toString(pNTheaders32->OptionalHeader.LoaderFlags, Hexadecimal),
																										_T("Reserved, must be zero"));
					FillData(m_OptionalItemsInfo, _T("No. of Data Dirs"),
																	DWORD_toString(pNTheaders32->OptionalHeader.NumberOfRvaAndSizes));

					for (DWORD i = 0; i < pNTheaders32->OptionalHeader.NumberOfRvaAndSizes; i++)
					{
						FillData(m_OptionalItemsInfo, DataDirectoryNames[i] + (i != IMAGE_DIRECTORY_ENTRY_SECURITY ? _T(" RVA") : _T(" Offset")),
													DWORD_toString(pNTheaders32->OptionalHeader.DataDirectory[i].VirtualAddress, Hexadecimal),
						i == IMAGE_DIRECTORY_ENTRY_GLOBALPTR  && pNTheaders32->OptionalHeader.DataDirectory[i].VirtualAddress != 0? _T("Value: ") +
																		DWORD_toString(m_PEReaderWriter.GetGlobalPtrData()) : _T(""));
						FillData(m_OptionalItemsInfo, DataDirectoryNames[i] + _T(" Size"),
											DWORD_toString(pNTheaders32->OptionalHeader.DataDirectory[i].Size),
											FormattedBytesSize(pNTheaders32->OptionalHeader.DataDirectory[i].Size));
					}

					break;

				case PEReadWrite::PE64:
					FillData(m_OptionalItemsInfo, _T("Reserved Stack size"),
																QWORD_toString(pNTheaders64->OptionalHeader.SizeOfStackReserve),
																FormattedBytesSize(pNTheaders64->OptionalHeader.SizeOfStackReserve));
					FillData(m_OptionalItemsInfo, _T("Committed Stack size"),
																QWORD_toString(pNTheaders64->OptionalHeader.SizeOfStackCommit),
																FormattedBytesSize(pNTheaders64->OptionalHeader.SizeOfStackCommit));
					FillData(m_OptionalItemsInfo, _T("Reserved Heap size"),
																QWORD_toString(pNTheaders64->OptionalHeader.SizeOfHeapReserve),
																FormattedBytesSize(pNTheaders64->OptionalHeader.SizeOfHeapReserve));
					FillData(m_OptionalItemsInfo, _T("Committed Heap size"),
																QWORD_toString(pNTheaders64->OptionalHeader.SizeOfHeapCommit),
																FormattedBytesSize(pNTheaders64->OptionalHeader.SizeOfHeapCommit));
					FillData(m_OptionalItemsInfo, _T("Loader Flags"),
																DWORD_toString(pNTheaders64->OptionalHeader.LoaderFlags, Hexadecimal),
																_T("Reserved, must be zero"));
					FillData(m_OptionalItemsInfo, _T("No. of Data Dirs"),
																DWORD_toString(pNTheaders64->OptionalHeader.NumberOfRvaAndSizes));

					for (DWORD i = 0; i < pNTheaders64->OptionalHeader.NumberOfRvaAndSizes; i++)
					{
						FillData(m_OptionalItemsInfo, DataDirectoryNames[i] + (i != IMAGE_DIRECTORY_ENTRY_SECURITY ? _T(" RVA") : _T(" Offset")),
													DWORD_toString(pNTheaders64->OptionalHeader.DataDirectory[i].VirtualAddress, Hexadecimal),
					i == IMAGE_DIRECTORY_ENTRY_GLOBALPTR  && pNTheaders64->OptionalHeader.DataDirectory[i].VirtualAddress != 0 ? _T("Value: ") +
																		DWORD_toString(m_PEReaderWriter.GetGlobalPtrData()) : _T(""));
						FillData(m_OptionalItemsInfo, DataDirectoryNames[i] + _T(" Size"),
											DWORD_toString(pNTheaders64->OptionalHeader.DataDirectory[i].Size),
											FormattedBytesSize(pNTheaders64->OptionalHeader.DataDirectory[i].Size));
					}
				}
			}
		}
	}
	
	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = GenericColumnText[i];
		ListView_InsertColumn(m_hListViewCOFFHeader, i, &column);
		ListView_InsertColumn(m_hListViewOptionalHeader, i, &column);
	}

	// Insert ListView items for COFF data list view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < m_COFFItemsInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) m_COFFItemsInfo[i].szText.c_str();
		ListView_InsertItem(m_hListViewCOFFHeader, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) m_COFFItemsInfo[i].szData.c_str();
		ListView_SetItem(m_hListViewCOFFHeader, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) m_COFFItemsInfo[i].szComments.c_str();
		ListView_SetItem(m_hListViewCOFFHeader, &item);
	}

	// Insert ListView items for Optional data list view
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < m_OptionalItemsInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) m_OptionalItemsInfo[i].szText.c_str();
		ListView_InsertItem(m_hListViewOptionalHeader, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) m_OptionalItemsInfo[i].szData.c_str();
		ListView_SetItem(m_hListViewOptionalHeader, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) m_OptionalItemsInfo[i].szComments.c_str();
		ListView_SetItem(m_hListViewOptionalHeader, &item);
	}

	// Resize columns
	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
	{
		ListView_SetColumnWidth(m_hListViewCOFFHeader, i,
											i == GetArraySize(GenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(m_hListViewOptionalHeader, i,
											i == GetArraySize(GenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);
	}
}

tstring PropertyPageHandler_PEHeaders::lstCOFFHeader_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(m_COFFTooltipInfo, Index);
}

tstring PropertyPageHandler_PEHeaders::lstOptionalHeader_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(m_OptionalTooltipInfo, Index);
}

void PropertyPageHandler_PEHeaders::lstCOFFHeader_OnContextMenu(LONG x, LONG y, int Index)
{
	return Generic_OnContextMenu(m_COFFTooltipInfo, m_COFFItemsInfo, x, y, Index);
}

void PropertyPageHandler_PEHeaders::lstOptionalHeader_OnContextMenu(LONG x, LONG y, int Index)
{
	return Generic_OnContextMenu(m_OptionalTooltipInfo, m_OptionalItemsInfo, x, y, Index);
}