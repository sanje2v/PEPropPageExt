#include "MiscFuncs.h"


using namespace std;

/*void DebugWrite(char *szMsg)
{
	DWORD byteswritten;
	DWORD a;
	HANDLE hFile = CreateFile(L"C:\\TestFolder\\dbg.txt", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(hFile, 0, NULL, FILE_END);
	WriteFile(hFile, szMsg, strlen(szMsg), &byteswritten, NULL);
	CloseHandle(hFile);
}*/

void LogError(wstring Info, bool bCritical)
{
	MessageBox(NULL,
			   LPWSTR(Info.c_str()),
			   (bCritical ? NULL : L"Information"), 
			   MB_OK | MB_APPLMODAL | (bCritical ? MB_ICONERROR : MB_ICONINFORMATION));
}

void wstring_ReplaceAll(wstring& Source, WCHAR cToFind, WCHAR cToReplace)
{
	size_t lastfindpos = -1;

	while ((lastfindpos = Source.find(cToFind, lastfindpos + 1)) != wstring::npos)
		Source.replace(lastfindpos, 1, 1, cToReplace);
}

wstring Integer_toString(int value)
{
	WCHAR szBuffer[1024];
	swprintf_s(szBuffer, ARRAYSIZE(szBuffer), L"%i", value);

	return szBuffer;
}

wstring BYTE_toString(BYTE value, FORMATTYPE type, bool fixedaligned, bool prefix)
{
	WCHAR szBuffer[1024];

	if (fixedaligned)
		swprintf_s(szBuffer, ARRAYSIZE(szBuffer), (type == Decimal ? L"%.3u" : L"%.2X"), value);
	else
		swprintf_s(szBuffer, ARRAYSIZE(szBuffer), (type == Decimal ? L"%u" : L"%X"), value);

	return (prefix && type == Hexadecimal ? L"0x " + wstring(szBuffer) : wstring(szBuffer));
}

wstring WORD_toString(WORD value, FORMATTYPE type, bool fixedaligned, bool prefix)
{
	WCHAR szBuffer[1024];

	if (fixedaligned)
		swprintf_s(szBuffer, ARRAYSIZE(szBuffer), (type == Decimal ? L"%.5u" : L"%.4X"), value);
	else
		swprintf_s(szBuffer, ARRAYSIZE(szBuffer), (type == Decimal ? L"%u" : L"%X"), value);

	return (prefix && type == Hexadecimal ? L"0x" + wstring(szBuffer) : wstring(szBuffer));
}

wstring DWORD_toString(DWORD value, FORMATTYPE type, bool fixedaligned, bool prefix)
{
	WCHAR szBuffer[1024];

	if (fixedaligned)
		swprintf_s(szBuffer, ARRAYSIZE(szBuffer), (type == Decimal ? L"%.10u" : L"%.8X"), value);
	else
		swprintf_s(szBuffer, ARRAYSIZE(szBuffer), (type == Decimal ? L"%u" : L"%X"), value);

	return (prefix && type == Hexadecimal ? L"0x" + wstring(szBuffer) : wstring(szBuffer));
}

wstring QWORD_toString(ULONGLONG value, FORMATTYPE type, bool fixedaligned, bool prefix)
{
	WCHAR szBuffer[1024];

	if (fixedaligned)
		swprintf_s(szBuffer, ARRAYSIZE(szBuffer), (type == Decimal ? L"%.16I64u" : L"%.16I64X"), value);
	else
		swprintf_s(szBuffer, ARRAYSIZE(szBuffer), (type == Decimal ? L"%I64u" : L"%I64X"), value);

	return (prefix && type == Hexadecimal ? L"0x" + wstring(szBuffer) : wstring(szBuffer));
}

wstring Signature_toString(DWORD value)
{
	return MultiByte_toString((char *) &value, true, sizeof(DWORD));
}

wstring float_toString(float value)//, int decimalplaces = 2)
{
	WCHAR szBuffer[1024];
	swprintf_s(szBuffer, ARRAYSIZE(szBuffer), L"%.2f", value);

	return szBuffer;
}

LPWSTR LEOrdering_toString(BYTE order)
{
	return (order == 0 ? L"Little Endian" : L"Big Endian");
}

LPWSTR LECPUtype_toString(WORD type)
{
	switch (type)
	{
		case 0x0001: return L"Intel 80286 or upwardly compatible";
		case 0x0002: return L"Intel 80386 or upwardly compatible";
		case 0x0003: return L"Intel 80486 or upwardly compatible";
		case 0x0004: return L"Intel 80586 or upwardly compatible";
		case 0x0020: return L"Intel i860 (N10) or compatible";
		case 0x0021: return L"Intel \"N11\" or compatible";
		case 0x0040: return L"MIPS Mark I (R2000, R3000) or compatible";
		case 0x0041: return L"MIPS Mark II (R6000) or compatible";
		case 0x0042: return L"MIPS Mark III (R4000) or compatible";
	}

	return L"Unknown CPU type";
}

