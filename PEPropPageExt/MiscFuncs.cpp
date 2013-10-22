#include "MiscFuncs.h"
#include <algorithm>


/*void DebugWrite(char *szMsg)
{
	DWORD byteswritten;
	DWORD a;
	HANDLE hFile = CreateFile(_T("C:\\TestFolder\\dbg.txt"), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(hFile, 0, NULL, FILE_END);
	WriteFile(hFile, szMsg, strlen(szMsg), &byteswritten, NULL);
	CloseHandle(hFile);
}*/

void LogError(tstring Info, bool bCritical)
{
	MessageBox(NULL, (PTCHAR) Info.c_str(), (bCritical ? NULL : _T("Information")), 
				MB_OK | MB_APPLMODAL | (bCritical ? MB_ICONERROR : MB_ICONINFORMATION));
}

void tstring_ReplaceAll(tstring& Source, TCHAR cToFind, TCHAR cToReplace)
{
	size_t lastfindpos = -1;

	while ((lastfindpos = Source.find(cToFind, lastfindpos + 1)) != tstring::npos)
		Source.replace(lastfindpos, 1, 1, cToReplace);
}

tstring Integer_toString(int value)
{
	TCHAR buffer[16];

	_stprintf_s(buffer, 16, _T("%i"), value);

	return buffer;
}

tstring DWORD_toString(DWORD value, FORMATTYPE type)
{
	TCHAR buffer[16];

	_stprintf_s(buffer, 16, (type == Decimal ? _T("%u") : _T("%X")), value);

	return (type == Decimal ? _T("") : _T("0x")) + tstring(buffer);
}

tstring QWORD_toString(ULONGLONG value, FORMATTYPE type)
{
	TCHAR buffer[24];

	_stprintf_s(buffer, 24, (type == Decimal ? _T("%I64u") : _T("%I64X")), value);

	return (type == Decimal ? _T("") : _T("0x")) + tstring(buffer);
}

tstring Signature_toString(DWORD value)
{
	return MultiByte_toString((char *) &value, true, sizeof(DWORD));
}

tstring float_toString(float value)//, int decimalplaces = 2)
{
	TCHAR buffer[16];

	_stprintf_s(buffer, 16, _T("%.2f"), value);

	return buffer;
}

LPTSTR LEOrdering_toString(BYTE order)
{
	return (order == 0 ? _T("Little Endian") : _T("Big Endian"));
}

LPTSTR LECPUtype_toString(WORD type)
{
	switch (type)
	{
		case 0x0001: return _T("Intel 80286 or upwardly compatible");
		case 0x0002: return _T("Intel 80386 or upwardly compatible");
		case 0x0003: return _T("Intel 80486 or upwardly compatible");
		case 0x0004: return _T("Intel 80586 or upwardly compatible");
		case 0x0020: return _T("Intel i860 (N10) or compatible");
		case 0x0021: return _T("Intel \"N11\" or compatible");
		case 0x0040: return _T("MIPS Mark I ( R2000, R3000) or compatible");
		case 0x0041: return _T("MIPS Mark II ( R6000 ) or compatible");
		case 0x0042: return _T("MIPS Mark III ( R4000 ) or compatible");
	}

	return _T("Unknown CPU type");
}

LPTSTR LEOStype_toString(WORD type)
{
	switch (type)
	{
		case 0x0001: return _T("OS/2");
		case 0x0002: return _T("Windows");
		case 0x0003: return _T("DOS 4.x");
		case 0x0004: return _T("Windows 386");
	}

	return _T("Unknown OS type");
}

