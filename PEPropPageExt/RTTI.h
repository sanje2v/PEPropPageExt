#pragma once
#include "CommonDefs.h"
#include "MiscFuncs.h"
#include <Windows.h>
#include <vector>
#include <tchar.h>

#define FillData2(vectorobj, ...)		(vectorobj).push_back(GenericTooltip(__VA_ARGS__))

using namespace std;


class RTTI abstract	// Custom class to emit 'Runtime Type Information'
{
public:
	// Enumerated types of structure 'types' that this class handles
	enum RTTITypes
	{
		RTTI_IMAGE_DOS_HEADER,
		RTTI_COFF_HEADER,
		RTTI_ROM_HEADER,
		RTTI_IMAGE_OPTIONAL_HEADER32,
		RTTI_IMAGE_OPTIONAL_HEADER64,
		RTTI_IMAGE_ROM_OPTIONAL_HEADER,
		RTTI_SECTION_HEADER,
		RTTI_IMPORT_DIRECTORY,
		RTTI_EXPORT_DIRECTORY,
		RTTI_BASE_RELOC,
		RTTI_DEBUG_DIRECTORY,
		RTTI_LOAD_CONFIG32,
		RTTI_LOAD_CONFIG64
	};

	struct GenericTooltip
	{
		tstring FullName;
		tstring Type;
		tstring Size;
		tstring FileOffset;

		GenericTooltip(tstring name,
						tstring type,
						tstring size,
						UINT_PTR fileoffset)
			: FullName(name),
				Type(type),
				Size(size),
				FileOffset(QWORD_toString(fileoffset, Hexadecimal)) { }
	};

	// RTTI functions to emit information about structures
	static void GetTooltipInfo(vector<GenericTooltip>& listtooltip, UINT_PTR filebaseaddr, RTTITypes type)
	{
		switch (type)
		{
		case RTTI_IMAGE_DOS_HEADER:
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_magic"), _T("WORD"), _T("2 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_cblp"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_cp"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_crlc"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_cparhdr"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_minalloc"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_maxalloc"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_ss"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_sp"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_csum"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_ip"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_cs"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_lfarlc"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_ovno"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_res[4]"), _T("QWORD"), _T("8 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_oemid"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_oeminfo"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_res2[4]"), _T("QWORD"), _T("4 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_res2[4]"), _T("QWORD"), _T("4 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_res2[2]"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_DOS_HEADER.e_lfanew"), _T("LONG"), _T("4 bytes"), filebaseaddr+=sizeof(LONG));

			break;

		case RTTI_COFF_HEADER:
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS.Signature"), _T("DWORD"), _T("4 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.Machine"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.NumberOfSections"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.TimeDateStamp"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.PointerToSymbolTable"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.NumberOfSymbols"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.SizeOfOptionalHeader"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.Characteristics"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));

			break;

		case RTTI_ROM_HEADER:
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.Machine"), _T("WORD"), _T("2 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.NumberOfSections"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.TimeDateStamp"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.PointerToSymbolTable"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.NumberOfSymbols"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.SizeOfOptionalHeader"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.Characteristics"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));

			break;