LPWSTR LEOStype_toString(WORD type)
{
	switch (type)
	{
		case 0x0001: return L"OS/2";
		case 0x0002: return L"Windows";
		case 0x0003: return L"DOS 4.x";
		case 0x0004: return L"Windows 386";
	}

	return L"Unknown OS type";
}

wstring LEModuletypeflags_toString(DWORD value)
{
	// NOTE: Flags in this function are from 'www.textfiles.com/programming/FORMATS/lxexe.txt'
	wstring FlagsDesc;

	if (value == 0)
		return L"";

	DWORD flag = 0x01L;

	if (TestFlag(value, 0x01L))
		FlagsDesc += L"Reserved(0x01), ";
	else if (TestFlag(value, 0x02L))
		FlagsDesc += L"Reserved(0x02), ";

	if (TestFlag(value, 0x04L))
		FlagsDesc += L"Per-Process Initialization (DLL only), ";
	else
		FlagsDesc += L"Global Initialization (DLL only), ";

	if (TestFlag(value, 0x08L))
		FlagsDesc += L"Reserved(0x08), ";
	else if (TestFlag(value, 0x10L))
		FlagsDesc += L"Has internal relocation info, ";
	else if (TestFlag(value, 0x20L))
		FlagsDesc += L"Has external relocation info, ";
	else if (TestFlag(value, 0x40L))
		FlagsDesc += L"Reserved(0x40), ";
	else if (TestFlag(value, 0x80L))
		FlagsDesc += L"Reserved(0x80), ";
	else if (TestFlag(value, 0x100L))
		FlagsDesc += L"Incompatible with PM windowing, ";
	else if (TestFlag(value, 0x200L))
		FlagsDesc += L"Compatible with PM windowing, ";
	else if (TestFlag(value, 0x300L))
		FlagsDesc += L"Uses PM windowing API, ";
	else if (TestFlag(value, 0x400L))
		FlagsDesc += L"Reserved(0x400), ";
	else if (TestFlag(value, 0x800L))
		FlagsDesc += L"Reserved(0x800), ";
	else if (TestFlag(value, 0x1000L))
		FlagsDesc += L"Reserved(0x1000), ";
	else if (TestFlag(value, 0x2000L))
		FlagsDesc += L"Module cannot be loaded, ";
	else if (TestFlag(value, 0x4000L))
		FlagsDesc += L"Reserved(0x4000), ";
	else if (TestFlag(value, 0x8000L))
		FlagsDesc += L"Module is a DLL, ";
	else if (TestFlag(value, 0x18000L))
		FlagsDesc += L"Protected Memory Library, ";
	else if (TestFlag(value, 0x20000L))
		FlagsDesc += L"Physical Device Driver, ";
	else if (TestFlag(value, 0x28000L))
		FlagsDesc += L"Virtual Device Driver, ";
	else if (TestFlag(value, 0x40000000L))
		FlagsDesc += L"Per-process Library Termination, ";

	return RemoveTrailingCommaSpace(FlagsDesc);
}

wstring VersionNums_toString(WORD major, WORD minor)
{
	return DWORD_toString(major) + L'.' + DWORD_toString(minor);
}

wstring MagicNum_toString(DWORD num)
{
	switch (num)
	{
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:	return L"32-bit Image";
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:	return L"64-bit Image";
		case IMAGE_ROM_OPTIONAL_HDR_MAGIC:	return L"ROM Image";
	}

	return L"Unknown value";
}

wstring OSId_toString(WORD major, WORD minor)
{
	/*
	Windows OS Ids from: 
	http://msdn.microsoft.com/en-us/library/windows/desktop/ms724832%28v=vs.85%29.aspx
	*/

	if(major < 5)
		return L"Earlier than Windows 2000";

	switch (major)
	{
	case 5:
		switch (minor)
		{
			case 0:	return L"Windows 2000";
			case 1:	return L"Windows XP";
			case 2: return L"Windows XP 64-Bit Edition, Windows Server 2003 & R2";
		}

		break;

	case 6:
		switch (minor)
		{
			case 0: return L"Windows Vista, Windows Server 2008";
			case 1:	return L"Windows Server 2008 R2, Windows 7";
			case 2: return L"Windows Server 2012, Windows 8";
			case 3: return L"Windows Server 2012 R2, Windows 8.1";
		}
	}
	
	return L"Unknown/Invalid OS version number";
}