tstring LEModuletypeflags_toString(DWORD value)
{
	// NOTE: Flags in this function are from www.textfiles.com/programming/FORMATS/lxexe.txt
	tstring temp;

	if (value == 0)
		return _T("");

	DWORD flag = 0x01L;

	if (TestFlag(value, 0x01L))
		temp += _T("Reserved(0x01), ");
	else if (TestFlag(value, 0x02L))
		temp += _T("Reserved(0x02), ");

	if (!TestFlag(value, 0x04L))
		temp += _T("Global Initialization (DLL only), ");
	else
		temp += _T("Per-Process Initialization (DLL only), ");

	if (TestFlag(value, 0x08L))
		temp += _T("Reserved(0x08), ");
	else if (TestFlag(value, 0x10L))
		temp += _T("Has internal relocation info, ");
	else if (TestFlag(value, 0x20L))
		temp += _T("Has external relocation info, ");
	else if (TestFlag(value, 0x40L))
		temp += _T("Reserved(0x40), ");
	else if (TestFlag(value, 0x80L))
		temp += _T("Reserved(0x80), ");
	else if (TestFlag(value, 0x100L))
		temp += _T("Incompatible with PM windowing, ");
	else if (TestFlag(value, 0x200L))
		temp += _T("Compatible with PM windowing, ");
	else if (TestFlag(value, 0x300L))
		temp += _T("Uses PM windowing API, ");
	else if (TestFlag(value, 0x400L))
		temp += _T("Reserved(0x400), ");
	else if (TestFlag(value, 0x800L))
		temp += _T("Reserved(0x800), ");
	else if (TestFlag(value, 0x1000L))
		temp += _T("Reserved(0x1000), ");
	else if (TestFlag(value, 0x2000L))
		temp += _T("Module cannot be loaded, ");
	else if (TestFlag(value, 0x4000L))
		temp += _T("Reserved(0x4000), ");
	else if (TestFlag(value, 0x8000L))
		temp += _T("Module is a DLL, ");
	else if (TestFlag(value, 0x18000L))
		temp += _T("Protected Memory Library, ");
	else if (TestFlag(value, 0x20000L))
		temp += _T("Physical Device Driver, ");
	else if (TestFlag(value, 0x28000L))
		temp += _T("Virtual Device Driver, ");
	else if (TestFlag(value, 0x40000000L))
		temp += _T("Per-process Library Termination, ");

	return RemoveTrailingSpace(temp);
}

tstring VersionNums_toString(WORD major, WORD minor)
{
	return DWORD_toString(major) + _T(".") + DWORD_toString(minor);
}

tstring MagicNum_toString(DWORD num)
{
	switch (num)
	{
	case IMAGE_NT_OPTIONAL_HDR32_MAGIC:	return _T("32-bit Image");
	case IMAGE_NT_OPTIONAL_HDR64_MAGIC:	return _T("64-bit Image");
	case IMAGE_ROM_OPTIONAL_HDR_MAGIC:	return _T("ROM Image");
	}

	return _T("Unknown value");
}

tstring OSIdString(WORD major, WORD minor)
{
	/* Windows OS Ids from: http://msdn.microsoft.com/en-us/library/windows/desktop/ms724832%28v=vs.85%29.aspx */

	if(major < 5)
		return _T("Earlier than Windows 2000");

	switch (major)
	{
	case 5:
		switch (minor)
		{
		case 0:	return _T("Windows 2000");
		case 1:	return _T("Windows XP");
		case 2: return _T("Windows XP 64-Bit Edition, Windows Server 2003, Windows Server 2003 R2");
		}

		break;

	case 6:
		switch (minor)
		{
		case 0: return _T("Windows Vista, Windows Server 2008");
		case 1:	return _T("Windows Server 2008 R2, Windows 7");
		case 2: return _T("Windows Server 2012, Windows 8");
		}
	}
	
	return _T("Unknown or invalid OS version number");
}

tstring FormattedBytesSize(ULONGLONG size)
{
	tstring temp = _T("bytes");

	if (size < 1024)
		return temp;
	else if (size < 1024 * 1024)
		return temp + _T(" (") + float_toString(size / 1024.0f) + _T(" KB)");
	else
		return temp + _T(" (") + float_toString(size / (1024.0f * 1024.0f)) + _T(" MB)");
}