		case RTTI_IMAGE_OPTIONAL_HEADER32:
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.Magic"), _T("WORD"), _T("2 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.LinkerVersion"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfCode"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfInitializedData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfUninitializedData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.AddressOfEntryPoint"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.BaseOfCode"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.BaseOfData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.ImageBase"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SectionAlignment"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.FileAlignment"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.OperatingSystemVersion"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.ImageVersion"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SubsystemVersion"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.Win32VersionValue"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfImage"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfHeaders"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.CheckSum"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.Subsystem"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DllCharacteristics"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfStackReserve"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeofStackCommit"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfHeapReserve"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfHeapCommit"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.LoaderFlags"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.NumberOfRvaAndSizes"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[0].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[0].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[1].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[1].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[2].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[2].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[3].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[3].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[4].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[4].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[5].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[5].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[6].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[6].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[7].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[7].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[8].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[8].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[9].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[9].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[10].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[10].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[11].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[11].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[12].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[12].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[13].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[13].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[14].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[14].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[15].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[15].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));

			break;

		case RTTI_IMAGE_OPTIONAL_HEADER64:
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.Magic"), _T("WORD"), _T("2 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.LinkerVersion"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.SizeOfCode"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.SizeOfInitializedData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.SizeOfUninitializedData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.AddressOfEntryPoint"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.BaseOfCode"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.ImageBase"), _T("QWORD"), _T("8 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SectionAlignment"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.FileAlignment"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.OperatingSystemVersion"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.ImageVersion"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SubsystemVersion"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.Win32VersionValue"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SizeOfImage"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SizeOfHeaders"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.CheckSum"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.Subsystem"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DllCharacteristics"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SizeOfStackReserve"), _T("QWORD"), _T("8 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SizeofStackCommit"), _T("QWORD"), _T("8 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SizeOfHeapReserve"), _T("QWORD"), _T("8 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SizeOfHeapCommit"), _T("QWORD"), _T("8 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.LoaderFlags"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.NumberOfRvaAndSizes"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[0].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[0].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[1].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[1].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[2].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[2].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[3].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[3].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[4].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[4].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[5].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[5].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[6].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[6].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[7].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[7].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[8].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[8].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[9].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[9].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[10].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[10].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[11].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[11].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[12].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[12].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[13].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[13].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[14].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[14].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[15].VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[15].Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));

			break;

		case RTTI_IMAGE_ROM_OPTIONAL_HEADER:
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.Magic"), _T("WORD"), _T("2 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.LinkerVersion"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.SizeOfCode"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.SizeOfInitializedData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.SizeOfUninitializedData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.AddressOfEntryPoint"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.BaseOfCode"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.BaseOfData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.BaseOfBss"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.GprMask"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.CprMask[0]"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.CprMask[1]"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.CprMask[2]"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.CprMask[3]"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.GpValue"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));

			break;

		case RTTI_SECTION_HEADER:
			FillData2(listtooltip, _T("IMAGE_SECTION_HEADER.Name"), _T("BYTE, null-terminated string"), _T("At most 8 BYTEs, else null padded"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_SECTION_HEADER.VirtualSize"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=(8 * sizeof(BYTE)));
			FillData2(listtooltip, _T("IMAGE_SECTION_HEADER.VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_SECTION_HEADER.SizeOfRawData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_SECTION_HEADER.PointerToRawData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_SECTION_HEADER.PointerToRelocations"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_SECTION_HEADER.PointerToLinenumbers"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_SECTION_HEADER.NumberOfRelocations"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_SECTION_HEADER.NumberOfLinenumbers"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_SECTION_HEADER.Characteristics"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(WORD));

			break;

		case RTTI_IMPORT_DIRECTORY:
			FillData2(listtooltip, _T("IMAGE_IMPORT_DESCRIPTOR.Characteristics"), _T("DWORD"), _T("4 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_IMPORT_DESCRIPTOR.TimeDateStamp"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_IMPORT_DESCRIPTOR.ForwarderChain"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_IMPORT_DESCRIPTOR.Name"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_IMPORT_DESCRIPTOR.FirstThunk"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));

			break;

		case RTTI_EXPORT_DIRECTORY:
			FillData2(listtooltip, _T("IMAGE_EXPORT_DIRECTORY.Characteristics"), _T("DWORD"), _T("4 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_EXPORT_DIRECTORY.TimeDateStamp"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_EXPORT_DIRECTORY.Version"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_EXPORT_DIRECTORY.Name"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_EXPORT_DIRECTORY.Base"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_EXPORT_DIRECTORY.NumberOfFunctions"), _T("DWORD"), _T("4 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_EXPORT_DIRECTORY.NumberOfNames"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_EXPORT_DIRECTORY.AddressOfFunctions"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_EXPORT_DIRECTORY.AddressOfNames"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_EXPORT_DIRECTORY.AddressOfNameOrdinals"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));

			break;

		case RTTI_BASE_RELOC:
			FillData2(listtooltip, _T("IMAGE_BASE_RELOCATION.VirtualAddress"), _T("DWORD"), _T("4 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_BASE_RELOCATION.SizeOfBlock"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));

			break;

		case RTTI_DEBUG_DIRECTORY:
			FillData2(listtooltip, _T("IMAGE_DEBUG_DIRECTORY.Characteristics"), _T("DWORD"), _T("4 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_DEBUG_DIRECTORY.TimeDateStamp"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_DEBUG_DIRECTORY.Version"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_DEBUG_DIRECTORY.Type"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_DEBUG_DIRECTORY.SizeOfData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_DEBUG_DIRECTORY.AddressOfRawData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_DEBUG_DIRECTORY.PointerToRawData"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));

			break;

		case RTTI_LOAD_CONFIG32:
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.TimeDateStamp"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.Version"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.GlobalFlagsClear"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.GlobalFlagsSet"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.CriticalSectionDefaultTimeout"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.DeCommitFreeBlockThreshold"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.DeCommitTotalFreeThreshold"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.LockPrefixTable"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.MaximumAllocationSize"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.VirtualMemoryThreshold"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.ProcessHeapFlags"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.ProcessAffinityMask"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.CSDVersion"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.Reserved1"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.EditList"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.SecurityCookie"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.SEHandlerTable"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.SEHandlerCount"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));

			break;

		case RTTI_LOAD_CONFIG64:
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.Size"), _T("DWORD"), _T("4 bytes"), filebaseaddr);
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.TimeDateStamp"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.Version"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.GlobalFlagsClear"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.GlobalFlagsSet"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.CriticalSectionDefaultTimeout"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.DeCommitFreeBlockThreshold"), _T("QWORD"), _T("8 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.DeCommitTotalFreeThreshold"), _T("QWORD"), _T("8 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.LockPrefixTable"), _T("QWORD"), _T("8 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.MaximumAllocationSize"), _T("QWORD"), _T("8 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.VirtualMemoryThreshold"), _T("QWORD"), _T("8 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.ProcessAffinityMask"), _T("QWORD"), _T("8 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.ProcessHeapFlags"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(ULONGLONG));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.CSDVersion"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.Reserved1"), _T("WORD"), _T("2 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.EditList"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(WORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.SecurityCookie"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.SEHandlerTable"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
			FillData2(listtooltip, _T("IMAGE_LOAD_CONFIG_DIRECTORY32.SEHandlerCount"), _T("DWORD"), _T("4 bytes"), filebaseaddr+=sizeof(DWORD));
		}
	}
};

#undef FillData2