wstring FormattedBytesSize(ULONGLONG size)
{
	wstring SizeDesc = L"bytes";

	if (size < 1024)
		return SizeDesc;
	else if (size < 1024 * 1024)
		return SizeDesc + L" (" + float_toString(size / 1024.0f) + L" KB)";
	else
		return SizeDesc + L" (" + float_toString(size / (1024.0f * 1024.0f)) + L" MB)";
}

wstring MachineType_toString(WORD value)
{
	switch (value)
	{
		case IMAGE_FILE_MACHINE_UNKNOWN: return L"Any machine";
		case IMAGE_FILE_MACHINE_AM33: return L"Matsushita AM33";
		case IMAGE_FILE_MACHINE_AMD64: return L"Intel/AMD x64";
		case IMAGE_FILE_MACHINE_ARM: return L"ARM little endian";
		case IMAGE_FILE_MACHINE_EBC: return L"EFI byte code";
		case IMAGE_FILE_MACHINE_I386: return L"Intel x86";
		case IMAGE_FILE_MACHINE_IA64: return L"Intel Itanium";
		case IMAGE_FILE_MACHINE_M32R: return L"Mitsubishi M32R little endian";
		case IMAGE_FILE_MACHINE_MIPS16:	return L"MIPS16";
		case IMAGE_FILE_MACHINE_MIPSFPU: return L"MIPS with FPU";
		case IMAGE_FILE_MACHINE_MIPSFPU16: return L"MIPS16 with FPU";
		case IMAGE_FILE_MACHINE_POWERPC: return L"Power PC little endian";
		case IMAGE_FILE_MACHINE_POWERPCFP: return L"Power PC with floating point support";
		case IMAGE_FILE_MACHINE_R4000: return L"MIPS little endian";
		case IMAGE_FILE_MACHINE_SH3: return L"Hitachi SH3";
		case IMAGE_FILE_MACHINE_SH3DSP: return L"Hitachi SH3 DSP";
		case IMAGE_FILE_MACHINE_SH4: return L"Hitachi SH4";
		case IMAGE_FILE_MACHINE_SH5: return L"Hitachi SH5";
		case IMAGE_FILE_MACHINE_THUMB: return L"Thumb";
		case IMAGE_FILE_MACHINE_WCEMIPSV2: return L"MIPS little endian WCE v2";
	}

	return L"Unknown machine type";
}

wstring ImageCharacteristics_toString(DWORD value)
{
	wstring CharsDesc;

	if (value == 0)
		return L"";

	if (TestFlag(value, IMAGE_FILE_RELOCS_STRIPPED))
		CharsDesc += L"No relocation info (Will only load at preferred address), ";

	if(TestFlag(value, IMAGE_FILE_EXECUTABLE_IMAGE))
		CharsDesc += L"Executable, ";

	if(TestFlag(value, IMAGE_FILE_LINE_NUMS_STRIPPED))
		CharsDesc += L"COFF Line no. stripped, ";

	if(TestFlag(value, IMAGE_FILE_LOCAL_SYMS_STRIPPED))
		CharsDesc += L"COFF symbol table stripped, ";

	if(TestFlag(value, IMAGE_FILE_AGGRESIVE_WS_TRIM))
		CharsDesc += L"Aggressively trim Working Set (Obsolete), ";

	if(TestFlag(value, IMAGE_FILE_LARGE_ADDRESS_AWARE))
		CharsDesc += L"2GB+ addr aware, ";

	if(TestFlag(value, IMAGE_FILE_BYTES_REVERSED_LO))
		CharsDesc += L"Little endian (Deprecated), ";

	if(TestFlag(value, IMAGE_FILE_32BIT_MACHINE))
		CharsDesc += L"For 32-bit machine, ";

	if(TestFlag(value, IMAGE_FILE_DEBUG_STRIPPED))
		CharsDesc += L"Debug info in separate file, ";

	if(TestFlag(value, IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP))
		CharsDesc += L"If in removable media - copy and run from swap, ";

	if(TestFlag(value, IMAGE_FILE_NET_RUN_FROM_SWAP))
		CharsDesc += L"If on network - copy and run from swap, ";

	if(TestFlag(value, IMAGE_FILE_SYSTEM))
		CharsDesc += L"System file, ";

	if(TestFlag(value, IMAGE_FILE_DLL))
		CharsDesc += L"DLL file, ";

	if(TestFlag(value, IMAGE_FILE_UP_SYSTEM_ONLY))
		CharsDesc += L"For uniprocessor machine only, ";

	if(TestFlag(value, IMAGE_FILE_BYTES_REVERSED_HI))
		CharsDesc += L"Big endian (Deprecated), ";

	return RemoveTrailingCommaSpace(CharsDesc);
}

