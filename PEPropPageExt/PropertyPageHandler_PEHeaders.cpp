#include "PropertyPageHandler.h"
#include <array>


PropertyPageHandler_PEHeaders::PropertyPageHandler_PEHeaders(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, std::ref(PEReaderWriter))
{
	m_hListViewCOFFHeader = GetDlgItem(m_hWnd, IDC_LISTCOFFHEADERDATA);
	m_hListViewOptionalHeader = GetDlgItem(m_hWnd, IDC_LISTOPTIONALHEADERDATA);

	// Setup controls with layout manager
	m_LayoutManager.AddChildConstraint(IDC_LISTCOFFHEADERDATA, CWA_LEFTRIGHT, CWA_TOP);
	m_LayoutManager.AddChildConstraint(IDC_LISTOPTIONALHEADERDATA, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

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
	int err;
	switch (m_PEReaderWriter.getSecondaryHeaderType())
	{
		case PEReadWrite::HeaderType::Unknown:
		{
			LogError(L"ERROR: PE signature not found. File is not valid.", true);
			return;
		}

		case PEReadWrite::HeaderType::NE:
		{
			PIMAGE_OS2_HEADER pNEheader;
			err = m_PEReaderWriter.getSecondaryHeader(std::ref(pNEheader));
			if (err)
			{
				LogError(L"ERROR: NE header is incomplete. File is not valid.", true);
				return;
			}

			m_COFFItemsInfo =
			{
				{ L"OS/2 Signature", DWORD_toString(pNEheader->ne_magic, Hexadecimal), L"\"NE\"" },
				{ L"Version no.", VersionNums_toString(pNEheader->ne_ver, pNEheader->ne_rev) },
				{ L"Entry Table offset", DWORD_toString(pNEheader->ne_enttab, Hexadecimal) },
				{
					L"Entry Table Size",
					DWORD_toString(pNEheader->ne_cbenttab),
					FormattedBytesSize(pNEheader->ne_cbenttab)
				},
				{ L"Checksum", DWORD_toString(pNEheader->ne_crc) },
				{ L"Flags", DWORD_toString(pNEheader->ne_flags, Hexadecimal) },
				{ L"Automatic data segment no.", DWORD_toString(pNEheader->ne_autodata, Hexadecimal) },
				{ L"Initial heap allocation", DWORD_toString(pNEheader->ne_heap, Hexadecimal) },
				{ L"Initial stack allocation", DWORD_toString(pNEheader->ne_stack) },
				{ L"Initial CS:IP setting", DWORD_toString(pNEheader->ne_csip) },
				{ L"Initial SS:SP setting", DWORD_toString(pNEheader->ne_sssp) },
				{ L"File segments", DWORD_toString(pNEheader->ne_cseg) },
				{ L"Reference Table entries", DWORD_toString(pNEheader->ne_cmod) },
				{ L"Non-resident name Size", DWORD_toString(pNEheader->ne_cbnrestab) },
				{ L"Segment Offset", DWORD_toString(pNEheader->ne_segtab) },
				{ L"Resource Offset", DWORD_toString(pNEheader->ne_rsrctab) },
				{ L"Resident Name Offset", DWORD_toString(pNEheader->ne_restab) },
				{ L"Module Reference Offset", DWORD_toString(pNEheader->ne_modtab) },
				{ L"Imported Names Offset", DWORD_toString(pNEheader->ne_imptab) },
				{ L"Non-resident Names Offset", DWORD_toString(pNEheader->ne_nrestab) },
				{ L"Movable entries", DWORD_toString(pNEheader->ne_cmovent) },
				{ L"No. of Segment alignment shift", DWORD_toString(pNEheader->ne_align) },
				{ L"Resource segments", DWORD_toString(pNEheader->ne_cres) },
				{ L"Target OS", DWORD_toString(pNEheader->ne_exetyp) },
				{ L"Other EXE flags", DWORD_toString(pNEheader->ne_flagsothers) },
				{ L"Return thunks Offset", DWORD_toString(pNEheader->ne_pretthunks) },
				{ L"Segment ref. bytes Offset", DWORD_toString(pNEheader->ne_psegrefbytes) },
				{ L"Min code swap area size", DWORD_toString(pNEheader->ne_swaparea) },
				{ L"Expected Windows version", DWORD_toString(pNEheader->ne_expver) }
			};
		}
		break;

		case PEReadWrite::HeaderType::LE:
		{
			PIMAGE_VXD_HEADER pLEheader;
			err = m_PEReaderWriter.getSecondaryHeader(std::ref(pLEheader));
			if (err)
			{
				LogError(L"ERROR: LE is incomplete. File is not valid.", true);
				return;
			}

			m_COFFItemsInfo =
			{
				{
					L"Magic number",
					DWORD_toString(pLEheader->e32_magic, Hexadecimal),
					L"\"LE\""
				},
				{
					L"Byte ordering",
					DWORD_toString(pLEheader->e32_border, Hexadecimal),
					LEOrdering_toString(pLEheader->e32_border)
				},
				{
					L"Word ordering",
					DWORD_toString(pLEheader->e32_worder, Hexadecimal),
					LEOrdering_toString(pLEheader->e32_worder)
				},
				{
					L"EXE format level",
					DWORD_toString(pLEheader->e32_level),
					L"Only defined value is 0" },
				{
					L"CPU type",
					DWORD_toString(pLEheader->e32_cpu, Hexadecimal),
					LECPUtype_toString(pLEheader->e32_cpu)
				},
				{
					L"OS type",
					DWORD_toString(pLEheader->e32_os, Hexadecimal),
					LEOStype_toString(pLEheader->e32_os)
				},
				{ L"Version", DWORD_toString(pLEheader->e32_ver) },
				{
					L"Flags",
					DWORD_toString(pLEheader->e32_mflags, Hexadecimal),
					LEModuletypeflags_toString(pLEheader->e32_mflags)
				},
				{ L"No. of pages", DWORD_toString(pLEheader->e32_mpages) },
				{ L"Object no. for EIP", DWORD_toString(pLEheader->e32_startobj) },
				{ L"EIP value", DWORD_toString(pLEheader->e32_eip, Hexadecimal) },
				{ L"Object no. for ESP", DWORD_toString(pLEheader->e32_stackobj) },
				{ L"ESP value", DWORD_toString(pLEheader->e32_esp, Hexadecimal) },
				{
					L"Page size",
					DWORD_toString(pLEheader->e32_pagesize),
					FormattedBytesSize(pLEheader->e32_pagesize)
				},
				{
					L"Last page size",
					DWORD_toString(pLEheader->e32_lastpagesize),
					FormattedBytesSize(pLEheader->e32_lastpagesize)
				},
				{
					L"Fixup size",
					DWORD_toString(pLEheader->e32_fixupsize),
					FormattedBytesSize(pLEheader->e32_fixupsize)
				},
				{
					L"Fixup checksum",
					DWORD_toString(pLEheader->e32_fixupsum, Hexadecimal)
				},
				{
					L"Loader size",
					DWORD_toString(pLEheader->e32_ldrsize),
					FormattedBytesSize(pLEheader->e32_ldrsize)
				},
				{ L"Loader checksum", DWORD_toString(pLEheader->e32_ldrsum, Hexadecimal) },
				{ L"Obj table offset", DWORD_toString(pLEheader->e32_objtab, Hexadecimal) },
				{ L"No. of objs", DWORD_toString(pLEheader->e32_objcnt) },
				{ L"Obj pagemap offset", DWORD_toString(pLEheader->e32_objmap, Hexadecimal) },
				{ L"Obj iter data map offset", DWORD_toString(pLEheader->e32_itermap, Hexadecimal) },
				{ L"Resource offset", DWORD_toString(pLEheader->e32_rsrctab, Hexadecimal) },
				{ L"Resource entries", DWORD_toString(pLEheader->e32_rsrccnt) },
				{ L"Resident Name offset", DWORD_toString(pLEheader->e32_restab, Hexadecimal) },
				{ L"Entry Table offset", DWORD_toString(pLEheader->e32_enttab, Hexadecimal) },
				{ L"Module Directive offset", DWORD_toString(pLEheader->e32_dirtab, Hexadecimal) },
				{ L"Directive entries", DWORD_toString(pLEheader->e32_dircnt) },
				{ L"Fixup Page offset", DWORD_toString(pLEheader->e32_fpagetab, Hexadecimal) },
				{ L"Fixup Record offset", DWORD_toString(pLEheader->e32_frectab, Hexadecimal) },
				{ L"Import Offset", DWORD_toString(pLEheader->e32_impmod, Hexadecimal) },
				{ L"Import entries", DWORD_toString(pLEheader->e32_impmodcnt) },
				{ L"Import Proc offset", DWORD_toString(pLEheader->e32_impproc, Hexadecimal) },
				{ L"Per-Page Checksum offset", DWORD_toString(pLEheader->e32_pagesum, Hexadecimal) },
				{ L"Enum Data Pages offset", DWORD_toString(pLEheader->e32_datapage, Hexadecimal) },
				{ L"No. of preload pages", DWORD_toString(pLEheader->e32_preload) },
				{ L"Non-resident Names offset", DWORD_toString(pLEheader->e32_nrestab, Hexadecimal) },
				{
					L"Non-resident Names Size",
					DWORD_toString(pLEheader->e32_cbnrestab),
					FormattedBytesSize(pLEheader->e32_cbnrestab)
				},
				{ L"Non-resident Name checksum", DWORD_toString(pLEheader->e32_nressum, Hexadecimal) },
				{ L"Obj no. for automatic data obj", DWORD_toString(pLEheader->e32_autodata) },
				{ L"Debugging info offset", DWORD_toString(pLEheader->e32_debuginfo, Hexadecimal) },
				{
					L"Debugging info Size",
					DWORD_toString(pLEheader->e32_debuglen),
					FormattedBytesSize(pLEheader->e32_debuglen)
				},
				{ L"Preload instance pages", DWORD_toString(pLEheader->e32_instpreload) },
				{ L"Demandload instance pages", DWORD_toString(pLEheader->e32_instdemand) },
				{
					L"Size of heap",
					DWORD_toString(pLEheader->e32_heapsize),
					FormattedBytesSize(pLEheader->e32_heapsize) + L", for 16-bit applications"
				},
				{
					L"Reserved 4 words",
					QWORD_toString(*PULONGLONG(pLEheader->e32_res3), Hexadecimal),
					L"Reserved, must be zero"
				},
				{
					L"Reserved 2 words",
					DWORD_toString(*LPDWORD(&pLEheader->e32_res3[8]), Hexadecimal),
					L"Reserved, must be zero"
				},
				{
					L"Windows resource offset",
					DWORD_toString(pLEheader->e32_winresoff, Hexadecimal)
				},
				{
					L"Windows resource length",
					DWORD_toString(pLEheader->e32_winreslen),
					FormattedBytesSize(pLEheader->e32_winreslen)
				},
				{ L"Device ID", DWORD_toString(pLEheader->e32_devid, Hexadecimal) },
				{ L"DDK version", DWORD_toString(pLEheader->e32_ddkver) }
			};
		}
		break;

		case PEReadWrite::HeaderType::PE:
		{
			PIMAGE_DOS_HEADER pDOSheader;
			err = m_PEReaderWriter.getPrimaryHeader(std::ref(pDOSheader));
			assert(err == PEReadWrite::noErr);

			// NOTE: The following union removes need for casting processing when processing 32-bit and 64-bit header data
			union
			{
				PIMAGE_NT_HEADERS pNTheaders;			// Use this pointer when refering to data common to both 32/64 headers
				PIMAGE_NT_HEADERS32 pNTheaders32;		// Use this pointer when refering to 32-bit header specific section
				PIMAGE_NT_HEADERS64 pNTheaders64;		// Use this pointer when refering to 64-bit header specific section
			};
			
			PEReadWrite::PEType PEType = m_PEReaderWriter.getPEType();
			err = m_PEReaderWriter.getSecondaryHeader(std::ref(pNTheaders));
			if (err)
			{
				LogError(L"ERROR: Incomplete 'IMAGE_PE_HEADERS' data. File is not valid.", true);
				return;
			}

			// Prepare Tooltip for COFF listview
			switch (PEType)
			{
				case PEReadWrite::PEType::PE32:
				case PEReadWrite::PEType::PE64:
					RTTI::GetTooltipInfo(m_COFFTooltipInfo, pDOSheader->e_lfanew, RTTI::RTTI_COFF_HEADER);
					break;
				
				case PEReadWrite::PEType::ROM:
					RTTI::GetTooltipInfo(m_COFFTooltipInfo, pDOSheader->e_lfanew, RTTI::RTTI_ROM_HEADER);
					break;

				default:
					break;
			}

			// Data in COFF header
			switch (PEType)
			{
				case PEReadWrite::PEType::PE32:
				case PEReadWrite::PEType::PE64:
				default:
					m_COFFItemsInfo =
					{
						{
							L"PE Signature",
							DWORD_toString(pNTheaders->Signature, Hexadecimal),
							(pNTheaders->Signature == IMAGE_NT_SIGNATURE ? L"\"PE\\0\\0\"" : L"Invalid signature, must be 'PE\\0\\0'")
						}
					};

				case PEReadWrite::PEType::ROM:
					m_COFFItemsInfo.insert(m_COFFItemsInfo.cend(),
					{
						{
							L"Machine",
							DWORD_toString(pNTheaders->FileHeader.Machine, Hexadecimal),
							MachineType_toString(pNTheaders->FileHeader.Machine)
						},
						{
							L"No. of Sections",
							DWORD_toString(pNTheaders->FileHeader.NumberOfSections),
							(pNTheaders->FileHeader.NumberOfSections == 0 ? L"Invalid value, cannot be zero" : L"")
						},
						{
							L"Time and Date",
							DWORD_toString(pNTheaders->FileHeader.TimeDateStamp),
							TimeDateStamp_toString(pNTheaders->FileHeader.TimeDateStamp)
						},
						{
							L"Symbol Table ptr",
							DWORD_toString(pNTheaders->FileHeader.PointerToSymbolTable, Hexadecimal),
							L"Deprecated, must be zero"
						},
						{
							L"No. of Symbols",
							DWORD_toString(pNTheaders->FileHeader.NumberOfSymbols),
							L"Deprecated, must be zero"
						},
						{
							L"Optional Header size",
							DWORD_toString(pNTheaders->FileHeader.SizeOfOptionalHeader),
							FormattedBytesSize(pNTheaders->FileHeader.SizeOfOptionalHeader)
						},
						{
							L"Characteristics",
							DWORD_toString(pNTheaders->FileHeader.Characteristics, Hexadecimal),
							ImageCharacteristics_toString(pNTheaders->FileHeader.Characteristics)
						}
					});
					break;
			}

			// Prepare Tooltip for Optional header
			switch (PEType)
			{
				case PEReadWrite::PEType::PE32:
					RTTI::GetTooltipInfo(m_OptionalTooltipInfo, pDOSheader->e_lfanew + sizeof(IMAGE_NT_HEADERS32), RTTI::RTTI_IMAGE_OPTIONAL_HEADER32);
					break;

				case PEReadWrite::PEType::PE64:
					RTTI::GetTooltipInfo(m_OptionalTooltipInfo, pDOSheader->e_lfanew + sizeof(IMAGE_NT_HEADERS64), RTTI::RTTI_IMAGE_OPTIONAL_HEADER64);
					break;
				
				case PEReadWrite::PEType::ROM:
					RTTI::GetTooltipInfo(m_OptionalTooltipInfo, pDOSheader->e_lfanew + sizeof(IMAGE_ROM_HEADERS), RTTI::RTTI_IMAGE_ROM_OPTIONAL_HEADER);
					break;

				default:
					break;
			}

			// Data in Optional header
			if (PEType == PEReadWrite::PEType::Unknown)
				break;			// If the Portable Executable header signature is not valid, proceed no further

			m_OptionalItemsInfo =
			{
				{
					L"Magic Number",
					DWORD_toString(pNTheaders->OptionalHeader.Magic, Hexadecimal),
					MagicNum_toString(pNTheaders->OptionalHeader.Magic)
				},
				{
					L"Linker Version",
					VersionNums_toString(pNTheaders->OptionalHeader.MajorLinkerVersion,
										 pNTheaders->OptionalHeader.MinorLinkerVersion)
				},
				{
					L"Code Section size",
					DWORD_toString(pNTheaders->OptionalHeader.SizeOfCode),
					FormattedBytesSize(pNTheaders->OptionalHeader.SizeOfCode)
				},
				{
					L"Initialized Data size",
					DWORD_toString(pNTheaders->OptionalHeader.SizeOfInitializedData),
					FormattedBytesSize(pNTheaders->OptionalHeader.SizeOfInitializedData)
				},
				{
					L"Uninitialized Data size",
					DWORD_toString(pNTheaders->OptionalHeader.SizeOfUninitializedData),
					FormattedBytesSize(pNTheaders->OptionalHeader.SizeOfUninitializedData)
				},
				{
					L"Addr of Entry Point",
					DWORD_toString(pNTheaders->OptionalHeader.AddressOfEntryPoint, Hexadecimal),
					(L'\"' +
					m_PEReaderWriter.getContainingSectionName(pNTheaders->OptionalHeader.AddressOfEntryPoint) +
					L"\" Section")
				},
				{
					L"Code Section Base Addr",
					DWORD_toString(pNTheaders->OptionalHeader.BaseOfCode, Hexadecimal),
					(L'\"' +
					m_PEReaderWriter.getContainingSectionName(pNTheaders->OptionalHeader.BaseOfCode) +
					L"\" Section")
				}
			};

			const wstring DataDirectoryNames[] =
			{
				L"Export Table", L"Import Table", L"Resource Table",
				L"Exception Table", L"Certificate Table", L"Base Reloc Table",
				L"Debug Data", L"Architecture Data", L"Global Ptr", L"TLS Table",
				L"Load Config Table", L"Bound Table", L"Import Addr Table",
				L"Delay Import Desc", L"CLR Runtime Header", L"Reserved Data"
			};
			DWORD CalculatedSum = 0;

			switch (PEType)
			{
				case PEReadWrite::PEType::ROM:
				{
					PIMAGE_ROM_OPTIONAL_HEADER pROMheader;
					err = m_PEReaderWriter.getROMHeader(std::ref(pROMheader));
					if (err)
					{
						LogError(L"ERROR: 'IMAGE_ROM_OPTIONAL_HEADER' data is incomplete. File is not vaild.", true);
						break;
					}

					m_OptionalItemsInfo.insert(m_OptionalItemsInfo.cend(),
					{
						{ L"Bss Section Base Addr", QWORD_toString(pROMheader->BaseOfBss, Hexadecimal) },
						{ L"Gpr mask", DWORD_toString(pROMheader->GprMask, Hexadecimal) },
						{ L"Cpr mask[0]", DWORD_toString(pROMheader->CprMask[0], Hexadecimal) },
						{ L"Cpr mask[1]", DWORD_toString(pROMheader->CprMask[1], Hexadecimal) },
						{ L"Cpr mask[2]", DWORD_toString(pROMheader->CprMask[2], Hexadecimal) },
						{ L"Cpr mask[3]", DWORD_toString(pROMheader->CprMask[3], Hexadecimal) },
						{ L"Gpr value", DWORD_toString(pROMheader->GpValue) }
					});
				}
				break;

				case PEReadWrite::PEType::PE32:
					m_OptionalItemsInfo.insert(m_OptionalItemsInfo.cend(),
					{
						{
							L"Data Section Base Addr",
							DWORD_toString(pNTheaders32->OptionalHeader.BaseOfData, Hexadecimal),
							(L'\"' +
							m_PEReaderWriter.getContainingSectionName(pNTheaders32->OptionalHeader.BaseOfData) +
							L"\" Section")
						}
					});

				case PEReadWrite::PEType::PE64:
					m_OptionalItemsInfo.insert(m_OptionalItemsInfo.cend(),
					{
						{
							L"Image Base Addr",
							(PEType == PEReadWrite::PEType::PE32 ? DWORD_toString(pNTheaders32->OptionalHeader.ImageBase, Hexadecimal) :
																  QWORD_toString(pNTheaders64->OptionalHeader.ImageBase, Hexadecimal))
						},
						{
							L"Section Alignment",
							DWORD_toString(pNTheaders->OptionalHeader.SectionAlignment, Hexadecimal)
						},
						{
							L"File Alignment",
							DWORD_toString(pNTheaders->OptionalHeader.FileAlignment, Hexadecimal)
						},
						{
							L"Min OS Version",
							VersionNums_toString(pNTheaders->OptionalHeader.MajorOperatingSystemVersion,
												 pNTheaders->OptionalHeader.MinorOperatingSystemVersion),
							OSId_toString(pNTheaders->OptionalHeader.MajorOperatingSystemVersion,
										  pNTheaders->OptionalHeader.MinorOperatingSystemVersion)
						},
						{
							L"Image Version",
							VersionNums_toString(pNTheaders->OptionalHeader.MajorImageVersion,
												 pNTheaders->OptionalHeader.MinorImageVersion)
						},
						{
							L"Subsystem Version",
							VersionNums_toString(pNTheaders->OptionalHeader.MajorSubsystemVersion,
												 pNTheaders->OptionalHeader.MinorSubsystemVersion)
						},
						{
							L"Win32 Version Value",
							DWORD_toString(pNTheaders->OptionalHeader.Win32VersionValue),
							L"Reserved, must be zero"
						},
						{
							L"Size of Image",
							DWORD_toString(pNTheaders->OptionalHeader.SizeOfImage),
							FormattedBytesSize(pNTheaders->OptionalHeader.SizeOfImage)
						},
						{
							L"Size of Headers",
							DWORD_toString(pNTheaders->OptionalHeader.SizeOfHeaders),
							FormattedBytesSize(pNTheaders->OptionalHeader.SizeOfHeaders)
						},
						{
							L"Checksum",
							DWORD_toString(pNTheaders->OptionalHeader.CheckSum, Hexadecimal),
							(m_PEReaderWriter.verifyPEChecksum(CalculatedSum) ? L"OK" :
																				(L"Zero/Invalid, should be " +
																				DWORD_toString(CalculatedSum, Hexadecimal)))
						},
						{
							L"Required Subsystem",
							DWORD_toString(pNTheaders->OptionalHeader.Subsystem),
							SubsystemID_toString(pNTheaders->OptionalHeader.Subsystem)
						},
						{
							L"DLL Characteristics",
							DWORD_toString(pNTheaders->OptionalHeader.DllCharacteristics, Hexadecimal),
							DllCharacteristics_toString(pNTheaders->OptionalHeader.DllCharacteristics)
						}
					});

					switch (PEType)
					{
						case PEReadWrite::PEType::PE32:
							m_OptionalItemsInfo.insert(m_OptionalItemsInfo.cend(),
							{
								{
									L"Reserved Stack size",
									DWORD_toString(pNTheaders32->OptionalHeader.SizeOfStackReserve),
									FormattedBytesSize(pNTheaders32->OptionalHeader.SizeOfStackReserve)
								},
								{
									L"Committed Stack size",
									DWORD_toString(pNTheaders32->OptionalHeader.SizeOfStackCommit),
									FormattedBytesSize(pNTheaders32->OptionalHeader.SizeOfStackCommit)
								},
								{
									L"Reserved Heap size",
									DWORD_toString(pNTheaders32->OptionalHeader.SizeOfHeapReserve),
									FormattedBytesSize(pNTheaders32->OptionalHeader.SizeOfHeapReserve)
								},
								{
									L"Committed Heap size",
									DWORD_toString(pNTheaders32->OptionalHeader.SizeOfHeapCommit),
									FormattedBytesSize(pNTheaders32->OptionalHeader.SizeOfHeapCommit)
								},
								{
									L"Loader Flags",
									DWORD_toString(pNTheaders32->OptionalHeader.LoaderFlags, Hexadecimal),
									L"Reserved, must be zero"
								},
								{
									L"No. of Data Dirs",
									DWORD_toString(pNTheaders32->OptionalHeader.NumberOfRvaAndSizes),
									(m_PEReaderWriter.hasInvalidNoOfDataDirectories() ?	L"Cannot be greater than 16, ignored" : L"")
								}
							});
							
							break;

						case PEReadWrite::PEType::PE64:
							m_OptionalItemsInfo.insert(m_OptionalItemsInfo.cend(),
							{
								{
									L"Reserved Stack size",
									QWORD_toString(pNTheaders64->OptionalHeader.SizeOfStackReserve),
									FormattedBytesSize(pNTheaders64->OptionalHeader.SizeOfStackReserve)
								},
								{
									L"Committed Stack size",
									QWORD_toString(pNTheaders64->OptionalHeader.SizeOfStackCommit),
									FormattedBytesSize(pNTheaders64->OptionalHeader.SizeOfStackCommit)
								},
								{
									L"Reserved Heap size",
									QWORD_toString(pNTheaders64->OptionalHeader.SizeOfHeapReserve),
									FormattedBytesSize(pNTheaders64->OptionalHeader.SizeOfHeapReserve)
								},
								{
									L"Committed Heap size",
									QWORD_toString(pNTheaders64->OptionalHeader.SizeOfHeapCommit),
									FormattedBytesSize(pNTheaders64->OptionalHeader.SizeOfHeapCommit)
								},
								{
									L"Loader Flags",
									DWORD_toString(pNTheaders64->OptionalHeader.LoaderFlags, Hexadecimal),
									L"Reserved, must be zero"
								},
								{
									L"No. of Data Dirs",
									DWORD_toString(pNTheaders64->OptionalHeader.NumberOfRvaAndSizes),
									(m_PEReaderWriter.hasInvalidNoOfDataDirectories() ? L"Cannot be greater than 16, ignored" : L"")
								}
							});
							break;
					}

					if (PEType != PEReadWrite::PEType::ROM)
					{
						int NoOfDataDirs = m_PEReaderWriter.getNoOfDataDirectories_Corrected();	// NOTE: The value in this field may be invalid (i.e. greater than 16)
						if (m_PEReaderWriter.hasInvalidNoOfDataDirectories())
							LogError(L"WARNING: 'IMAGE_NT_HEADERS.OptionalHeader.NumberOfRvaAndSizes' must be less than or equal to 16. Assumed it is 16.");

						for (int i = 0; i < NoOfDataDirs; ++i)
						{
							PIMAGE_DATA_DIRECTORY pDataDir;
							err = m_PEReaderWriter.getDataDirectory(i, std::ref(pDataDir));
							if (err)
							{
								LogError(L"ERROR: Couldn't read data directory with index " + DWORD_toString(i) + L". File is not valid.", true);
								break;
							}

							m_OptionalItemsInfo.insert(m_OptionalItemsInfo.cend(),
							{
								{
									DataDirectoryNames[i] + (i != IMAGE_DIRECTORY_ENTRY_SECURITY ? L" RVA" : L" Offset"),
									DWORD_toString(pDataDir->VirtualAddress, Hexadecimal),
									(i != IMAGE_DIRECTORY_ENTRY_SECURITY && pDataDir->VirtualAddress != 0 ?
									L'\"' + m_PEReaderWriter.getContainingSectionName(pDataDir->VirtualAddress) + L"\" Section" : L"")
								},
								{
									DataDirectoryNames[i] + L" Size",
									DWORD_toString(pDataDir->Size),
									FormattedBytesSize(pDataDir->Size)
								}
							});
						}
					}
			}
		}
	
		// Insert ListView columns
		LV_COLUMN column;
		ZeroMemory(&column, sizeof(column));

		for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
		{
			column.mask = LVCF_TEXT;
			column.pszText = szGenericColumnText[i];

			ListView_InsertColumn(m_hListViewCOFFHeader, i, &column);
			ListView_InsertColumn(m_hListViewOptionalHeader, i, &column);
		}

		// Insert ListView items for COFF data list view
		LV_ITEM item;
		ZeroMemory(&item, sizeof(item));

		for (size_t i = 0; i < m_COFFItemsInfo.size(); ++i)
		{
			item.iItem = int(i);
			item.iSubItem = 0;
			item.mask = LVIF_TEXT;
			item.pszText = LPWSTR(_wcsdup(m_COFFItemsInfo[i].Text.c_str()));
			ListView_InsertItem(m_hListViewCOFFHeader, &item);
			free(item.pszText);

			item.iSubItem = 1;
			item.pszText = LPWSTR(_wcsdup(m_COFFItemsInfo[i].Data.c_str()));
			ListView_SetItem(m_hListViewCOFFHeader, &item);
			free(item.pszText);

			item.iSubItem = 2;
			item.pszText = LPWSTR(_wcsdup(m_COFFItemsInfo[i].Comments.c_str()));
			ListView_SetItem(m_hListViewCOFFHeader, &item);
			free(item.pszText);
		}

		// Insert ListView items for Optional data list view
		ZeroMemory(&item, sizeof(item));

		for (size_t i = 0; i < m_OptionalItemsInfo.size(); ++i)
		{
			item.iItem = int(i);
			item.iSubItem = 0;
			item.mask = LVIF_TEXT;
			item.pszText = LPWSTR(_wcsdup(m_OptionalItemsInfo[i].Text.c_str()));
			ListView_InsertItem(m_hListViewOptionalHeader, &item);
			free(item.pszText);

			item.iSubItem = 1;
			item.pszText = LPWSTR(_wcsdup(m_OptionalItemsInfo[i].Data.c_str()));
			ListView_SetItem(m_hListViewOptionalHeader, &item);
			free(item.pszText);

			item.iSubItem = 2;
			item.pszText = LPWSTR(_wcsdup(m_OptionalItemsInfo[i].Comments.c_str()));
			ListView_SetItem(m_hListViewOptionalHeader, &item);
			free(item.pszText);
		}

		// Resize columns
		for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
		{
			ListView_SetColumnWidth(m_hListViewCOFFHeader,
									i,
									(i == ARRAYSIZE(szGenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));
			ListView_SetColumnWidth(m_hListViewOptionalHeader,
									i,
									(i == ARRAYSIZE(szGenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));
		}
	}
}

wstring PropertyPageHandler_PEHeaders::lstCOFFHeader_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(m_COFFTooltipInfo, Index);
}

wstring PropertyPageHandler_PEHeaders::lstOptionalHeader_OnGetTooltip(int Index)
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