tstring MachineType_toString(WORD value)
{
	switch (value)
	{
	case IMAGE_FILE_MACHINE_UNKNOWN: return _T("Any machine");
	case IMAGE_FILE_MACHINE_AM33: return _T("Matsushita AM33");
	case IMAGE_FILE_MACHINE_AMD64: return _T("Intel/AMD x64");
	case IMAGE_FILE_MACHINE_ARM: return _T("ARM little endian");
	case IMAGE_FILE_MACHINE_EBC: return _T("EFI byte code");
	case IMAGE_FILE_MACHINE_I386: return _T("Intel x86");
	case IMAGE_FILE_MACHINE_IA64: return _T("Intel Itanium");
	case IMAGE_FILE_MACHINE_M32R: return _T("Mitsubishi M32R little endian");
	case IMAGE_FILE_MACHINE_MIPS16:	return _T("MIPS16");
	case IMAGE_FILE_MACHINE_MIPSFPU: return _T("MIPS with FPU");
	case IMAGE_FILE_MACHINE_MIPSFPU16: return _T("MIPS16 with FPU");
	case IMAGE_FILE_MACHINE_POWERPC: return _T("Power PC little endian");
	case IMAGE_FILE_MACHINE_POWERPCFP: return _T("Power PC with floating point support");
	case IMAGE_FILE_MACHINE_R4000: return _T("MIPS little endian");
	case IMAGE_FILE_MACHINE_SH3: return _T("Hitachi SH3");
	case IMAGE_FILE_MACHINE_SH3DSP: return _T("Hitachi SH3 DSP");
	case IMAGE_FILE_MACHINE_SH4: return _T("Hitachi SH4");
	case IMAGE_FILE_MACHINE_SH5: return _T("Hitachi SH5");
	case IMAGE_FILE_MACHINE_THUMB: return _T("Thumb");
	case IMAGE_FILE_MACHINE_WCEMIPSV2: return _T("MIPS little endian WCE v2");
	}

	return _T("Unknown machine type");
}

tstring ImageCharacteristics_toString(DWORD value)
{
	tstring temp;

	if (value == 0)
		return _T("");

	if (TestFlag(value, IMAGE_FILE_RELOCS_STRIPPED))
		temp += _T("No relocation info (Will only load at preferred address), ");

	if(TestFlag(value, IMAGE_FILE_EXECUTABLE_IMAGE))
		temp += _T("Executable, ");

	if(TestFlag(value, IMAGE_FILE_LINE_NUMS_STRIPPED))
		temp += _T("COFF Line no. stripped, ");

	if(TestFlag(value, IMAGE_FILE_LOCAL_SYMS_STRIPPED))
		temp += _T("COFF symbol table stripped, ");

	if(TestFlag(value, IMAGE_FILE_AGGRESIVE_WS_TRIM))
		temp += _T("Aggressively trim Working Set (Obsolete), ");

	if(TestFlag(value, IMAGE_FILE_LARGE_ADDRESS_AWARE))
		temp += _T("2GB+ addr aware, ");

	if(TestFlag(value, IMAGE_FILE_BYTES_REVERSED_LO))
		temp += _T("Little endian (Deprecated), ");

	if(TestFlag(value, IMAGE_FILE_32BIT_MACHINE))
		temp += _T("For 32-bit machine, ");

	if(TestFlag(value, IMAGE_FILE_DEBUG_STRIPPED))
		temp += _T("Debug info in separate file, ");

	if(TestFlag(value, IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP))
		temp += _T("If in removable media - copy and run from swap, ");

	if(TestFlag(value, IMAGE_FILE_NET_RUN_FROM_SWAP))
		temp += _T("If on network - copy and run from swap, ");

	if(TestFlag(value, IMAGE_FILE_SYSTEM))
		temp += _T("System file, ");

	if(TestFlag(value, IMAGE_FILE_DLL))
		temp += _T("DLL file, ");

	if(TestFlag(value, IMAGE_FILE_UP_SYSTEM_ONLY))
		temp += _T("For uniprocessor machine only, ");

	if(TestFlag(value, IMAGE_FILE_BYTES_REVERSED_HI))
		temp += _T("Big endian (Deprecated), ");

	return RemoveTrailingSpace(temp);
}

tstring SubsystemID_toString(WORD value)
{
	switch (value)
	{
	case IMAGE_SUBSYSTEM_UNKNOWN:
		return _T("Unknown subsystem");

	case IMAGE_SUBSYSTEM_NATIVE:
		return _T("No subsystem required (device drivers and native system processes)");

	case IMAGE_SUBSYSTEM_WINDOWS_GUI:
		return _T("GUI subsystem");

	case IMAGE_SUBSYSTEM_WINDOWS_CUI:
		return _T("Character mode UI subsystem");

	case IMAGE_SUBSYSTEM_OS2_CUI:
		return _T("OS/2 CUI subsystem");

	case IMAGE_SUBSYSTEM_POSIX_CUI:
		return _T("POSIX CUI subsystem");

	case IMAGE_SUBSYSTEM_WINDOWS_CE_GUI:
		return _T("Windows CE system");
		
	case IMAGE_SUBSYSTEM_EFI_APPLICATION:
		return _T("Extensible Firmware Interface (EFI) application");

	case IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
		return _T("EFI driver with boot services");

	case IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
		return _T("EFI driver with run-time services");

	case IMAGE_SUBSYSTEM_EFI_ROM:
		return _T("EFI ROM image");

	case IMAGE_SUBSYSTEM_XBOX:
		return _T("XBOX system");
		
	case IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION:
		return _T("Boot application");
	}

	return _T("Unknown value");
}