wstring SubsystemID_toString(WORD value)
{
	switch (value)
	{
	case IMAGE_SUBSYSTEM_UNKNOWN:
		return L"Unknown subsystem";

	case IMAGE_SUBSYSTEM_NATIVE:
		return L"No subsystem required (device drivers and native system processes)";

	case IMAGE_SUBSYSTEM_WINDOWS_GUI:
		return L"GUI subsystem";

	case IMAGE_SUBSYSTEM_WINDOWS_CUI:
		return L"Character mode UI subsystem";

	case IMAGE_SUBSYSTEM_OS2_CUI:
		return L"OS/2 CUI subsystem";

	case IMAGE_SUBSYSTEM_POSIX_CUI:
		return L"POSIX CUI subsystem";

	case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:
		return L"Windows CE system";
		
	case IMAGE_SUBSYSTEM_EFI_APPLICATION:
		return L"Extensible Firmware Interface (EFI) application";

	case IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
		return L"EFI driver with boot services";

	case IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
		return L"EFI driver with run-time services";

	case IMAGE_SUBSYSTEM_EFI_ROM:
		return L"EFI ROM image";

	case IMAGE_SUBSYSTEM_XBOX:
		return L"XBOX system";
		
	case IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION:
		return L"Boot application";
	}

	return L"Unknown value";
}

wstring DllCharacteristics_toString(WORD value)
{
	wstring CharsDesc;

	if (value == 0)
		return L"";

	if (TestFlag(value, 0x0001))
		CharsDesc += L"Reserved(0x0001), ";

	if (TestFlag(value, 0x0002))
		CharsDesc += L"Reserved(0x0002), ";

	if (TestFlag(value, 0x0004))
		CharsDesc += L"Reserved(0x0004), ";

	if (TestFlag(value, 0x0008))
		CharsDesc += L"Reserved(0x0008), ";

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE))
		CharsDesc += L"Has relocation section, ";

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY))
		CharsDesc += L"Code integrity checks enforced, ";

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_NX_COMPAT))
		CharsDesc += L"DEP compatible, ";

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_NO_ISOLATION))
		CharsDesc += L"Not to be isolated, ";

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_NO_SEH))
		CharsDesc += L"No SEH, ";

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_NO_BIND))
		CharsDesc += L"No Binding, ";

	if (TestFlag(value, 0x1000))
		CharsDesc += L"Reserved(0x1000), ";

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_WDM_DRIVER))
		CharsDesc += L"Is a Windows Driver Model driver, ";

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE))
		CharsDesc += L"Terminal Server aware, ";

	return RemoveTrailingCommaSpace(CharsDesc);
}

wstring ProperSectionName(BYTE szRawSectionName[IMAGE_SIZEOF_SHORT_NAME])
{
	wstring SectionName, BufferChar;

	for (int i = 0; i < IMAGE_SIZEOF_SHORT_NAME; ++i)
	{
		BufferChar = MultiByte_toString((char *) &szRawSectionName[i], true, 1);

		if (BufferChar.at(0) == L'\0')
			break;

		SectionName += BufferChar;
	}

	return SectionName;
}

