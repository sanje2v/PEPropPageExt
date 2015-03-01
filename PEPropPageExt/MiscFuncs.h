#pragma once

#include "stdafx.h"
#include <string>
#include <algorithm>


enum FORMATTYPE { Decimal, Hexadecimal };

void LogError(std::wstring info, bool bCritical = false);
void wstring_ReplaceAll(wstring& source, WCHAR strtofind, WCHAR strtoreplace);
wstring Integer_toString(int value);
wstring BYTE_toString(BYTE, FORMATTYPE type = Decimal, bool fixedaligned = false, bool prefix = true);
wstring WORD_toString(WORD, FORMATTYPE type = Decimal, bool fixedaligned = false, bool prefix = true);
wstring DWORD_toString(DWORD, FORMATTYPE type = Decimal, bool fixedaligned = false, bool prefix = true);
wstring QWORD_toString(ULONGLONG, FORMATTYPE type = Decimal, bool fixedaligned = false, bool prefix = true);
wstring Signature_toString(DWORD);
wstring float_toString(float);
LPWSTR LEOrdering_toString(BYTE);
LPWSTR LECPUtype_toString(WORD);
LPWSTR LEOStype_toString(WORD);
wstring LEModuletypeflags_toString(DWORD);
wstring VersionNums_toString(WORD, WORD);
wstring MagicNum_toString(DWORD);
wstring OSId_toString(WORD, WORD);
wstring FormattedBytesSize(ULONGLONG);
wstring MachineType_toString(WORD);
wstring ImageCharacteristics_toString(DWORD);
wstring SubsystemID_toString(WORD);
wstring DllCharacteristics_toString(WORD);
wstring ProperSectionName(BYTE [IMAGE_SIZEOF_SHORT_NAME]);
wstring SectionCharacteristics_toString(DWORD);
wstring StandardSectionNameAnnotation(wstring);
wstring ExceptionArch_toString(DWORD);
wstring ExceptionFlag_toString(DWORD);
wstring LCID_toLocaleName(WORD);
wstring MultiByte_toString(const char *pszdata, bool bisANSI = true, int datasize = -1);
wstring MultiByte_toString(string &);
wstring TimeDateStamp_toString(DWORD);
wstring GUID_toString(GUID&);
wstring DebugType_toString(DWORD);
wstring DebugMiscDataType_toString(DWORD);
wstring ProcessorAffinityMask_toString(ULONGLONG);
wstring HeapFlags_toString(DWORD);
wstring CorImageFlags_toString(DWORD);
wstring CorHeapSizes_toString(BYTE);
wstring CorMetadataTablesSummary_toString(ULONGLONG, ULONGLONG, DWORD *);
wstring CorGUIDs_toString(BYTE *, DWORD);
wstring CorVTableType_toString(USHORT);
wstring BaseRelocType_toString(WORD);
wstring ResourceID_toString(WORD);