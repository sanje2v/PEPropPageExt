#include "stdafx.h"
#include "PEReadWrite.h"
#include "MiscFuncs.h"
#include "CommonDefs.h"
#include <algorithm>


#define LDR_IS_DATAFILE(handle)      (((ULONG_PTR)(handle)) & (ULONG_PTR)1)
#define LDR_IS_IMAGEMAPPING(handle)  (((ULONG_PTR)(handle)) & (ULONG_PTR)2)
#define LDR_IS_RESOURCE(handle)      (LDR_IS_IMAGEMAPPING(handle) || LDR_IS_DATAFILE(handle))


PEReadWrite::PEReadWrite(tstring filename)
{
	// This constructor combines default constructor and the call to 'Open'
	PEReadWrite();
	if (!Open(filename))
		throw std::exception("Target file couldn't be loaded");
}

PEReadWrite::~PEReadWrite() { Close(); }

///////////////////////////////// File loading and closing /////////////////////////////////
bool PEReadWrite::Open(tstring filename)
{
	// Map executable into process address space
	bool Retval = false;	// Needed by 'GOTO_RELEASE_HANDLER' macros
	
	// Try to open with both read-write permission first
	DWORD Permission = GENERIC_READ | GENERIC_WRITE;

TryReadOnlyPermission:
	m_hFile = CreateFile((LPTSTR) filename.c_str(),
							Permission,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
	if (GetLastError() == ERROR_ACCESS_DENIED ||
		GetLastError() == ERROR_SHARING_VIOLATION)
	{
		switch (Permission)
		{
		case GENERIC_READ | GENERIC_WRITE:	// If first try failed,
			Permission = GENERIC_READ;		// try to open with read permission only
			goto TryReadOnlyPermission;

		case GENERIC_READ:
			GOTO_RELEASE_HANDLER(0, false);	// If both attempts failed, give up
		}
	}
	if (m_hFile == INVALID_HANDLE_VALUE)	// Also, in case of any other error give up
		GOTO_RELEASE_HANDLER(0, false);

	// Make a note on whether the file was opened with both Read-Write access
	// or only Read access
	m_bFileReadOnly = Permission != (GENERIC_READ | GENERIC_WRITE);

	// Create a file mapping into memory
	m_hFileMappingObject = CreateFileMapping(m_hFile,
												NULL,
												(m_bFileReadOnly ? PAGE_READONLY : PAGE_READWRITE),
												0, 0, NULL);
	if (!m_hFileMappingObject)
		GOTO_RELEASE_HANDLER(1, false);
	
	// Create a view of the whole file
	m_pMapViewBaseAddress = MapViewOfFile(m_hFileMappingObject,
											(m_bFileReadOnly ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE),
											0, 0, 0);
	if (!m_pMapViewBaseAddress)
		GOTO_RELEASE_HANDLER(1, false);

	// Save filename
	m_Filename = filename;

	// If file is a Portable Executable, save pointers
	// to allow speedier access later
	if (GetPrimaryHeaderType() == MZHeader &&
		GetSecondaryHeaderType() == PEHeader)
	{
		m_pNTHeader = GetSecondaryHeader<PIMAGE_NT_HEADERS>();
		m_PEType = GetPEType();
		switch (m_PEType)
		{
		case PE32:
			m_NoOfDataDirs = reinterpret_cast<PIMAGE_NT_HEADERS32>(m_pNTHeader)->OptionalHeader.NumberOfRvaAndSizes;
			m_pDataDirs = reinterpret_cast<PIMAGE_NT_HEADERS32>(m_pNTHeader)->OptionalHeader.DataDirectory;

			break;

		case PE64:
			m_NoOfDataDirs = reinterpret_cast<PIMAGE_NT_HEADERS64>(m_pNTHeader)->OptionalHeader.NumberOfRvaAndSizes;
			m_pDataDirs = reinterpret_cast<PIMAGE_NT_HEADERS64>(m_pNTHeader)->OptionalHeader.DataDirectory;
		}

		m_pSectionHeaders = GetFirstSectionHeader();
	}
	

	GOTO_RELEASE_HANDLER(0, true);	// Success

	DEFINE_RELEASE_HANDLER(1, Close(););
	DEFINE_RELEASE_HANDLER(0, );

	return Retval;
}

void PEReadWrite::LoadFileAsResourceModule()
{
	// Map process/dll into address space
	// This is required to read/write resource
	if (m_hFileAsResourceModule == NULL)
		m_hFileAsResourceModule = LoadLibraryEx((LPTSTR) m_Filename.c_str(),
												NULL,
												LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
}

void PEReadWrite::Close()
{
	// Release all held resources
	if (m_pMapViewBaseAddress)
		UnmapViewOfFile(m_pMapViewBaseAddress);

	if (m_hFileMappingObject)
		CloseHandle(m_hFileMappingObject);

	if (m_hFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFile);

	if (m_hFileAsResourceModule)
		FreeLibrary(m_hFileAsResourceModule);

	m_bFileOpened = false;
	m_Filename = _T("");
	m_hFile = INVALID_HANDLE_VALUE;
	m_hFileMappingObject = NULL;
	m_pMapViewBaseAddress = NULL;
	m_hFileAsResourceModule = NULL;
}


///////////////////////////////// Utility functions /////////////////////////////////
void *PEReadWrite::FileOffsetToMemAddress(DWORD offset)
{
	return (void *) (reinterpret_cast<UINT_PTR>(m_pMapViewBaseAddress) + offset);
}

void *PEReadWrite::GetVA(DWORD rva, bool isalwaysfileoffset)
{
	if (isalwaysfileoffset)
		return FileOffsetToMemAddress(rva);

	if (rva < m_pSectionHeaders[0].VirtualAddress)
		return FileOffsetToMemAddress(rva);
	else
		for (WORD i = 0; i < m_pNTHeader->FileHeader.NumberOfSections; i++)
		{
			if (rva >= m_pSectionHeaders[i].VirtualAddress && 
				rva < m_pSectionHeaders[i].VirtualAddress + m_pSectionHeaders[i].Misc.VirtualSize)
				return FileOffsetToMemAddress(rva -
												m_pSectionHeaders[i].VirtualAddress +
												m_pSectionHeaders[i].PointerToRawData);
		}

	throw std::exception();
}

LARGE_INTEGER PEReadWrite::GetFileSize()
{
	LARGE_INTEGER filesize;
	filesize.QuadPart = 0L;

	GetFileSizeEx(m_hFile, &filesize);

	return filesize;
}

DWORD PEReadWrite::RVAToFileOffset(DWORD rva)
{
	if (rva < m_pSectionHeaders[0].VirtualAddress)
		return rva;
	else
		for (WORD i = 0; i < m_pNTHeader->FileHeader.NumberOfSections; i++)
		{
			if (rva >= m_pSectionHeaders[i].VirtualAddress && 
				rva < m_pSectionHeaders[i].VirtualAddress + m_pSectionHeaders[i].Misc.VirtualSize)
				return rva - m_pSectionHeaders[i].VirtualAddress + m_pSectionHeaders[i].PointerToRawData;
		}

	return 0;
}

DWORD PEReadWrite::FileOffsetToRVA(DWORD fileoffset)
{
	for (int i = 0; i < m_pNTHeader->FileHeader.NumberOfSections; i++)
	{
		if (m_pSectionHeaders[i].PointerToRawData <= fileoffset)
			if ((m_pSectionHeaders[i].PointerToRawData + m_pSectionHeaders[i].SizeOfRawData) > fileoffset)
			{
				fileoffset -= m_pSectionHeaders[i].PointerToRawData;
				fileoffset += m_pSectionHeaders[i].VirtualAddress;

				return fileoffset;
			}
	}

	return 0;
}

bool PEReadWrite::IsMicrosoftStyleMangledName(string input)
{
	return input.at(0) == '?';
}

string PEReadWrite::UnmangleCPPNames(string input, SymbolPart part)
{
	CHAR szBuffer[1024];
	size_t BufferSize = GetArraySize(szBuffer);
	DWORD RetVal;

	// First try to unmangle using Microsoft's function
	if (IsMicrosoftStyleMangledName(input))
	{
		RetVal = UnDecorateSymbolName(input.c_str(), szBuffer, (DWORD) BufferSize, 
									part == Name ? UNDNAME_NAME_ONLY : UNDNAME_COMPLETE);
		
		if (RetVal != 0)
			return szBuffer;
	}

	// If that failed, try to unmangle using GCC's function
	if (__cxa_demangle(input.c_str(), szBuffer, &BufferSize, NULL))
		return szBuffer;

	// All failed, then return the original name
	return input.c_str();
}


///////////////////////////////// Header functions /////////////////////////////////

// 1. Primary Header functions
PEReadWrite::HeaderType PEReadWrite::GetPrimaryHeaderType()
{
	switch (*GetPrimaryHeader<LPWORD>())
	{
		case IMAGE_DOS_SIGNATURE: return MZHeader;
	}

	return UnknownHeader;
}

template <typename T>
T PEReadWrite::GetPrimaryHeader()
{
	return static_cast<T>(m_pMapViewBaseAddress);
}

// 2. Secondary Header functions
PEReadWrite::HeaderType PEReadWrite::GetSecondaryHeaderType()
{
	assert(GetPrimaryHeaderType() == MZHeader);

	PIMAGE_DOS_HEADER pDOSHeader = GetPrimaryHeader<PIMAGE_DOS_HEADER>();

	switch (*((LPDWORD) FileOffsetToMemAddress(pDOSHeader->e_lfanew)))
	{
		case IMAGE_OS2_SIGNATURE: return NEHeader;
		case IMAGE_VXD_SIGNATURE: return LEHeader;
		case IMAGE_NT_SIGNATURE: return PEHeader;
	}

	return UnknownHeader;
}

template <typename T>
T PEReadWrite::GetSecondaryHeader()
{
	assert(GetPrimaryHeaderType() == MZHeader);

	PIMAGE_DOS_HEADER pDOSHeader = GetPrimaryHeader<PIMAGE_DOS_HEADER>();

	return (T) FileOffsetToMemAddress(pDOSHeader->e_lfanew);
}

// 3. Information from secondary header
PEReadWrite::PEType PEReadWrite::GetPEType()
{
	assert(GetSecondaryHeaderType() == PEHeader);

	switch (m_pNTHeader->OptionalHeader.Magic)
	{
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC: return PE32;
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC: return PE64;
		case IMAGE_ROM_OPTIONAL_HDR_MAGIC: return ROM;
	}

	return UnknownPEType;
}

DWORD PEReadWrite::GetNoOfDataDirectories()
{
	return m_NoOfDataDirs;
}

PIMAGE_DATA_DIRECTORY PEReadWrite::GetDataDirectory(int index)
{
	return &m_pDataDirs[index];
}


///////////////////////////////// Checksum function /////////////////////////////////
bool PEReadWrite::VerifyPEChecksum(DWORD *pcalculatedsum = NULL)
{
	DWORD ChecksumInHeader, Calculatedchecksum;

	if (CheckSumMappedFile(m_pMapViewBaseAddress,
							GetFileSize().LowPart,
							&ChecksumInHeader,
							&Calculatedchecksum))
	{
		if (pcalculatedsum)
			*pcalculatedsum = Calculatedchecksum;

		return ChecksumInHeader == Calculatedchecksum;	// Return true if checksum
															// in PE header and calculated
															// checksum matches
	}

	return false;
}


///////////////////////////////// Section functions /////////////////////////////////
PIMAGE_SECTION_HEADER PEReadWrite::GetFirstSectionHeader()
{
	return IMAGE_FIRST_SECTION(m_pNTHeader);
}

unsigned short PEReadWrite::GetNoOfSections()
{
	return m_pNTHeader->FileHeader.NumberOfSections;
}

PIMAGE_SECTION_HEADER PEReadWrite::GetSectionHeader(int index)
{
	return &GetFirstSectionHeader()[index];
}


//////////////////////////// Generic resource functions //////////////////////////////

bool PEReadWrite::HasResources()
{
	if (m_NoOfDataDirs >= IMAGE_DIRECTORY_ENTRY_RESOURCE)
		return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size != 0;

	return false;
}


////////////////////////////////// Manifest functions //////////////////////////////////
BOOL CALLBACK PEReadWrite::CallbackFindEmbeddedManifest(HMODULE hModule,
														LPTSTR lpType,
														LONG_PTR lParam)
{
	// See if there are any manifest in the resource section
	if (IS_INTRESOURCE(lpType) && lpType == MAKEINTRESOURCE(RT_MANIFEST))
	{
		*reinterpret_cast<bool *>(lParam) = true;
		
		return FALSE;	// If one is found, no need to look further
	}

	return TRUE;
}

BOOL CALLBACK PEReadWrite::CallbackFindEmbeddedManifestRawNames(HMODULE hModule,
																LPCTSTR lpszType,
																LPTSTR lpszName,
																LONG_PTR lParam)
{
	// Lookup manifest raw names
	// NOTE: They are called raw names because they might either be a pointer
	// to a C-string or just a normal number
	vector<LPTSTR> *plstNames = reinterpret_cast<vector<LPTSTR> *>(lParam);

	plstNames->push_back(lpszName);

	return TRUE;
}

BOOL CALLBACK PEReadWrite::CallbackEnumEmbeddedManifestLangs(HMODULE hModule,
																LPCTSTR lpszType,
																LPCTSTR lpszName,
																WORD wIDLanguage,
																LONG_PTR lParam)
{
	// Lookup the languages the manifest is available for a given 'lpszName'
	vector<WORD> *plstLangs = reinterpret_cast<vector<WORD> *>(lParam);

	plstLangs->push_back(wIDLanguage);

	return TRUE;
}

bool PEReadWrite::HasManifest()
{
	return GetManifestSource() != ManifestNotPresent;
}


tstring PEReadWrite::MakeManifestFilename()
{
	// External manifest files are required to be named
	// '<EXE or DLL name>.<Extension>.manifest'
	return m_Filename + _T(".manifest");
}

PEReadWrite::ManifestSource PEReadWrite::GetManifestSource()
{
	// Figure out if resource section contains a manifest,
	// there is an external manifest file or there are no manifests
	// defined for this file
	bool bFound;

	LoadFileAsResourceModule();

	EnumResourceTypes(m_hFileAsResourceModule,
						CallbackFindEmbeddedManifest,
						(LONG_PTR) &bFound);
	if (bFound)
		return ManifestInternal;

	if (PathFileExists((LPTSTR) MakeManifestFilename().c_str()))
		return ManifestExternal;

	return ManifestNotPresent;
}

vector<LPTSTR> PEReadWrite::GetManifestRawNames()
{
	// Use 'CallbackFindEmbeddedManifestRawNames' to enumerate raw names
	vector<LPTSTR> lstRawNames;

	LoadFileAsResourceModule();

	EnumResourceNames(m_hFileAsResourceModule, MAKEINTRESOURCE(RT_MANIFEST),
						CallbackFindEmbeddedManifestRawNames, (LONG_PTR) &lstRawNames);

	return lstRawNames;
}

vector<tstring> PEReadWrite::GetManifestNames()
{
	// Use the enumerated raw names to generate proper names
	vector<LPTSTR> lstRawNames = GetManifestRawNames();
	vector<tstring> lstProperNames;

	for (int i = 0; i < lstRawNames.size(); i++)
		if (IS_INTRESOURCE(lstRawNames[i]))
			lstProperNames.push_back((LPTSTR) DWORD_toString((UINT) lstRawNames[i], Hexadecimal).c_str());
		else
			lstProperNames.push_back(lstRawNames[i]);

	return lstProperNames;
}

vector<WORD> PEReadWrite::GetManifestLangsFromRawName(LPCTSTR lpszName)
{
	// Use the raw name to look up the available languages
	vector<WORD> lstLangs;

	LoadFileAsResourceModule();

	EnumResourceLanguages(m_hFileAsResourceModule,
							MAKEINTRESOURCE(RT_MANIFEST),
							lpszName,
							CallbackEnumEmbeddedManifestLangs,
							(LONG_PTR) &lstLangs);

	return lstLangs;
}

tstring PEReadWrite::GetManifest(LPTSTR lpszName, WORD lcid)
{
	tstring buffer;
	LoadFileAsResourceModule();

	switch (GetManifestSource())
	{
	case ManifestInternal:
		{
			// Open internal manifest
			HRSRC hResInfo = FindResourceEx(m_hFileAsResourceModule,
											MAKEINTRESOURCE(RT_MANIFEST),
											lpszName, lcid);

			if (hResInfo)
			{
				HGLOBAL hResData = LoadResource(m_hFileAsResourceModule, hResInfo);
				

				if(hResData)
				{
					LPTSTR pManifestXML = (LPTSTR) LockResource(hResData);
					buffer = MultiByte_toString((CHAR *) pManifestXML, false,
														SizeofResource(m_hFileAsResourceModule, hResInfo));

					FreeResource(hResData);
					
					return buffer;
				}
			}
		}

		break;

	case ManifestExternal:
		{
			// Open external manifest file
			HANDLE hManifestFile = CreateFile((LPTSTR) MakeManifestFilename().c_str(),
												GENERIC_READ, FILE_SHARE_READ, NULL,
												OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			DWORD cbFileSize;
			if (hManifestFile == INVALID_HANDLE_VALUE)			// File couldn't be opened
			{
				LogError(_T("External manifest file couldn't be opened!"));
				break;
			}

			cbFileSize = ::GetFileSize(hManifestFile, NULL);
			if (cbFileSize == 0)
			{
				CloseHandle(hManifestFile);
				break;
			}

			unique_ptr<PCHAR> lpBuffer(new PCHAR[cbFileSize]);
			DWORD cbBytesRead;
			ReadFile(hManifestFile, lpBuffer.get(), cbFileSize, &cbBytesRead, NULL);
			CloseHandle(hManifestFile);

			if (cbBytesRead == 0)
				break;

			buffer = MultiByte_toString((CHAR *) lpBuffer.get(), false, cbBytesRead);

			return buffer;
		}

		break;
	}

	return _T("");
}


///////////////////////////////// Exception handling functions /////////////////////////////////
bool PEReadWrite::HasExceptionHandlingData()
{
	if (m_NoOfDataDirs >= IMAGE_DIRECTORY_ENTRY_EXCEPTION + 1)
		return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size != 0;

	return false;
}

template <typename T>
T PEReadWrite::GetFirstExceptionHandler()
{
	assert(GetSecondaryHeaderType() == PEHeader);

	return (T) GetVA(m_pDataDirs[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress);
}

template <typename T>
T PEReadWrite::GetExceptionHandler(int index)
{
	assert(GetSecondaryHeaderType() == PEHeader);
	assert(HasExceptionHandlingData());

	return &(GetFirstExceptionHandler<T>()[index]);
}

int PEReadWrite::GetExceptionHandlingDataSize()
{
	assert(HasExceptionHandlingData());

	PIMAGE_LOAD_CONFIG_DIRECTORY pLoadConfig = (PIMAGE_LOAD_CONFIG_DIRECTORY) GetVA(
							m_pDataDirs[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress);

	return pLoadConfig->Size;
}

int PEReadWrite::GetNoOfExceptionHandlers()
{
	switch (m_pNTHeader->FileHeader.Machine)
	{
	case IMAGE_FILE_MACHINE_ALPHA:
		return GetExceptionHandlingDataSize() / sizeof(IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY);

	case IMAGE_FILE_MACHINE_ALPHA64:
		return GetExceptionHandlingDataSize() / sizeof(IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY);

	case IMAGE_FILE_MACHINE_ARM:
	case IMAGE_FILE_MACHINE_POWERPC:
	case IMAGE_FILE_MACHINE_POWERPCFP:
	case IMAGE_FILE_MACHINE_SH3:
	case IMAGE_FILE_MACHINE_SH3DSP:
	case IMAGE_FILE_MACHINE_SH3E:
	case IMAGE_FILE_MACHINE_SH4:
		return GetExceptionHandlingDataSize() / sizeof(IMAGE_CE_RUNTIME_FUNCTION_ENTRY);

	case IMAGE_FILE_MACHINE_AMD64:
	case IMAGE_FILE_MACHINE_IA64:
		return GetExceptionHandlingDataSize() / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);
	}

	return 0;
}


///////////////////////////////// Base Relocation functions /////////////////////////////////
bool PEReadWrite::HasBaseRelocationData()
{
	if (m_NoOfDataDirs >= IMAGE_DIRECTORY_ENTRY_BASERELOC + 1)
		return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size != 0;

	return false;
}

PIMAGE_BASE_RELOCATION PEReadWrite::GetFirstBaseRelocationTable()
{
	return (PIMAGE_BASE_RELOCATION) GetVA(m_pDataDirs[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
}

int PEReadWrite::GetNoOfBaseRelocationTables()
{
	PIMAGE_BASE_RELOCATION pBaseRelocTable = GetFirstBaseRelocationTable();
	int Count = 0;

	while (pBaseRelocTable->SizeOfBlock != 0)
	{
		Count++;
		pBaseRelocTable = (PIMAGE_BASE_RELOCATION) ((UINT_PTR) pBaseRelocTable + pBaseRelocTable->SizeOfBlock);
	}

	return Count;
}

PIMAGE_BASE_RELOCATION PEReadWrite::GetBaseRelocationTable(int index)
{
	PIMAGE_BASE_RELOCATION pBaseRelocTable = GetFirstBaseRelocationTable();
	int Count = 0;

	while (Count != index)
	{
		Count++;
		pBaseRelocTable = (PIMAGE_BASE_RELOCATION) ((UINT_PTR) pBaseRelocTable + pBaseRelocTable->SizeOfBlock);
	}

	return pBaseRelocTable;
}

int PEReadWrite::GetNoOfBaseRelocationEntries(int tableindex)
{
	PIMAGE_BASE_RELOCATION pBaseRelocTable = GetBaseRelocationTable(tableindex);

	return (pBaseRelocTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(IMAGE_FIXUP_ENTRY);
}

PIMAGE_FIXUP_ENTRY PEReadWrite::GetBaseRelocationEntry(int tableindex, int rowindex)
{
	PIMAGE_BASE_RELOCATION pBaseRelocTable = GetBaseRelocationTable(tableindex);
	PIMAGE_FIXUP_ENTRY pBaseRelocEntry = (PIMAGE_FIXUP_ENTRY) ((UINT_PTR) pBaseRelocTable + sizeof(IMAGE_BASE_RELOCATION));
	
	return &pBaseRelocEntry[rowindex];
}


//////////////////////////////////// Debug Info functions ///////////////////////////////////
bool PEReadWrite::HasDebugData()
{
	if (m_NoOfDataDirs >= IMAGE_DIRECTORY_ENTRY_DEBUG + 1)
		return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_DEBUG].Size != 0;

	return false;
}

PIMAGE_DEBUG_DIRECTORY PEReadWrite::GetFirstDebugDirectory()
{
	assert(GetSecondaryHeaderType() == PEHeader);

	return (PIMAGE_DEBUG_DIRECTORY) GetVA(m_pDataDirs[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress);
}

PIMAGE_DEBUG_DIRECTORY PEReadWrite::GetDebugDirectory(int index)
{
	return &GetFirstDebugDirectory()[index];
}

int PEReadWrite::GetNoOfDebugDirs()
{
	return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_DEBUG].Size / sizeof(IMAGE_DEBUG_DIRECTORY);
}


/////////////////////////////////// Global Ptr functions ////////////////////////////////////
DWORD PEReadWrite::GetGlobalPtrData()
{
	return *static_cast<LPDWORD>(GetVA(m_pDataDirs[IMAGE_DIRECTORY_ENTRY_GLOBALPTR].VirtualAddress));
}


//////////////////////////////// Load Configuration functions ////////////////////////////////
bool PEReadWrite::HasLoadConfigData()
{
	if (m_NoOfDataDirs >= IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG + 1)
		return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size != 0;

	return false;
}

template<typename T> T PEReadWrite::GetLoadConfigurationTable()
{
	return (T) GetVA(m_pDataDirs[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress);
}

int PEReadWrite::GetLoadConfigurationTableSize()
{
	return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size;
}


///////////////////////////////////// TLS functions //////////////////////////////////////////
bool PEReadWrite::HasTLSData()
{
	if (m_NoOfDataDirs >= IMAGE_DIRECTORY_ENTRY_TLS + 1)
		return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_TLS].Size != 0;

	return false;
}

void *PEReadWrite::GetTLSDirectory()
{
	return GetVA(m_pDataDirs[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
}

int PEReadWrite::GetTLSTableSize()
{
	return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_TLS].Size;
}


///////////////////////////////////// CLR functions //////////////////////////////////////////
bool PEReadWrite::HasCLRData()
{
	if (m_NoOfDataDirs >= IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR + 1)
		return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size != 0;

	return false;
}

PIMAGE_COR20_HEADER PEReadWrite::GetCLRHeader()
{
	return (PIMAGE_COR20_HEADER) GetVA(m_pDataDirs[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress);
}

int PEReadWrite::GetCLRHeaderSize()
{
	return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size;
}


///////////////////////////////////// Export functions /////////////////////////////////////////
bool PEReadWrite::HasExports()
{
	if (m_NoOfDataDirs >= IMAGE_DIRECTORY_ENTRY_EXPORT + 1)
		return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_EXPORT].Size != 0;

	return false;
}

PIMAGE_EXPORT_DIRECTORY PEReadWrite::GetExportDirectory()
{
	assert(GetSecondaryHeaderType() == PEHeader);

	return (PIMAGE_EXPORT_DIRECTORY) GetVA(m_pDataDirs[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
}

LPWORD PEReadWrite::GetExportOrdinalsTable()
{
	return (LPWORD) GetVA(GetExportDirectory()->AddressOfNameOrdinals);
}

WORD PEReadWrite::GetExportOrdinal(LPWORD pordinalstable, int index)
{
	return pordinalstable[index];
}

LPDWORD PEReadWrite::GetExportNamesTable()
{
	return (LPDWORD) GetVA(GetExportDirectory()->AddressOfNames);
}

string PEReadWrite::GetExportName(LPDWORD pnamestable, int index)
{
	return (char *) GetVA(pnamestable[index]);
}

vector<pair<WORD, string> > PEReadWrite::GetExportOrdinalsAndNames()
{
	vector<pair<WORD, string> > lstOrdinalsAndNames;
	PIMAGE_EXPORT_DIRECTORY pExportDir = GetExportDirectory();
	LPWORD pOrdinalsTable = GetExportOrdinalsTable();
	LPDWORD pNamesTable = GetExportNamesTable();

	for (DWORD i = 0; i < pExportDir->NumberOfNames; i++)
		lstOrdinalsAndNames.push_back(pair<WORD, string>(
						GetExportOrdinal(pOrdinalsTable, i), GetExportName(pNamesTable, i)));

	return lstOrdinalsAndNames;
}

vector<DWORD> PEReadWrite::GetExportRVAs()
{
	vector<DWORD> listRVAs;
	PIMAGE_EXPORT_DIRECTORY pExportDir = GetExportDirectory();

	for(DWORD i = 0; i < pExportDir->NumberOfFunctions; i++)
		listRVAs.push_back(((LPDWORD) GetVA(pExportDir->AddressOfFunctions))[i]);

	return listRVAs;
}

bool PEReadWrite::IsExportForwarderName(DWORD rva)
{
	return rva >= m_pDataDirs[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress &&
					rva < m_pDataDirs[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress +
						m_pDataDirs[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
}

tstring PEReadWrite::GetExportForwarderName(DWORD rva)
{
	return MultiByte_toString((char *) GetVA(rva));
}


///////////////////////////////////// Import functions /////////////////////////////////////////
bool PEReadWrite::HasStaticImports()
{
	if (m_NoOfDataDirs >= IMAGE_DIRECTORY_ENTRY_IMPORT + 1)
		return (m_pDataDirs[IMAGE_DIRECTORY_ENTRY_IMPORT].Size != 0);

	return false;
}

bool PEReadWrite::HasDelayedImports()
{
	if (m_NoOfDataDirs >= IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT + 1)
		return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].Size != 0;

	return false;
}

bool PEReadWrite::HasImports()
{
	// Tests if either normal Imports or Delayed Imports are available
	return HasStaticImports() || HasDelayedImports();
}

bool PEReadWrite::IsImportsAlreadyBound()
{
	if (m_NoOfDataDirs >= IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT + 1)
		return m_pDataDirs[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size != 0;

	return false;
}

PIMAGE_IMPORT_DESCRIPTOR PEReadWrite::GetFirstImportDirectory()
{
	assert(GetSecondaryHeaderType() == PEHeader);

	return (PIMAGE_IMPORT_DESCRIPTOR) GetVA(m_pDataDirs[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
}

PImgDelayDescr PEReadWrite::GetFirstDelayImportDirectory()
{
	return (PImgDelayDescr) GetVA(m_pDataDirs[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress);
}

PIMAGE_IMPORT_DESCRIPTOR PEReadWrite::GetImportDirectory(int index)
{
	return &GetFirstImportDirectory()[index];
}

PImgDelayDescr PEReadWrite::GetDelayImportDirectory(int index)
{
	return &GetFirstDelayImportDirectory()[index];
}

vector<PEReadWrite::ImportNameType> PEReadWrite::GetImportsModules(LPDWORD noofstaticimportmodules,
																	LPDWORD noofdelayedimportmodules)
{
	vector<ImportNameType> lstModules;
	vector<ImportNameType> lstStaticMods = GetImportsModules(Static);
	vector<ImportNameType> lstDelayedMods = GetImportsModules(Delayed);

	lstModules = lstStaticMods;
	lstModules.insert(lstModules.end(), lstDelayedMods.begin(), lstDelayedMods.end());

	if (noofstaticimportmodules)
		*noofstaticimportmodules = (DWORD) lstStaticMods.size();

	if (noofdelayedimportmodules)
		*noofdelayedimportmodules = (DWORD) lstDelayedMods.size();

	return lstModules;
}

vector<PEReadWrite::ImportNameType> PEReadWrite::GetImportsModules(ImportType importtype)
{
	vector<ImportNameType> lstModules;

	switch(importtype)
	{
	case Static:
		{
			if (!HasStaticImports())
				break;

			PIMAGE_IMPORT_DESCRIPTOR pImportDescriptors = GetFirstImportDirectory();

			if(pImportDescriptors)
				while(pImportDescriptors->Name != 0)
				{
					lstModules.push_back(ImportNameType(MultiByte_toString(
												(char *) GetVA(pImportDescriptors->Name)), Static));

					pImportDescriptors++;
				}
		}

		break;

	case Delayed:
		{
			if (!HasDelayedImports())
				break;

			PImgDelayDescr pDelayLoadDirectory = GetFirstDelayImportDirectory();

			if(pDelayLoadDirectory)
				while(pDelayLoadDirectory->rvaDLLName != 0)
				{
					lstModules.push_back(ImportNameType(MultiByte_toString(
										(char *) GetVA(pDelayLoadDirectory->rvaDLLName)), Delayed));

					pDelayLoadDirectory++;
				}
		}
	}

	return lstModules;
}

vector<PEReadWrite::ImportFunction> PEReadWrite::GetImportsFunctions(DWORD index, ImportType importtype)
{
	vector<ImportFunction> lstFunctions;

	if ((importtype == Static && !HasStaticImports()) ||
		(importtype == Delayed && !HasDelayedImports()))
		return lstFunctions;

	switch (m_PEType)
	{
	case PE32:
		{
			PIMAGE_THUNK_DATA32 pImportLookupTable32;

			switch (importtype)
			{
			case Static:
					pImportLookupTable32 = (PIMAGE_THUNK_DATA32) GetVA(IsImportsAlreadyBound() ?
																	GetImportDirectory(index)->OriginalFirstThunk :
																	GetImportDirectory(index)->FirstThunk);

				break;

			case Delayed:
				pImportLookupTable32 = (PIMAGE_THUNK_DATA32) GetVA(GetDelayImportDirectory(index)->rvaINT);
			}

			while (pImportLookupTable32->u1.Ordinal)
			{
				if (IMAGE_SNAP_BY_ORDINAL32(pImportLookupTable32->u1.Ordinal))
					lstFunctions.push_back(ImportFunction(ByOrdinal, 0, "", IMAGE_ORDINAL32(pImportLookupTable32->u1.Ordinal)));
				else
					lstFunctions.push_back(ImportFunction(ByName,
											((PIMAGE_IMPORT_BY_NAME) GetVA(pImportLookupTable32->u1.Function))->Hint,
											(char *) ((PIMAGE_IMPORT_BY_NAME) GetVA(pImportLookupTable32->u1.Function))->Name));

				pImportLookupTable32++;
			}
		}

		break;

	case PE64:
		{
			PIMAGE_THUNK_DATA64 pImportLookupTable64;

			switch (importtype)
			{
			case Static:
				pImportLookupTable64 = (PIMAGE_THUNK_DATA64) GetVA(IsImportsAlreadyBound() ?
																GetImportDirectory(index)->OriginalFirstThunk :
																GetImportDirectory(index)->FirstThunk);

				break;

			case Delayed:
				pImportLookupTable64 = (PIMAGE_THUNK_DATA64) GetVA(GetDelayImportDirectory(index)->rvaINT);
			}

			while (pImportLookupTable64->u1.Ordinal)
			{
				if (IMAGE_SNAP_BY_ORDINAL64(pImportLookupTable64->u1.Ordinal))
					lstFunctions.push_back(ImportFunction(ByOrdinal, 0, "", IMAGE_ORDINAL64(pImportLookupTable64->u1.Ordinal)));
				else
					lstFunctions.push_back(ImportFunction(ByName,
												((PIMAGE_IMPORT_BY_NAME) GetVA((DWORD) pImportLookupTable64->u1.Function))->Hint,
										(char *) ((PIMAGE_IMPORT_BY_NAME) GetVA((DWORD) pImportLookupTable64->u1.Function))->Name));

				pImportLookupTable64++;
			}
		}
	}

	return lstFunctions;
}