wstring SectionCharacteristics_toString(DWORD value)
{
	wstring CharsDesc;

	if (value == 0)
		return L"Reserved(0x00000000)";

	if (TestFlag(value, 0x00000001))
		CharsDesc += L"Reserved(0x00000001), ";

	if (TestFlag(value, 0x00000002))
		CharsDesc += L"Reserved(0x00000002), ";

	if (TestFlag(value, 0x00000004))
		CharsDesc += L"Reserved(0x00000004), ";

	if (TestFlag(value, IMAGE_SCN_TYPE_NO_PAD))
		CharsDesc += L"No padding to next boundary, ";

	if (TestFlag(value, 0x00000010))
		CharsDesc += L"Reserved(0x00000010), ";

	if (TestFlag(value, IMAGE_SCN_CNT_CODE))
		CharsDesc += L"Has code, ";

	if (TestFlag(value, IMAGE_SCN_CNT_INITIALIZED_DATA))
		CharsDesc += L"Initialized data, ";

	if (TestFlag(value, IMAGE_SCN_CNT_UNINITIALIZED_DATA))
		CharsDesc += L"Uninitialized data, ";

	if (TestFlag(value, 0x00000100))
		CharsDesc += L"Reserved(0x00000100), ";

	if (TestFlag(value, IMAGE_SCN_LNK_INFO))
		CharsDesc += L"Comments or other info, ";

	if (TestFlag(value, 0x00000400))
		CharsDesc += L"Reserved(0x00000400), ";

	if (TestFlag(value, IMAGE_SCN_LNK_REMOVE))
		CharsDesc += L"Should be removed during linking, ";

	if (TestFlag(value, IMAGE_SCN_LNK_COMDAT))
		CharsDesc += L"COMDAT data, ";

	if (TestFlag(value, IMAGE_SCN_GPREL))
		CharsDesc += L"Data referenced through global ptrs, ";

	if (TestFlag(value, 0x00020000))
		CharsDesc += L"ARM Thumb code/Reserved(0x00020000) for other machine types, ";

	if (TestFlag(value, 0x00040000))
		CharsDesc += L"Reserved(0x00040000), ";

	if (TestFlag(value, 0x00080000))
		CharsDesc += L"Reserved(0x00080000), ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_1BYTES))
		CharsDesc += L"Align data on 1-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_2BYTES))
		CharsDesc += L"Align data on 2-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_4BYTES))
		CharsDesc += L"Align data on 4-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_8BYTES))
		CharsDesc += L"Align data on 8-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_16BYTES))
		CharsDesc += L"Align data on 16-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_32BYTES))
		CharsDesc += L"Align data on 32-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_64BYTES))
		CharsDesc += L"Align data on 64-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_128BYTES))
		CharsDesc += L"Align data on 128-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_256BYTES))
		CharsDesc += L"Align data on 256-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_512BYTES))
		CharsDesc += L"Align data on 512-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_1024BYTES))
		CharsDesc += L"Align data on 1024-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_2048BYTES))
		CharsDesc += L"Align data on 2048-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_4096BYTES))
		CharsDesc += L"Align data on 4096-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_ALIGN_8192BYTES))
		CharsDesc += L"Align data on 8192-byte boundary, ";

	if (TestFlag(value, IMAGE_SCN_LNK_NRELOC_OVFL))
		CharsDesc += L"Extended relocations, ";

	if (TestFlag(value, IMAGE_SCN_MEM_DISCARDABLE))
		CharsDesc += L"No need to load, ";

	if (TestFlag(value, IMAGE_SCN_MEM_NOT_CACHED))
		CharsDesc += L"Non-cacheable, ";

	if (TestFlag(value, IMAGE_SCN_MEM_NOT_PAGED))
		CharsDesc += L"Non-pagable, ";

	if (TestFlag(value, IMAGE_SCN_MEM_SHARED))
		CharsDesc += L"Sharable, ";

	if (TestFlag(value, IMAGE_SCN_MEM_EXECUTE))
		CharsDesc += L"Executable, ";

	if (TestFlag(value, IMAGE_SCN_MEM_READ))
		CharsDesc += L"Readable, ";

	if (TestFlag(value, IMAGE_SCN_MEM_WRITE))
		CharsDesc += L"Writable, ";

	return RemoveTrailingCommaSpace(CharsDesc);
}

wstring StandardSectionNameAnnotation(wstring name)
{
	std::transform(name.cbegin(), name.cend(), name.begin(), ::tolower);

	if (name == L".text" || name == L".code" || name == L"code")
		return L"Code Section";

	if (name == L".debug")
		return L"Debug Section";

	if (name == L".drectve")
		return L"Directive Section";

	if (name == L".edata")
		return L"Export Data Section";

	if (name == L".idata")
		return L"Import Data Section";

	if (name == L".data")
		return L"Initialized Modifiable Data Section";

	if (name == L".reloc")
		return L"Base Relocation Fixes Section";

	if (name == L".tls")
		return L"Thread Local Storage Section";

	if (name == L".rsrc")
		return L"Resource Data Section";

	if (name == L".pdata")
		return L"Function Pointer Table Section";

	if ((name == L".rdata") || (name == L".rodata"))
		return L"Initialized Constant Data Section";

	if (name == L".bss")
		return L"Static Data Section";

	return L"";
}

wstring ExceptionArch_toString(DWORD val)
{
	return (val ? L"Consists of 32-bit instructions" : L"Consists of 64-bit instructions");
}

wstring ExceptionFlag_toString(DWORD flag)
{
	return (flag ? L"Exception handler exists" : L"No exception handler exists");
}

