#pragma once

#include "stdafx.h"
#include "CommonDefs.h"
#include <Windows.h>
#include <string>


using namespace std;


enum FORMATTYPE {Decimal, Hexadecimal};

void LogError(tstring info, bool bCritical = false);
void tstring_ReplaceAll(tstring& source, TCHAR strtofind, TCHAR strtoreplace);
tstring Integer_toString(int value);
tstring DWORD_toString(DWORD, FORMATTYPE = Decimal);
tstring QWORD_toString(ULONGLONG, FORMATTYPE = Decimal);
tstring Signature_toString(DWORD);
tstring float_toString(float);
LPTSTR LEOrdering_toString(BYTE);
LPTSTR LECPUtype_toString(WORD);
LPTSTR LEOStype_toString(WORD);
tstring LEModuletypeflags_toString(DWORD);
tstring VersionNums_toString(WORD, WORD);
tstring MagicNum_toString(DWORD);
tstring OSIdString(WORD, WORD);
tstring FormattedBytesSize(ULONGLONG);
tstring MachineType_toString(WORD);
tstring ImageCharacteristics_toString(DWORD);
tstring SubsystemID_toString(WORD);
tstring DllCharacteristics_toString(WORD);
tstring ProperSectionName(BYTE [IMAGE_SIZEOF_SHORT_NAME]);
tstring SectionCharacteristics_toString(DWORD);
tstring StandardSectionNameAnnotation(tstring);
tstring ExceptionArch_toString(DWORD);
tstring ExceptionFlag_toString(DWORD);
tstring LCIDtoLangName(WORD);
tstring MultiByte_toString(const char *, bool = true, int = -1);
tstring MultiByte_toString(string &);
tstring TimeDateStamp_toString(DWORD);
tstring GUID_toString(GUID&);
tstring DebugType_toString(DWORD);
tstring DebugMiscDataType_toString(DWORD);
tstring ProcessorAffinityMask_toString(ULONGLONG);
tstring HeapFlags_toString(DWORD);
tstring CorImageFlags_toString(DWORD);
tstring CorHeapSizes_toString(BYTE);
tstring CorMetadataTablesSummary_toString(ULONGLONG, ULONGLONG, DWORD *);
tstring CorGUIDs_toString(BYTE *, DWORD);
tstring CorVTableFlags_toString(USHORT);
tstring BaseRelocType_toString(WORD);