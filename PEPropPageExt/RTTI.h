#pragma once
#include "MiscFuncs.h"
#include <string>
#include <vector>
#include <tchar.h>


using namespace std;

namespace RTTI	// Custom class to emit 'Runtime Type Information'
{
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
		wstring FullName;
		wstring Type;
		wstring Size;
		wstring FileOffset;

		GenericTooltip(wstring name,
						wstring type,
						wstring size,
						UINT_PTR fileoffset)
						: FullName(std::move(name)),
						Type(std::move(type)),
						Size(std::move(size)),
						FileOffset(std::move(QWORD_toString(fileoffset, Hexadecimal))) {}
	};

	// RTTI functions to emit information about structures
	static void GetTooltipInfo(vector<GenericTooltip>& listtooltip, UINT_PTR filebaseaddr, RTTITypes type)
	{
		switch (type)
		{
		case RTTI_IMAGE_DOS_HEADER:
			listtooltip =
			{
				{ L"IMAGE_DOS_HEADER.e_magic", L"WORD", L"2 bytes", filebaseaddr },
				{ L"IMAGE_DOS_HEADER.e_cblp", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_cp", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_crlc", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_cparhdr", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_minalloc", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_maxalloc", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_ss", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_sp", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_csum", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_ip", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_cs", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_lfarlc", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_ovno", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_res[4]", L"QWORD", L"8 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_DOS_HEADER.e_oemid", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_oeminfo", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_res2[4]", L"QWORD", L"4 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_DOS_HEADER.e_res2[4]", L"QWORD", L"4 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_DOS_HEADER.e_res2[2]", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_DOS_HEADER.e_lfanew", L"LONG", L"4 bytes", filebaseaddr += sizeof(LONG) }
			};

			break;

		case RTTI_COFF_HEADER:
			listtooltip =
			{
				{ L"IMAGE_NT_HEADERS.Signature", L"DWORD", L"4 bytes", filebaseaddr },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.Machine", L"WORD", L"2 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.NumberOfSections", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.TimeDateStamp", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.PointerToSymbolTable", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.NumberOfSymbols", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.SizeOfOptionalHeader", L"WORD", L"2 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.Characteristics", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) }
			};

			break;

		case RTTI_ROM_HEADER:
			listtooltip =
			{
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.Machine", L"WORD", L"2 bytes", filebaseaddr },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.NumberOfSections", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.TimeDateStamp", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.PointerToSymbolTable", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.NumberOfSymbols", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.SizeOfOptionalHeader", L"WORD", L"2 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS::IMAGE_FILE_HEADER.Characteristics", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) }
			};

			break;

		case RTTI_IMAGE_OPTIONAL_HEADER32:
			listtooltip =
			{
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.Magic", L"WORD", L"2 bytes", filebaseaddr },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.LinkerVersion", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfCode", L"DWORD", L"4 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfInitializedData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfUninitializedData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.AddressOfEntryPoint", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.BaseOfCode", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.BaseOfData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.ImageBase", L"DWORD", L"4 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SectionAlignment", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.FileAlignment", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.OperatingSystemVersion", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.ImageVersion", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SubsystemVersion", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.Win32VersionValue", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfImage", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfHeaders", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.CheckSum", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.Subsystem", L"WORD", L"2 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DllCharacteristics", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfStackReserve", L"DWORD", L"4 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeofStackCommit", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfHeapReserve", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.SizeOfHeapCommit", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.LoaderFlags", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.NumberOfRvaAndSizes", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[0].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[0].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[1].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[1].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[2].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[2].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[3].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[3].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[4].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[4].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[5].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[5].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[6].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[6].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[7].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[7].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[8].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[8].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[9].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[9].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[10].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[10].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[11].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[11].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[12].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[12].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[13].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[13].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[14].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[14].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[15].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER32.DataDirectory[15].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) }
			};

			break;

		case RTTI_IMAGE_OPTIONAL_HEADER64:
			listtooltip =
			{
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.Magic", L"WORD", L"2 bytes", filebaseaddr },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.LinkerVersion", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.SizeOfCode", L"DWORD", L"4 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.SizeOfInitializedData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.SizeOfUninitializedData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.AddressOfEntryPoint", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS32::IMAGE_OPTIONAL_HEADER64.BaseOfCode", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.ImageBase", L"QWORD", L"8 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SectionAlignment", L"DWORD", L"4 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.FileAlignment", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.OperatingSystemVersion", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.ImageVersion", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SubsystemVersion", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.Win32VersionValue", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SizeOfImage", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SizeOfHeaders", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.CheckSum", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.Subsystem", L"WORD", L"2 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DllCharacteristics", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SizeOfStackReserve", L"QWORD", L"8 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SizeofStackCommit", L"QWORD", L"8 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SizeOfHeapReserve", L"QWORD", L"8 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.SizeOfHeapCommit", L"QWORD", L"8 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.LoaderFlags", L"DWORD", L"4 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.NumberOfRvaAndSizes", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[0].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[0].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[1].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[1].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[2].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[2].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[3].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[3].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[4].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[4].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[5].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[5].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[6].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[6].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[7].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[7].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[8].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[8].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[9].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[9].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[10].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[10].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[11].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[11].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[12].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[12].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[13].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[13].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[14].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[14].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[15].VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_NT_HEADERS64::IMAGE_OPTIONAL_HEADER64.DataDirectory[15].Size", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) }
			};

			break;

		case RTTI_IMAGE_ROM_OPTIONAL_HEADER:
			listtooltip =
			{
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.Magic", L"WORD", L"2 bytes", filebaseaddr },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.LinkerVersion", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.SizeOfCode", L"DWORD", L"4 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.SizeOfInitializedData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.SizeOfUninitializedData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.AddressOfEntryPoint", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.BaseOfCode", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.BaseOfData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.BaseOfBss", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.GprMask", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.CprMask[0]", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.CprMask[1]", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.CprMask[2]", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.CprMask[3]", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_ROM_HEADERS::IMAGE_ROM_OPTIONAL_HEADER.GpValue", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) }
			};

			break;

		case RTTI_SECTION_HEADER:
			listtooltip =
			{
				{ L"IMAGE_SECTION_HEADER.Name", L"BYTE, null-terminated string", L"At most 8 BYTEs, else null padded", filebaseaddr },
				{ L"IMAGE_SECTION_HEADER.VirtualSize", L"DWORD", L"4 bytes", filebaseaddr += (8 * sizeof(BYTE)) },
				{ L"IMAGE_SECTION_HEADER.VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_SECTION_HEADER.SizeOfRawData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_SECTION_HEADER.PointerToRawData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_SECTION_HEADER.PointerToRelocations", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_SECTION_HEADER.PointerToLinenumbers", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_SECTION_HEADER.NumberOfRelocations", L"WORD", L"2 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_SECTION_HEADER.NumberOfLinenumbers", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_SECTION_HEADER.Characteristics", L"DWORD", L"4 bytes", filebaseaddr += sizeof(WORD) }
			};

			break;

		case RTTI_IMPORT_DIRECTORY:
			listtooltip =
			{
				{ L"IMAGE_IMPORT_DESCRIPTOR.Characteristics", L"DWORD", L"4 bytes", filebaseaddr },
				{ L"IMAGE_IMPORT_DESCRIPTOR.TimeDateStamp", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_IMPORT_DESCRIPTOR.ForwarderChain", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_IMPORT_DESCRIPTOR.Name", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_IMPORT_DESCRIPTOR.FirstThunk", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) }
			};

			break;

		case RTTI_EXPORT_DIRECTORY:
			listtooltip =
			{
				{ L"IMAGE_EXPORT_DIRECTORY.Characteristics", L"DWORD", L"4 bytes", filebaseaddr },
				{ L"IMAGE_EXPORT_DIRECTORY.TimeDateStamp", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_EXPORT_DIRECTORY.Version", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_EXPORT_DIRECTORY.Name", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_EXPORT_DIRECTORY.Base", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_EXPORT_DIRECTORY.NumberOfFunctions", L"DWORD", L"4 bytes", filebaseaddr },
				{ L"IMAGE_EXPORT_DIRECTORY.NumberOfNames", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_EXPORT_DIRECTORY.AddressOfFunctions", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_EXPORT_DIRECTORY.AddressOfNames", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_EXPORT_DIRECTORY.AddressOfNameOrdinals", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) }
			};

			break;

		case RTTI_BASE_RELOC:
			listtooltip =
			{
				{ L"IMAGE_BASE_RELOCATION.VirtualAddress", L"DWORD", L"4 bytes", filebaseaddr },
				{ L"IMAGE_BASE_RELOCATION.SizeOfBlock", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) }
			};

			break;

		case RTTI_DEBUG_DIRECTORY:
			listtooltip =
			{
				{ L"IMAGE_DEBUG_DIRECTORY.Characteristics", L"DWORD", L"4 bytes", filebaseaddr },
				{ L"IMAGE_DEBUG_DIRECTORY.TimeDateStamp", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_DEBUG_DIRECTORY.Version", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_DEBUG_DIRECTORY.Type", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_DEBUG_DIRECTORY.SizeOfData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_DEBUG_DIRECTORY.AddressOfRawData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_DEBUG_DIRECTORY.PointerToRawData", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) }
			};

			break;

		case RTTI_LOAD_CONFIG32:
			listtooltip =
			{
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.Size", L"DWORD", L"4 bytes", filebaseaddr },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.TimeDateStamp", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.Version", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.GlobalFlagsClear", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.GlobalFlagsSet", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.CriticalSectionDefaultTimeout", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.DeCommitFreeBlockThreshold", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.DeCommitTotalFreeThreshold", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.LockPrefixTable", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.MaximumAllocationSize", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.VirtualMemoryThreshold", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.ProcessHeapFlags", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.ProcessAffinityMask", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.CSDVersion", L"WORD", L"2 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.Reserved1", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.EditList", L"DWORD", L"4 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.SecurityCookie", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.SEHandlerTable", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.SEHandlerCount", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) }
			};

			break;

		case RTTI_LOAD_CONFIG64:
			listtooltip =
			{
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.Size", L"DWORD", L"4 bytes", filebaseaddr },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.TimeDateStamp", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.Version", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.GlobalFlagsClear", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.GlobalFlagsSet", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.CriticalSectionDefaultTimeout", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.DeCommitFreeBlockThreshold", L"QWORD", L"8 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.DeCommitTotalFreeThreshold", L"QWORD", L"8 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.LockPrefixTable", L"QWORD", L"8 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.MaximumAllocationSize", L"QWORD", L"8 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.VirtualMemoryThreshold", L"QWORD", L"8 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.ProcessAffinityMask", L"QWORD", L"8 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.ProcessHeapFlags", L"DWORD", L"4 bytes", filebaseaddr += sizeof(ULONGLONG) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.CSDVersion", L"WORD", L"2 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.Reserved1", L"WORD", L"2 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.EditList", L"DWORD", L"4 bytes", filebaseaddr += sizeof(WORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.SecurityCookie", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.SEHandlerTable", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) },
				{ L"IMAGE_LOAD_CONFIG_DIRECTORY32.SEHandlerCount", L"DWORD", L"4 bytes", filebaseaddr += sizeof(DWORD) }
			};
		}
	}
};