wstring LCID_toLocaleName(WORD lcid)
{
	if (lcid == 0)
		return L"Neutral";

	// NOTE: The function 'LCIDToLocaleName()' is only available from Vista onwards
	typedef int(*pLCIDToLocaleName)(LCID Locale, LPWSTR lpName, int cchName, DWORD dwFlags);

	pLCIDToLocaleName LCIDToLocaleName = pLCIDToLocaleName(GetProcAddress(GetModuleHandle(L"kernel32"), "LCIDToLocaleName"));
	
	WCHAR szLocaleName[32] = { 0 };
	LCIDToLocaleName(lcid, szLocaleName, ARRAYSIZE(szLocaleName), 0);

	return szLocaleName;
}

wstring MultiByte_toString(const char *pszdata, bool bisANSI, int datasize)
{
	// This function allows ANSI, UTF-8 (with or without BOM) and UTF-16 (with or without BOM,
	// for UTF-16 in UNICODE mode the datasize must be given) to be converted into either 
	// 'string' or 'wstring' according to whether 'UNICODE' is defined or not.
	// NOTE: Other code pages and UTF-32 is not supported.

	if (pszdata == NULL ||
		datasize < -1 ||
		datasize == 0)
		return L"";

	if (!bisANSI)
	{
		// Is not ANSI, so is either UTF-8 with BOM or UTF-16 with or without BOM
		if (datasize == -1)
		{
			if (pszdata[0] != (char) 0x00 && pszdata[1] != (char) 0x00)
				if ((*reinterpret_cast<const DWORD *>(pszdata) & 0x00FFFFFF) == BOM_UTF8)		// Is UTF-8?
					pszdata += sizeof(char) * 3;
				else if ((*reinterpret_cast<const WORD *>(pszdata)) == BOM_UTF16)				// Is UTF-16?
					return reinterpret_cast<const wchar_t *>(pszdata + sizeof(char) * 2);
		}
		else // datasize >= 2
		{
			if (pszdata[1] != (char) 0x00)
				if ((*reinterpret_cast<const DWORD *>(pszdata) & 0x00FFFFFF) == BOM_UTF8)		// Is UTF-8?
					pszdata += sizeof(char) * 3;
				else if ((*reinterpret_cast<const WORD *>(pszdata)) == BOM_UTF16)				// Is UTF-16?
					return wstring(reinterpret_cast<const wchar_t *>(pszdata + sizeof(char) * 2),
																datasize - sizeof(char) * 2);

			if (IsTextUnicode(pszdata, datasize, NULL))
			{
				const wchar_t * pknowntypedata = reinterpret_cast<const wchar_t *>(pszdata);

				return wstring(pknowntypedata, datasize / sizeof(wchar_t) - 
								(pknowntypedata[datasize / sizeof(wchar_t) - 1] == L'\0' ? 1 : 0));
			}
		}
	}

	int requiredsize = MultiByteToWideChar(CP_UTF8, 0, pszdata, datasize, NULL, 0);
	unique_ptr<WCHAR> pbuffer(new WCHAR[requiredsize]);
	MultiByteToWideChar(CP_UTF8, 0, pszdata, datasize, pbuffer.get(), requiredsize);

	return wstring(pbuffer.get(), requiredsize - (datasize == -1 ? 1 : 0));
}

wstring MultiByte_toString(string &data)
{
	return MultiByte_toString(data.c_str());
}

wstring TimeDateStamp_toString(DWORD timedatestamp)
{
	WCHAR szBuffer[1024];

	if (_tctime32_s(szBuffer, (__time32_t *)&timedatestamp))
		return L"Invalid time/date stamp";

	return szBuffer;
}

wstring GUID_toString(GUID& guid)
{
	WCHAR szBuffer[1024];

	StringFromGUID2(guid, szBuffer, ARRAYSIZE(szBuffer));

	return szBuffer;
}

wstring DebugType_toString(DWORD value)
{
	switch (value)
	{
		case IMAGE_DEBUG_TYPE_COFF:	return L"COFF debug info";
		case IMAGE_DEBUG_TYPE_CODEVIEW: return L"CodeView debug info";
		case IMAGE_DEBUG_TYPE_FPO: return L"Frame pointer omission information";
		case IMAGE_DEBUG_TYPE_MISC: return L"Location of .DBG file";
		case IMAGE_DEBUG_TYPE_EXCEPTION: return L"Copy of .pdata section";
		case IMAGE_DEBUG_TYPE_FIXUP: return L"Reserved";
		case IMAGE_DEBUG_TYPE_OMAP_TO_SRC: return L"Mapping from RVA in image to RVA in source image";
		case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC: return L"Mapping from RVA in source image to RVA in image";
		case IMAGE_DEBUG_TYPE_BORLAND: return L"Reserved for Borland";
		case IMAGE_DEBUG_TYPE_RESERVED10: return L"Reserved";
		case IMAGE_DEBUG_TYPE_CLSID: return L"Reserved";
	}

	return L"Unknown/Invalid type";
}

