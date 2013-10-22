#pragma once

#include "stdafx.h"
#include "CommonDefs.h"
#include "cxxabi.h"
#include <ImageHlp.h>
#include <delayimp.h>
#include <Windows.h>
#include <cor.h>
#include <CorHdr.h>
#include <string>
#include <vector>

#define WITHOUT_SE_HANDLER32_SIZE		64
#define WITHOUT_SE_HANDLER64_SIZE		96


using namespace std;

typedef struct {
WORD offset:12;
WORD type:4;
} IMAGE_FIXUP_ENTRY, *PIMAGE_FIXUP_ENTRY;

class PEReadWrite
{
public:
	// Public types
	//enum OpenType { ReadOnly = GENERIC_READ, WriteOnly = GENERIC_WRITE };
	enum HeaderType { UnknownHeader, MZHeader, NEHeader, LEHeader, PEHeader };
	enum ChecksumCheck { ChecksumError, ChecksumNotPresent, Correct, Incorrect };
	enum PEType { UnknownPEType, PE32, PE64, ROM };
	enum ManifestSource { ManifestNotPresent, ManifestInternal, ManifestExternal };
	enum SymbolPart { Name, Full };
	enum ImportType { Static, Delayed };
	enum FunctionImportType { ByOrdinal, ByName };

	struct ImportNameType
	{
		tstring Name;
		ImportType Type;

		ImportNameType(tstring& name, ImportType type)
			: Name(name), Type(type) { }
	};

	struct ImportFunction
	{
		FunctionImportType Type;
		WORD Hint;
		string Name;
		WORD Ordinal;

		ImportFunction(FunctionImportType type,
						WORD hint = 0,
						string name = "",
						WORD ordinal = 0)
						: Type(type), Hint(hint), Name(name), Ordinal(ordinal)
		{ }
	};

private:
	// Private variables
	tstring m_Filename;					// Filename (with path) of target executable
	HANDLE m_hFile;						// File handle to executable
	bool m_bFileOpened;					// File opened successfully and object ready?
	bool m_bFileReadOnly;				// Is the file read only?
	HANDLE m_hFileMappingObject;		// Handle to file mapping view object
	LPVOID m_pMapViewBaseAddress;		// Virtual address where the executable is mapped
	HMODULE m_hFileAsResourceModule;	// File loaded as a module for resource reading/writing

	PEType Type;						// Type of PE (See PEType enumeration above)

	// Specific PE locations and data
	PIMAGE_NT_HEADERS m_pNTHeader;
	PEType m_PEType;
	DWORD m_NoOfDataDirs;
	PIMAGE_DATA_DIRECTORY m_pDataDirs;
	PIMAGE_SECTION_HEADER m_pSectionHeaders;

	// Private functions
	PIMAGE_SECTION_HEADER GetFirstSectionHeader();
	bool HasStaticImports();
	bool HasDelayedImports();
	PIMAGE_IMPORT_DESCRIPTOR GetFirstImportDirectory();
	PImgDelayDescr GetFirstDelayImportDirectory();
	LPWORD GetExportOrdinalsTable();
	WORD GetExportOrdinal(LPWORD pordinalstable, int index);
	LPDWORD GetExportNamesTable();
	string GetExportName(LPDWORD pnamestable, int index);
	template<typename T> T GetFirstExceptionHandler();
	PIMAGE_BASE_RELOCATION GetFirstBaseRelocationTable();
	PIMAGE_DEBUG_DIRECTORY GetFirstDebugDirectory();

	void LoadFileAsResourceModule();	// Use 'LoadLibraryEx' to load for resource read/write

	static BOOL CALLBACK CallbackFindEmbeddedManifest(HMODULE hModule, LPTSTR lpType, LONG_PTR lParam);
	static BOOL CALLBACK CallbackFindEmbeddedManifestRawNames(HMODULE hModule, LPCTSTR lpszType,
																LPTSTR lpszName, LONG_PTR lParam);
	static BOOL CALLBACK CallbackEnumEmbeddedManifestLangs(HMODULE hModule, LPCTSTR lpszType,
															LPCTSTR lpszName, WORD wIDLanguage,
															LONG_PTR lParam);

public:
	explicit PEReadWrite() : m_bFileOpened(false),
								m_Filename(_T("")),
								m_hFile(INVALID_HANDLE_VALUE),
								m_hFileMappingObject(NULL),
								m_pMapViewBaseAddress(NULL),
								m_hFileAsResourceModule(NULL){ }
	PEReadWrite(tstring filename);
	virtual ~PEReadWrite();

	bool Open(wstring filename);	// Opens and maps file
	void Close();					// Unmaps and closes file