tstring DllCharacteristics_toString(WORD value)
{
	tstring temp;

	if (value == 0)
		return _T("");

	if (TestFlag(value, 0x0001))
		temp += _T("Reserved(0x0001), ");

	if (TestFlag(value, 0x0002))
		temp += _T("Reserved(0x0002), ");

	if (TestFlag(value, 0x0004))
		temp += _T("Reserved(0x0004), ");

	if (TestFlag(value, 0x0008))
		temp += _T("Reserved(0x0008), ");

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE))
		temp += _T("Has relocation section, ");

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY))
		temp += _T("Code integrity checks enforced, ");

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_NX_COMPAT))
		temp += _T("DEP compatible, ");

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_NO_ISOLATION))
		temp += _T("Not to be isolated, ");

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_NO_SEH))
		temp += _T("No SEH, ");

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_NO_BIND))
		temp += _T("No Binding, ");

	if (TestFlag(value, 0x1000))
		temp += _T("Reserved(0x1000), ");

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_WDM_DRIVER))
		temp += _T("Is a Windows Driver Model driver, ");

	if (TestFlag(value, IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE))
		temp += _T("Terminal Server aware, ");

	return RemoveTrailingSpace(temp);
}

tstring ProperSectionName(BYTE szRawSectionName[IMAGE_SIZEOF_SHORT_NAME])
{
	tstring Buffer;
	tstring BufferChar;

	for (int i = 0; i < IMAGE_SIZEOF_SHORT_NAME; i++)
	{
		BufferChar = MultiByte_toString((char *) &szRawSectionName[i], true, 1);

		if (BufferChar.at(0) == _T('\0'))
			break;

		Buffer += BufferChar;
	}

	return Buffer;
}