wstring DebugMiscDataType_toString(DWORD value)
{
	switch (value)
	{
		case IMAGE_DEBUG_MISC_EXENAME: return L"Executable name";
	}

	return L"Unknown/Invalid type";
}

wstring ProcessorAffinityMask_toString(ULONGLONG value)
{
	wstring AffinityDesc = L"Use Processors: ";

	if (value == 0)
		return AffinityDesc += L"All";

	for (ULONGLONG i = 1; i < sizeof(ULONGLONG) * 8; i *= 2)
		if (TestFlag(value, i))
			AffinityDesc += QWORD_toString(i / 2) + L", ";

	return AffinityDesc;
}

wstring HeapFlags_toString(DWORD value)
{
	wstring FlagsDesc;

	if (value == 0)
		return L"No flags set";

	if (TestFlag(value, HEAP_CREATE_ENABLE_EXECUTE))
		FlagsDesc += L"Allow code execution in heap created at process start, ";

	if (TestFlag(value, HEAP_GENERATE_EXCEPTIONS))
		FlagsDesc += L"Raise exception on heap allocate failure, ";

	if (TestFlag(value, HEAP_NO_SERIALIZE))
		FlagsDesc += L"Serialized heap access not allowed, ";

	return RemoveTrailingCommaSpace(FlagsDesc);
}

wstring CorImageFlags_toString(DWORD value)
{
	wstring FlagsDesc;

	if (value == 0)
		return L"No flags set (Invalid image)";

	if (TestFlag(value, COMIMAGE_FLAGS_ILONLY))
		FlagsDesc += L"Managed code only, ";

	if (TestFlag(value, COMIMAGE_FLAGS_32BITREQUIRED))
		FlagsDesc += L"32-bit machine required, ";

	if (TestFlag(value, COMIMAGE_FLAGS_IL_LIBRARY))
		FlagsDesc += L"Managed code library, ";

	if (TestFlag(value, COMIMAGE_FLAGS_STRONGNAMESIGNED))
		FlagsDesc += L"Contains signed strong name, ";

	if (TestFlag(value, COMIMAGE_FLAGS_NATIVE_ENTRYPOINT))
		FlagsDesc += L"Entry point is native code, ";

	if (TestFlag(value, COMIMAGE_FLAGS_TRACKDEBUGDATA))
		FlagsDesc += L"Contains tracking debug data, ";

	return RemoveTrailingCommaSpace(FlagsDesc);
}

wstring CorHeapSizes_toString(BYTE value)
{
	wstring SizesDesc;

	SizesDesc = L"Index into '#Strings' heap is " + wstring(TestFlag(value, 0x1) ? L"4" : L"2") + L" bytes, ";
	SizesDesc += L"Index into '#GUID' heap is " + wstring(TestFlag(value, 0x2) ? L"4" : L"2") + L" bytes, ";
	SizesDesc += L"Index into '#Blob' heap is " + wstring(TestFlag(value, 0x4) ? L"4" : L"2") + L" bytes";
	
	return SizesDesc;
}

wstring CorMetadataTablesSummary_toString(ULONGLONG ValidTables, ULONGLONG Sorted, DWORD *pRowCounts)
{
	static const int NumOfKnownTables = 45;
	WCHAR szTableNames[][NumOfKnownTables] = 
	{
		L"Module", L"TypeRef", L"TypeDef", L"Unknown type 3", L"Field", L"Unknown type 5", 
		L"MethodDef", L"Unknown type 7", L"Param", L"InterfaceImpl", L"MemberRef", L"Constant", 
		L"CustomAttribute", L"FieldMarshal", L"DeclSecurity", L"ClassLayout", L"FieldLayout", 
		L"StandAloneSig", L"EventMap", L"Unknown type 19", L"Event", L"PropertyMap", 
		L"Unknown type 22", L"Property", L"MethodSemantics", L"MethodImpl", L"ModuleRef", 
		L"TypeSpec", L"ImplMap", L"FieldRVA", L"Unknown type 30", L"Unknown type 31", 
		L"Assembly", L"AssemblyProcessor", L"AssemblyOS", L"AssemblyRef", L"AssemblyRefProcessor", 
		L"AssemblyRefOS", L"File", L"ExportedType", L"ManifestResource", L"NestedClass", 
		L"GenericParam", L"MethodSpec",L"GenericParamConstr"
	};
	wstring Summary;
	unsigned long i = 0, n = 0;

	for (ULONGLONG j = 1; i < sizeof(ULONGLONG) * 8; j*=2, i++)
		if (TestFlag(ValidTables, j))
		{
			if (i < NumOfKnownTables)
				Summary += wstring(szTableNames[i]) + (TestFlag(Sorted, j) ? L"\tYes\t" : L"\tNo\t") + DWORD_toString(pRowCounts[n]) + L'\n';
			else
				Summary += L"Unknown type " + DWORD_toString(i + 1) + (TestFlag(Sorted, j) ? L"\tYes\t" : L"\tNo\t") + DWORD_toString(pRowCounts[n]) + L'\n';

			++n;
		}

	Summary += L"Total items: " + DWORD_toString(n) + L'\n';

	return Summary;
}

