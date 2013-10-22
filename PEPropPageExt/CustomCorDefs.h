#pragma once

#include <windows.h>
#include <cor.h>
#include <CorHdr.h>

#define CLR_META_DATA_SIGNATURE		0x424A5342UL

typedef IMAGE_COR_VTABLEFIXUP* PIMAGE_COR_VTABLEFIXUP;

typedef struct _META_DATA_SECTION_HEADER1
{
	DWORD Signature;
	WORD MajorVersion;
	WORD MinorVersion;
	DWORD Reserved;
	DWORD Length;
	char Name;
} META_DATA_SECTION_HEADER1, *PMETA_DATA_SECTION_HEADER1;

typedef struct _META_DATA_SECTION_HEADER2
{
	WORD Flags;
	WORD NoOfStreams;
} META_DATA_SECTION_HEADER2, *PMETA_DATA_SECTION_HEADER2;

typedef struct _META_STREAM_HEADER
{
	DWORD Offset;
	DWORD Size;
	char Name[1];
} META_STREAM_HEADER, *PMETA_STREAM_HEADER;

typedef struct _META_COMPOSITE_HEADER
{
	DWORD Reserved;
	BYTE MajorVersion;
	BYTE MinorVersion;
	BYTE HeapSizes;
	BYTE Padding;
	ULONGLONG Valid;
	ULONGLONG Sorted;
	DWORD Rows[1];
} META_COMPOSITE_HEADER, *PMETA_COMPOSITE_HEADER;