	void *FileOffsetToMemAddress(DWORD offset);
	void *GetVA(DWORD rva, bool isalwaysfileoffset = false);		// Get Virtual Address from RVA
	LARGE_INTEGER GetFileSize();
	DWORD RVAToFileOffset(DWORD rva);
	DWORD FileOffsetToRVA(DWORD fileoffset);

	// Header functions
	HeaderType GetPrimaryHeaderType();
	template<typename T> T GetPrimaryHeader();
	HeaderType GetSecondaryHeaderType();
	template<typename T> T GetSecondaryHeader();

	DWORD GetNoOfDataDirectories();
	PIMAGE_DATA_DIRECTORY GetDataDirectory(int index);

	// PE type
	PEType GetPEType();
	
	// Checksum function
	bool VerifyPEChecksum(DWORD *pcalculatedsum);

	// Section functions
	WORD GetNoOfSections();						// No. of section headers
	PIMAGE_SECTION_HEADER GetSectionHeader(int index);

	// Generic resource function
	bool HasResources();

	// Manifest functions
	bool HasManifest();
	tstring MakeManifestFilename();
	ManifestSource GetManifestSource();
	vector<LPTSTR> GetManifestRawNames();
	vector<tstring> GetManifestNames();
	vector<WORD> GetManifestLangsFromRawName(LPCTSTR);
	tstring GetManifest(LPTSTR = NULL, WORD = 0);

	// Exception Handling functions
	bool HasExceptionHandlingData();
	template<typename T> T GetExceptionHandler(int index);
	int GetExceptionHandlingDataSize();
	int GetNoOfExceptionHandlers();

	// Base Relocation functions
	bool HasBaseRelocationData();
	int GetNoOfBaseRelocationTables();
	PIMAGE_BASE_RELOCATION GetBaseRelocationTable(int index);
	int GetNoOfBaseRelocationEntries(int tableindex);
	PIMAGE_FIXUP_ENTRY GetBaseRelocationEntry(int tableindex, int rowindex);

	// Debug Info functions
	bool HasDebugData();
	PIMAGE_DEBUG_DIRECTORY GetDebugDirectory(int index);
	int GetNoOfDebugDirs();

	// Global Ptr functions
	DWORD GetGlobalPtrData();

	// Load Configuration functions
	bool HasLoadConfigData();
	template<typename T> T GetLoadConfigurationTable();
	int GetLoadConfigurationTableSize();

	// TLS functions
	bool HasTLSData();
	void *GetTLSDirectory();
	int GetTLSTableSize();

	// CLR functions
	bool HasCLRData();
	PIMAGE_COR20_HEADER GetCLRHeader();
	int GetCLRHeaderSize();

	// Export functions
	bool HasExports();
	PIMAGE_EXPORT_DIRECTORY GetExportDirectory();
	vector<pair<WORD, string> > GetExportOrdinalsAndNames();
	vector<DWORD> GetExportRVAs();
	bool IsExportForwarderName(DWORD rva);
	tstring GetExportForwarderName(DWORD rva);

	// Import functions (Both normal imports and delayed imports)
	bool HasImports();
	bool IsImportsAlreadyBound();
	PIMAGE_IMPORT_DESCRIPTOR GetImportDirectory(int index);
	PImgDelayDescr GetDelayImportDirectory(int index);
	vector<ImportNameType> GetImportsModules(LPDWORD noofstaticimportmodules = NULL,
												LPDWORD noofdelayedimportmodules = NULL);
	vector<ImportNameType> GetImportsModules(ImportType);
	vector<ImportFunction> GetImportsFunctions(DWORD index, ImportType importtype);
	
	// Functions common to both Import/Export
	static bool IsMicrosoftStyleMangledName(string);
	static string UnmangleCPPNames(string input, SymbolPart part);
};

// Explicit template initializations for extern member functions
template PIMAGE_NT_HEADERS32 PEReadWrite::GetSecondaryHeader();
template PIMAGE_NT_HEADERS64 PEReadWrite::GetSecondaryHeader();
template PIMAGE_VXD_HEADER PEReadWrite::GetSecondaryHeader();
template PIMAGE_OS2_HEADER PEReadWrite::GetSecondaryHeader();
template UINT_PTR PEReadWrite::GetSecondaryHeader();

template PIMAGE_RUNTIME_FUNCTION_ENTRY PEReadWrite::GetExceptionHandler(int);
template PIMAGE_CE_RUNTIME_FUNCTION_ENTRY PEReadWrite::GetExceptionHandler(int);
template PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY PEReadWrite::GetExceptionHandler(int);
template PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY PEReadWrite::GetExceptionHandler(int);

template PIMAGE_LOAD_CONFIG_DIRECTORY32 PEReadWrite::GetLoadConfigurationTable();
template PIMAGE_LOAD_CONFIG_DIRECTORY64 PEReadWrite::GetLoadConfigurationTable();