wstring CorGUIDs_toString(BYTE *pData, DWORD Size)
{
	wstring GUIDDesc;

	for (DWORD i = 0; i < Size; i += 16)
	{
		GUIDDesc += L"{" + DWORD_toString(*((DWORD *)pData), Hexadecimal).substr(2) + L'-';
		GUIDDesc += DWORD_toString(*((WORD *)(pData += sizeof(DWORD))), Hexadecimal).substr(2) + L'-';
		GUIDDesc += DWORD_toString(*((WORD *)(pData += sizeof(WORD))), Hexadecimal).substr(2) + L'-';
		GUIDDesc += QWORD_toString(*((ULONGLONG *)(pData += sizeof(WORD))), Hexadecimal).substr(2) + L"}\n";

		pData += sizeof(ULONGLONG);
	}

	return GUIDDesc;
}

wstring CorVTableType_toString(USHORT value)
{
	wstring TypeDesc;

	if (TestFlag(int(value), COR_VTABLE_32BIT))
		TypeDesc = L"Slots are 32-bits, ";

	if (TestFlag(int(value), COR_VTABLE_64BIT))
		TypeDesc += L"Slots are 64-bits, ";

	if (TestFlag(int(value), COR_VTABLE_FROM_UNMANAGED))
		TypeDesc += L"Unmanaged to managed transition code, ";

	if (TestFlag(int(value), COR_VTABLE_FROM_UNMANAGED_RETAIN_APPDOMAIN))
		TypeDesc += L"Unmanaged to managed transition code (retain app domain), ";

	if (TestFlag(int(value), COR_VTABLE_CALL_MOST_DERIVED))
		TypeDesc += L"Call most derived method token, ";

	if (TypeDesc.empty())
		return L"Unknown/Invalid type";

	return RemoveTrailingCommaSpace(TypeDesc);
}

wstring BaseRelocType_toString(WORD type)
{
	switch (type)
	{
	case IMAGE_REL_BASED_ABSOLUTE: return L"Base relocation skipped, used to pad a block";
	case IMAGE_REL_BASED_HIGH: return L"Add the high 16-bits of difference to 16-bit field at offset";
	case IMAGE_REL_BASED_LOW: return L"Add the low 16-bits of difference to 16-bit field at offset";
	case IMAGE_REL_BASED_HIGHLOW: return L"Apply all 32-bits of difference to 32-bit field at offset";
	case IMAGE_REL_BASED_HIGHADJ: return L"Two part relocation, add the high 16-bits of difference to 16-bit field at offset, "
										 L"the following relocation handles low 16-bit relocation";
	case IMAGE_REL_BASED_MIPS_JMPADDR: return L"Apply to a MIPS jump instruction";
	case 6: return L"Reserved type 6, must be zero";
	case 7: return L"Reserved type 7, must be zero";
	case IMAGE_REL_BASED_MIPS_JMPADDR16: return L"Apply to MIPS16 jump instruction";
	case IMAGE_REL_BASED_DIR64: return L"Apply the difference to 64-bit field at offset";
	}

	return L"Unknown/Invalid type";
}

wstring ResourceID_toString(WORD id)
{
	switch (id)
	{
		case 1: return L"Cursor";
		case 2: return L"Bitmap";
		case 3: return L"Icon";
		case 4: return L"Menu";
		case 5: return L"Dialog";
		case 6: return L"String";
		case 7: return L"Font Directory";
		case 8: return L"Font";
		case 9: return L"Accelerator";
		case 10: return L"Application Defined Raw Data";
		case 11: return L"Message Table";
		case 12: return L"Cursor Group";
		case 14: return L"Icon Group";
		case 16: return L"Version";
		case 17: return L"Dialog Include";
		case 19: return L"Plug Play";
		case 20: return L"VxD";
		case 21: return L"Animated Cursor";
		case 22: return L"Animated Icon";
		case 23: return L"HTML";
		case 24: return L"Manifest";
		case 240: return L"DLGINIT";
		case 241: return L"Toolbar";
	}

	return L"Custom";
}