tstring SectionCharacteristics_toString(DWORD value)
{
	tstring temp;

	if (value == 0)
		return _T("Reserved(0x00000000)");

	if (TestFlag(value, 0x00000001))
		temp += _T("Reserved(0x00000001), ");

	if (TestFlag(value, 0x00000002))
		temp += _T("Reserved(0x00000002), ");

	if (TestFlag(value, 0x00000004))
		temp += _T("Reserved(0x00000004), ");

	if (TestFlag(value, IMAGE_SCN_TYPE_NO_PAD))
		temp += _T("No padding to next boundary, ");

	if (TestFlag(value, 0x00000010))
		temp += _T("Reserved(0x00000010), ");

	if (TestFlag(value, IMAGE_SCN_CNT_CODE))
		temp += _T("Has code, ");

	if (TestFlag(value, IMAGE_SCN_CNT_INITIALIZED_DATA))
		temp += _T("Initialized data, ");

	if (TestFlag(value, IMAGE_SCN_CNT_UNINITIALIZED_DATA))
		temp += _T("Uninitialized data, ");

	if (TestFlag(value, 0x00000100))
		temp += _T("Reserved(0x00000100), ");

	if (TestFlag(value, IMAGE_SCN_LNK_INFO))
		temp += _T("Comments or other info, ");

	if (TestFlag(value, 0x00000400))
		temp += _T("Reserved(0x00000400), ");

	if (TestFlag(value, IMAGE_SCN_LNK_REMOVE))
		temp += _T("Should be removed during linking, ");

	if (TestFlag(value, IMAGE_SCN_LNK_COMDAT))
		temp += _T("COMDAT data, ");

	if (TestFlag(value, IMAGE_SCN_GPREL))
		temp += _T("Data referenced through global ptrs, ");

	if (TestFlag(value, 0x00020000))
		temp += _T("ARM Thumb code/Reserved(0x00020000) for other machine types, ");

	if (TestFlag(value, 0x00040000))
		temp += _T("Reserved(0x00040000), ");

	if (TestFlag(value, 0x00080000))
		temp += _T("Reserved(0x00080000), ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_1BYTES))
		temp += _T("Align data on 1-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_2BYTES))
		temp += _T("Align data on 2-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_4BYTES))
		temp += _T("Align data on 4-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_8BYTES))
		temp += _T("Align data on 8-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_16BYTES))
		temp += _T("Align data on 16-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_32BYTES))
		temp += _T("Align data on 32-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_64BYTES))
		temp += _T("Align data on 64-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_128BYTES))
		temp += _T("Align data on 128-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_256BYTES))
		temp += _T("Align data on 256-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_512BYTES))
		temp += _T("Align data on 512-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_1024BYTES))
		temp += _T("Align data on 1024-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_2048BYTES))
		temp += _T("Align data on 2048-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_4096BYTES))
		temp += _T("Align data on 4096-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_ALIGN_8192BYTES))
		temp += _T("Align data on 8192-byte boundary, ");

	if (TestFlag(value, IMAGE_SCN_LNK_NRELOC_OVFL))
		temp += _T("Extended relocations, ");

	if (TestFlag(value, IMAGE_SCN_MEM_DISCARDABLE))
		temp += _T("No need to load, ");

	if (TestFlag(value, IMAGE_SCN_MEM_NOT_CACHED))
		temp += _T("Non-cacheable, ");

	if (TestFlag(value, IMAGE_SCN_MEM_NOT_PAGED))
		temp += _T("Non-pagable, ");

	if (TestFlag(value, IMAGE_SCN_MEM_SHARED))
		temp += _T("Sharable, ");

	if (TestFlag(value, IMAGE_SCN_MEM_EXECUTE))
		temp += _T("Executable, ");

	if (TestFlag(value, IMAGE_SCN_MEM_READ))
		temp += _T("Readable, ");

	if (TestFlag(value, IMAGE_SCN_MEM_WRITE))
		temp += _T("Writable, ");

	return RemoveTrailingSpace(temp);
}

tstring StandardSectionNameAnnotation(tstring name)
{
	transform(name.begin(), name.end(), name.begin(), ::tolower);

	if (name == _T(".text"))
		return _T("Code Section");

	if (name == _T(".debug"))
		return _T("Debug Section");

	if (name == _T(".drectve"))
		return _T("Directive Section");

	if (name == _T(".edata"))
		return _T("Export Data Section");

	if (name == _T(".idata"))
		return _T("Import Data Section");

	if (name == _T(".data"))
		return _T("Initialized Modifiable Data Section");

	if (name == _T(".reloc"))
		return _T("Base Relocation Fixes Section");

	if (name == _T(".tls"))
		return _T("Thread Local Storage Section");

	if (name == _T(".rsrc"))
		return _T("Resource Data Section");

	if (name == _T(".pdata"))
		return _T("Function Pointer Table Section");

	if ((name == _T(".rdata")) || (name == _T(".rodata")))
		return _T("Initialized Constant Data Section");

	if (name == _T(".bss"))
		return _T("Static Data Section");

	return _T("");
}

tstring LCIDtoLangName(WORD lcid)
{
	int BufferSize = LCIDToLocaleName(lcid, NULL, 0, 0);

	if (BufferSize)
	{
		LPTSTR szBuffer1 = new TCHAR[BufferSize];
		tstring szBuffer2;

		LCIDToLocaleName(lcid, szBuffer1, BufferSize, 0);
		szBuffer2 = szBuffer1;
		delete szBuffer1;

		return szBuffer2;
	}

	return _T("Unknown language");
}

tstring ExceptionArch_toString(DWORD val)
{
	return val ? _T("Consists of 32-bit instructions") : _T("Consists of 64-bit instructions");
}

tstring ExceptionFlag_toString(DWORD flag)
{
	return flag ? _T("Exception handler exists") : _T("No exception handler exists");
}

tstring MultiByte_toString(const char *pszdata, bool bisANSI, int datasize)
{
	// This function allows ANSI, UTF-8 (with or without BOM) and UTF-16 (with or without BOM,
	// for UTF-16 in UNICODE mode the datasize must be given) to be converted into either 
	// 'string' or 'wstring' according to whether 'UNICODE' is defined or not.
	// NOTE: Other code pages and UTF-32 is not supported.

	if (pszdata == NULL || datasize < -1 || datasize == 0)
		return _T("");

#ifndef UNICODE
	// If it's already ANSI, nothing to do so just return the pointer
	if (bisANSI || datasize == 1)
		if (datasize == -1)
			return pszdata;
		else
			return tstring(pszdata, datasize);

	// If not ANSI (though UTF-8 without BOM and ANSI is interchangeable), find which
	// UNICODE encoding it is and convert it to ANSI

	// Make sure that at least the first three bytes of 'pszdata' is readable
	// and any Byte order mark is removed before processing
	if (datasize == -1)
	{
		if (pszdata[0] != (char) 0x00 && pszdata[1] != (char) 0x00)
			if ((*reinterpret_cast<const DWORD *>(pszdata) & 0x00FFFFFF) == BOM_UTF8)		// Is UTF-8?
				return pszdata + sizeof(char) * 3;
			else if ((*reinterpret_cast<const WORD *>(pszdata)) == BOM_UTF16)				// Is UTF-16?
				pszdata += sizeof(char) * 2;
	}
	else // datasize >= 2
	{
		if (pszdata[1] != (char) 0x00)
			if ((*reinterpret_cast<const DWORD *>(pszdata) & 0x00FFFFFF) == BOM_UTF8)		// Is UTF-8?
				return tstring(pszdata + sizeof(char) * 3, datasize - sizeof(char) * 3);
			else if ((*reinterpret_cast<const WORD *>(pszdata)) == BOM_UTF16)				// Is UTF-16?
				pszdata += sizeof(char) * 2;

		datasize /= sizeof(wchar_t);	// Calculate total no. of characters
	}	

	int requiredsize = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH) pszdata, datasize, NULL, 0, NULL, NULL);
	unique_ptr<char> pbuffer(new char[requiredsize]);
	WideCharToMultiByte(CP_UTF8, 0, (LPCWCH) pszdata, datasize, pbuffer.get(), requiredsize, NULL, NULL);
#else
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
					return tstring(reinterpret_cast<const wchar_t *>(pszdata + sizeof(char) * 2),
																datasize - sizeof(char) * 2);

			if (IsTextUnicode(pszdata, datasize, NULL))
			{
				const wchar_t * pknowntypedata = reinterpret_cast<const wchar_t *>(pszdata);

				return tstring(pknowntypedata, datasize / sizeof(wchar_t) - 
								(pknowntypedata[datasize / sizeof(wchar_t) - 1] == L'\0' ? 1 : 0));
			}
		}
	}

	int requiredsize = MultiByteToWideChar(CP_UTF8, 0, pszdata, datasize, NULL, 0);
	unique_ptr<wchar_t> pbuffer(new wchar_t[requiredsize]);
	MultiByteToWideChar(CP_UTF8, 0, pszdata, datasize, pbuffer.get(), requiredsize);
#endif

	return tstring(pbuffer.get(), requiredsize - (datasize == -1 ? 1 : 0));
}

tstring MultiByte_toString(string &data)
{
	return MultiByte_toString(data.c_str());
}

tstring TimeDateStamp_toString(DWORD timedatestamp)
{
	TCHAR buffer[32];

	if (_tctime32_s(buffer, (__time32_t *) &timedatestamp))
		return _T("Invalid time/date stamp");

	return buffer;
}

tstring GUID_toString(GUID& guid)
{
	TCHAR buffer[64];

	_stprintf(buffer, _T("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"), 
				guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], 
				guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	return buffer;
}

tstring DebugType_toString(DWORD value)
{
	switch (value)
	{
	case IMAGE_DEBUG_TYPE_COFF:	return _T("COFF debug info");
	case IMAGE_DEBUG_TYPE_CODEVIEW: return _T("CodeView debug info");
	case IMAGE_DEBUG_TYPE_FPO: return _T("Frame pointer omission information");
	case IMAGE_DEBUG_TYPE_MISC: return _T("Location of .DBG file");
	case IMAGE_DEBUG_TYPE_EXCEPTION: return _T("Copy of .pdata section");
	case IMAGE_DEBUG_TYPE_FIXUP: return _T("Reserved");
	case IMAGE_DEBUG_TYPE_OMAP_TO_SRC: return _T("Mapping from RVA in image to RVA in source image");
	case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC: return _T("Mapping from RVA in source image to RVA in image");
	case IMAGE_DEBUG_TYPE_BORLAND: return _T("Reserved for Borland");
	case IMAGE_DEBUG_TYPE_RESERVED10: return _T("Reserved");
	case IMAGE_DEBUG_TYPE_CLSID: return _T("Reserved");
	}

	return _T("Unknown/Invalid type");
}

tstring DebugMiscDataType_toString(DWORD value)
{
	switch (value)
	{
	case IMAGE_DEBUG_MISC_EXENAME: return _T("Executable name");
	}

	return _T("Unknown/Invalid type");
}

tstring ProcessorAffinityMask_toString(ULONGLONG value)
{
	tstring temp = _T("Use Processors: ");

	if (value == 0)
		return temp += _T("All");

	for (ULONGLONG i = 1; i < sizeof(ULONGLONG) * 8; i*=2)
		if (TestFlag(value, i))
			temp += QWORD_toString(i / 2) + _T(", ");

	return temp;
}

tstring HeapFlags_toString(DWORD value)
{
	tstring temp;

	if (value == 0)
		return _T("No flags set");

	if (TestFlag(value, HEAP_CREATE_ENABLE_EXECUTE))
		temp += _T("Allow code execution in heap created at process start, ");

	if (TestFlag(value, HEAP_GENERATE_EXCEPTIONS))
		temp += _T("Raise exception on heap allocate failure, ");

	if (TestFlag(value, HEAP_NO_SERIALIZE))
		temp += _T("Serialized heap access not allowed, ");

	return RemoveTrailingSpace(temp);
}

tstring CorImageFlags_toString(DWORD value)
{
	tstring temp;

	if (value == 0)
		return _T("No flags set (Invalid image)");

	if (TestFlag(value, COMIMAGE_FLAGS_ILONLY))
		temp += _T("Managed code only, ");

	if (TestFlag(value, COMIMAGE_FLAGS_32BITREQUIRED))
		temp += _T("32-bit machine required, ");

	if (TestFlag(value, COMIMAGE_FLAGS_IL_LIBRARY))
		temp += _T("Managed code library, ");

	if (TestFlag(value, COMIMAGE_FLAGS_STRONGNAMESIGNED))
		temp += _T("Contains signed strong name, ");

	if (TestFlag(value, COMIMAGE_FLAGS_NATIVE_ENTRYPOINT))
		temp += _T("Entry point is native code, ");

	if (TestFlag(value, COMIMAGE_FLAGS_TRACKDEBUGDATA))
		temp += _T("Contains tracking debug data, ");

	return RemoveTrailingSpace(temp);
}

tstring CorHeapSizes_toString(BYTE value)
{
	tstring temp;

	temp += _T("Index into '#Strings' heap is ") + tstring(TestFlag(value, 0x1) ? _T("4") : _T("2")) + _T(" bytes, ");
	temp += _T("Index into '#GUID' heap is ") + tstring(TestFlag(value, 0x2) ? _T("4") : _T("2")) + _T(" bytes, ");
	temp += _T("Index into '#Blob' heap is ") + tstring(TestFlag(value, 0x4) ? _T("4") : _T("2")) + _T(" bytes");
	
	return temp;
}

tstring CorMetadataTablesSummary_toString(ULONGLONG ValidTables, ULONGLONG Sorted, DWORD *pRowCounts)
{
	static const int NumOfKnownTables = 45;
	TCHAR TableNames[][NumOfKnownTables] = {_T("Module"), _T("TypeRef"), _T("TypeDef"), _T("Unknown type 3"), _T("Field"), _T("Unknown type 5"), 
											_T("MethodDef"), _T("Unknown type 7"), _T("Param"), _T("InterfaceImpl"), _T("MemberRef"), _T("Constant"), 
											_T("CustomAttribute"), _T("FieldMarshal"), _T("DeclSecurity"), _T("ClassLayout"), _T("FieldLayout"), 
											_T("StandAloneSig"), _T("EventMap"), _T("Unknown type 19"), _T("Event"), _T("PropertyMap"), 
											_T("Unknown type 22"), _T("Property"), _T("MethodSemantics"), _T("MethodImpl"), _T("ModuleRef"), 
											_T("TypeSpec"), _T("ImplMap"), _T("FieldRVA"), _T("Unknown type 30"), _T("Unknown type 31"), 
											_T("Assembly"), _T("AssemblyProcessor"), _T("AssemblyOS"), _T("AssemblyRef"), _T("AssemblyRefProcessor"), 
											_T("AssemblyRefOS"), _T("File"), _T("ExportedType"), _T("ManifestResource"), _T("NestedClass"), 
											_T("GenericParam"), _T("MethodSpec"),_T("GenericParamConstr")};
	tstring temp;
	unsigned long i = 0, n = 0;

	for (ULONGLONG j = 1; i < sizeof(ULONGLONG) * 8; j*=2, i++)
		if (TestFlag(ValidTables, j))
		{
			if (i < NumOfKnownTables)
				temp += tstring(TableNames[i]) + (TestFlag(Sorted, j) ? _T("\tYes\t") : _T("\tNo\t")) + DWORD_toString(pRowCounts[n]) + _T("\n");
			else
				temp += _T("Unknown type ") + DWORD_toString(i + 1) + (TestFlag(Sorted, j) ? _T("\tYes\t") : _T("\tNo\t")) + DWORD_toString(pRowCounts[n]) + _T("\n");

			n++;
		}

	temp += _T("Total items: ") + DWORD_toString(n) + _T("\n");

	return temp;
}

tstring CorGUIDs_toString(BYTE *pData, DWORD Size)
{
	tstring temp;

	for (DWORD i = 0; i < Size; i += 16)
	{
		temp += _T("{") + DWORD_toString(*((DWORD *) pData), Hexadecimal).substr(2) + _T("-");
		temp += DWORD_toString(*((WORD *) (pData += sizeof(DWORD))), Hexadecimal).substr(2) + _T("-");
		temp += DWORD_toString(*((WORD *) (pData += sizeof(WORD))), Hexadecimal).substr(2) + _T("-");
		temp += QWORD_toString(*((ULONGLONG *) (pData += sizeof(WORD))), Hexadecimal).substr(2) + _T("}\n");

		pData += sizeof(ULONGLONG);
	}

	return temp;
}

tstring CorVTableFlags_toString(USHORT value)
{
	tstring temp;

	if (value == 0)
		return _T("No flags set");

	if (TestFlag(value, COR_VTABLE_32BIT))
		temp += _T("Slots are 32-bits, ");

	if (TestFlag(value, COR_VTABLE_64BIT))
		temp += _T("Slots are 64-bits, ");

	if (TestFlag(value, COR_VTABLE_FROM_UNMANAGED))
		temp += _T("Transition from unmanaged to managed code, ");

	if (TestFlag(value, COR_VTABLE_CALL_MOST_DERIVED))
		temp += _T("Call most derived method described by the token (only valid for virtual methods), ");

	return RemoveTrailingSpace(temp);
}

tstring BaseRelocType_toString(WORD type)
{
	switch (type)
	{
	case IMAGE_REL_BASED_ABSOLUTE: return _T("Base relocation skipped, used to pad a block");
	case IMAGE_REL_BASED_HIGH: return _T("Add the high 16-bits of difference to 16-bit field at offset");
	case IMAGE_REL_BASED_LOW: return _T("Add the low 16-bits of difference to 16-bit field at offset");
	case IMAGE_REL_BASED_HIGHLOW: return _T("Apply all 32-bits of difference to 32-bit field at offset");
	case IMAGE_REL_BASED_HIGHADJ: return _T("Two part relocation, add the high 16-bits of difference to 16-bit field at offset, ")
											_T("the following relocation handles low 16-bit relocation");
	case IMAGE_REL_BASED_MIPS_JMPADDR: return _T("Apply to a MIPS jump instruction");
	case 6: return _T("Reserved type 6, must be zero");
	case 7: return _T("Reserved type 7, must be zero");
	case IMAGE_REL_BASED_MIPS_JMPADDR16: return _T("Apply to MIPS16 jump instruction");
	case IMAGE_REL_BASED_DIR64: return _T("Apply the difference to 64-bit field at offset");
	}

	return _T("Unknown/Invalid type");
}