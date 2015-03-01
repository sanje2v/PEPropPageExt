#pragma once

#include <ImageHlp.h>
#include <Windows.h>
#include <delayimp.h>
#include <memory>
#include <string>
#include <vector>
#include <udis86.h>
#include "cxxabi.h"
#include "CustomCorDefs.h"
#include "MiscFuncs.h"
#include "unique_handle.h"


// Resource releaser lambda functions
static auto funcFileReleaser = [](HANDLE handle) { CloseHandle(handle); };
static auto funcFileMappingReleaser = [](HANDLE handle) { CloseHandle(handle); };
static auto funcMapViewReleaser = [](LPVOID ptr) { UnmapViewOfFile(ptr); };

class PEReadWrite
{
public:
	// Public types
	struct RichSigVCCompilerInfo
	{
		DWORD id;
		DWORD minver;
		DWORD vnum;
	};
	typedef struct
	{
		WORD offset : 12;
		WORD type : 4;
	} IMAGE_FIXUP_ENTRY, *PIMAGE_FIXUP_ENTRY;
	enum { LCT32_WITHOUT_SE_HANDLER_SIZE = 64, LCT64_WITHOUT_SE_HANDLER_SIZE = 96};
	enum { errUnintendedExecutionPath = -2, errInvalidMemory = -1, noErr = 0 };
	enum class HeaderType { Unknown, MZ, NE, LE, PE };
	enum class MachineType { Unknown, MatsushitaAM33, x64, ARM_LE, EFI_bytecode, I386, Itanium, MitsubishiM32R_LE, MIPS16, MIPSFPU, MIPS16_FPU, PowerPC_LE, PowerPCFPU, MIPS_LE, HitachiSH3, HitachiSH3DSP, HitachiSH3_LE, HitachiSH4, HitachiSH5, Thumb, MIPS_LE_WCEv2, Alpha_AXP, Alpha64 };
	enum class PEType { Unknown, PE32, PE64, ROM };
	enum class ChecksumCheck { Error, NotPresent, Correct, Incorrect };
	enum class ManifestSource { NotPresent, Internal, External };
	enum class SymbolPart { Name, Full };
	enum class ImportType { Static, Delayed };
	enum class FunctionImportType { ByOrdinal, ByName };
	enum class HexViewType { Byte, Word, DWord, QWord };

	static const WORD ID_MANIFEST = 24;

private:
	// NOTE: This class in not copyable
	PEReadWrite& operator=(const PEReadWrite&) = delete;
	PEReadWrite(const PEReadWrite&) = delete;

	// Private types
	static const unsigned int RICH_DATA_START_OFFSET = 0x80UL;
	static const unsigned int RICH_SIGNATURE = 0x68636952UL;		// 'Rich'

	// Private variables
	bool m_isFileOpened;
	wstring m_filename;

	// Cache
	PIMAGE_DOS_HEADER m_pDOSHeader;
	union
	{
		PIMAGE_NT_HEADERS m_pNTHeaders;
		PIMAGE_NT_HEADERS32 m_pNTHeaders32;
		PIMAGE_NT_HEADERS64 m_pNTHeaders64;
	};
	PEType m_PEType;

	// Private members
	unique_handle<HANDLE, decltype(funcFileReleaser)> m_hFile;
	unique_handle<HANDLE, decltype(funcFileMappingReleaser)> m_hFileMappingObject;
	unique_handle<LPVOID, decltype(funcMapViewReleaser)> m_pMapViewBaseAddress;

	// Utility functions
	LPVOID getAddr(DWORD rva, bool isFileOffset = false)
	{
		if (isFileOffset)
			return LPVOID(UINT_PTR(m_pMapViewBaseAddress.get()) + rva);

		// NOTE: If given address is not a file offset, it is a RVA
		PIMAGE_SECTION_HEADER pSectionHeader;
		int err = getSectionHeader(0, std::ref(pSectionHeader));
		if (err)
			return NULL;

		if (rva < pSectionHeader->VirtualAddress)
			return LPVOID(UINT_PTR(m_pMapViewBaseAddress.get()) + rva);
		else
			for (WORD i = 0; i < getNoOfSections(); ++i)
			{
				err = getSectionHeader(i, std::ref(pSectionHeader));
				if (err)
					break;

				DWORD SectionStart = pSectionHeader->VirtualAddress;
				DWORD SectionEnd = SectionStart + (pSectionHeader->Misc.VirtualSize == 0 ? pSectionHeader->SizeOfRawData : pSectionHeader->Misc.VirtualSize);

				if (rva >= SectionStart && rva < SectionEnd)
					return LPVOID(UINT_PTR(m_pMapViewBaseAddress.get())
					              + rva - SectionStart
					              + pSectionHeader->PointerToRawData);
			}

		return NULL;
	}

	bool isMemoryReadable(void *ptr, DWORD size)
	{
		return (UINT_PTR(ptr) >= UINT_PTR(getAddr(0, true)) &&
			    UINT_PTR(ptr) + size < UINT_PTR(getAddr(getFileSize().LowPart, true)));
	}

	LARGE_INTEGER getFileSize()
	{
		LARGE_INTEGER filesize = { 0UL };

		GetFileSizeEx(m_hFile.get(), &filesize);

		return filesize;
	}

	PIMAGE_DATA_DIRECTORY getFirstDataDirectory()
	{
		switch (m_PEType)
		{
			case PEType::PE32: return m_pNTHeaders32->OptionalHeader.DataDirectory;
			case PEType::PE64: return m_pNTHeaders64->OptionalHeader.DataDirectory;
		}

		return NULL;
	}

	PIMAGE_SECTION_HEADER getFirstSectionHeader()
	{
		return IMAGE_FIRST_SECTION(m_pNTHeaders);
	}

	PIMAGE_BASE_RELOCATION getFirstBaseRelocationTable()
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_BASERELOC, std::ref(pDataDir));
		if (err)
			return NULL;

		return PIMAGE_BASE_RELOCATION(getAddr(pDataDir->VirtualAddress));
	}

	PIMAGE_DEBUG_DIRECTORY getFirstDebugDirectory()
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_DEBUG, std::ref(pDataDir));
		if (err)
			return NULL;

		return PIMAGE_DEBUG_DIRECTORY(getAddr(pDataDir->VirtualAddress));
	}

	template <typename T>
	T getFirstExceptionHandler()
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_EXCEPTION, std::ref(pDataDir));
		if (err)
			return NULL;

		return T(getAddr(pDataDir->VirtualAddress));
	}

public:
	// Public functions
	PEReadWrite() throw()
	:	m_isFileOpened(false),
		m_filename(),
		m_pDOSHeader(NULL),
		m_pNTHeaders(NULL),
		m_PEType(PEType::Unknown),
		m_hFile(INVALID_HANDLE_VALUE, funcFileReleaser, INVALID_HANDLE_VALUE),
		m_hFileMappingObject(NULL, funcFileMappingReleaser),
		m_pMapViewBaseAddress(NULL, funcMapViewReleaser) {}
	~PEReadWrite() { close(); }

	bool open(wstring filename) throw()
	{
		// Open file
		unique_handle<HANDLE, decltype(funcFileReleaser)> hFile(CreateFile(filename.c_str(),
																		   GENERIC_READ,
																		   FILE_SHARE_READ,
																		   NULL,
																		   OPEN_EXISTING,
																		   FILE_ATTRIBUTE_NORMAL,
																		   NULL),
																funcFileReleaser,
																INVALID_HANDLE_VALUE);
		if (!hFile)
			return false;

		// Create a file mapping onto memory
		unique_handle<HANDLE, decltype(funcFileMappingReleaser)> hFileMappingObject(CreateFileMapping(hFile.get(),
																									  NULL,
																									  PAGE_READONLY,
																									  0,
																									  0,
																									  NULL),
																					funcFileMappingReleaser);
		if (!hFileMappingObject)
			return false;

		// Open a map view of file
		unique_handle<LPVOID, decltype(funcMapViewReleaser)> pMapViewBaseAddress(MapViewOfFile(hFileMappingObject.get(),
																							   FILE_MAP_READ,
																							   0,
																							   0,
																							   0),
																				funcMapViewReleaser);
		if (!pMapViewBaseAddress)
			return false;

		// Move ownership
		m_filename = std::move(filename);
		m_hFile = std::move(hFile);
		m_hFileMappingObject = std::move(hFileMappingObject);
		m_pMapViewBaseAddress = std::move(pMapViewBaseAddress);
		m_isFileOpened = true;

		// Cache pointers for PE files for fast access
		PIMAGE_DOS_HEADER pDOSHeader = NULL;
		union
		{
			PIMAGE_NT_HEADERS pNTHeaders;
			PIMAGE_NT_HEADERS32 pNTHeaders32;
			PIMAGE_NT_HEADERS64 pNTHeaders64;
		};
		pNTHeaders = NULL;
		PEType typePE = PEType::Unknown;

		int err;
		if (getPrimaryHeaderType() == HeaderType::MZ)
		{
			err = getPrimaryHeader(std::ref(pDOSHeader));

			if (getSecondaryHeaderType() == HeaderType::PE)
			{
				typePE = getPEType();

				switch (typePE)
				{
					case PEType::PE32:
						getSecondaryHeader(std::ref(pNTHeaders32));
						break;

					case PEType::PE64:
						getSecondaryHeader(std::ref(pNTHeaders64));
						break;
				}
			}
		}

		m_pDOSHeader = pDOSHeader;
		m_pNTHeaders = pNTHeaders;
		m_PEType = typePE;

		return true;
	}

	void close()
	{
		// Release resources (make sure they are released in appropriate order)
		// NOTE: "appropriate order" is opposite to the order they are defined
		if (!m_isFileOpened)
			return;

		m_isFileOpened = false;
		m_filename.clear();

		m_pMapViewBaseAddress.~unique_handle();
		m_hFileMappingObject.~unique_handle();
		m_hFile.~unique_handle();
	}

	wstring getFilename()
	{
		return m_filename;
	}

	// Header functions
	HeaderType getPrimaryHeaderType()
	{
		LPWORD pMagicNumber;
		int err = getPrimaryHeader(std::ref(pMagicNumber));
		if (err)
			return HeaderType::Unknown;

		switch (*pMagicNumber)
		{
			case IMAGE_DOS_SIGNATURE: return HeaderType::MZ;
		}

		return HeaderType::Unknown;
	}

	template<typename T>
	int getPrimaryHeader(std::reference_wrapper<T> ptr)
	{
		T pPrimaryHeader = T(getAddr(0, true));
		if (!isMemoryReadable(pPrimaryHeader, sizeof(*pPrimaryHeader)))
			return errInvalidMemory;

		ptr.get() = pPrimaryHeader;

		return noErr;
	}

	HeaderType getSecondaryHeaderType()
	{
		PIMAGE_DOS_HEADER pDOSHeader;
		int err = getPrimaryHeader(std::ref(pDOSHeader));
		if (err)
			return HeaderType::Unknown;

		LPDWORD pMagicNumber = LPDWORD(getAddr(pDOSHeader->e_lfanew, true));
		if (!isMemoryReadable(pMagicNumber, sizeof(DWORD)))
			return HeaderType::Unknown;

		switch (*pMagicNumber)
		{
			case IMAGE_NT_SIGNATURE: return HeaderType::PE;
			case DWORD(IMAGE_OS2_SIGNATURE): return HeaderType::NE;
			case DWORD(IMAGE_VXD_SIGNATURE): return HeaderType::LE;
		}

		return HeaderType::Unknown;
	}

	template<typename T>
	int getSecondaryHeader(std::reference_wrapper<T> ptr)
	{
		assert(getPrimaryHeaderType() == HeaderType::MZ);

		PIMAGE_DOS_HEADER pDOSHeader;
		int err = getPrimaryHeader(std::ref(pDOSHeader));
		if (err)
			return err;

		T pSecondaryHeader = T(getAddr(pDOSHeader->e_lfanew, true));
		if (!isMemoryReadable(pSecondaryHeader, sizeof(*pSecondaryHeader)))
			return errInvalidMemory;

		ptr.get() = pSecondaryHeader;

		return noErr;
	}

	PEType getPEType()
	{
		assert(getSecondaryHeaderType() == HeaderType::PE);
		
		PIMAGE_NT_HEADERS pNTHeaders;
		int err = getSecondaryHeader(std::ref(pNTHeaders));
		if (err)
			return PEType::Unknown;

		switch (pNTHeaders->OptionalHeader.Magic)
		{
			case IMAGE_NT_OPTIONAL_HDR32_MAGIC: return PEType::PE32;
			case IMAGE_NT_OPTIONAL_HDR64_MAGIC: return PEType::PE64;
			case IMAGE_ROM_OPTIONAL_HDR_MAGIC: return PEType::ROM;
		}

		return PEType::Unknown;
	}

	int getROMHeader(PIMAGE_ROM_OPTIONAL_HEADER& ptr)
	{
		assert(getPEType() == PEType::ROM);

		PIMAGE_NT_HEADERS pSecondaryHeader;
		int err = getSecondaryHeader(std::ref(pSecondaryHeader));
		if (err)
			return err;

		PIMAGE_ROM_OPTIONAL_HEADER pROMheader = PIMAGE_ROM_OPTIONAL_HEADER(UINT_PTR(pSecondaryHeader) + sizeof(IMAGE_FILE_HEADER));
		if (!isMemoryReadable(pROMheader, sizeof(*pROMheader)))
			return errInvalidMemory;

		ptr = pROMheader;

		return noErr;
	}

	MachineType getMachineType()
	{
		switch (m_pNTHeaders->FileHeader.Machine)
		{
			case IMAGE_FILE_MACHINE_AM33: return MachineType::MatsushitaAM33;
			case IMAGE_FILE_MACHINE_AMD64: return MachineType::x64;
			case IMAGE_FILE_MACHINE_ARM: return MachineType::ARM_LE;
			case IMAGE_FILE_MACHINE_EBC: return MachineType::EFI_bytecode;
			case IMAGE_FILE_MACHINE_I386: return MachineType::I386;
			case IMAGE_FILE_MACHINE_IA64: return MachineType::Itanium;
			case IMAGE_FILE_MACHINE_M32R: return MachineType::MitsubishiM32R_LE;
			case IMAGE_FILE_MACHINE_MIPS16: return MachineType::MIPS16;
			case IMAGE_FILE_MACHINE_MIPSFPU: return MachineType::MIPSFPU;
			case IMAGE_FILE_MACHINE_MIPSFPU16: return MachineType::MIPS16_FPU;
			case IMAGE_FILE_MACHINE_POWERPC: return MachineType::PowerPC_LE;
			case IMAGE_FILE_MACHINE_POWERPCFP: return MachineType::PowerPCFPU;
			case IMAGE_FILE_MACHINE_R4000: return MachineType::MIPS_LE;
			case IMAGE_FILE_MACHINE_SH3: return MachineType::HitachiSH3;
			case IMAGE_FILE_MACHINE_SH3DSP: return MachineType::HitachiSH3DSP;
			case IMAGE_FILE_MACHINE_SH4: return MachineType::HitachiSH4;
			case IMAGE_FILE_MACHINE_SH5: return MachineType::HitachiSH5;
			case IMAGE_FILE_MACHINE_THUMB: return MachineType::Thumb;
			case IMAGE_FILE_MACHINE_WCEMIPSV2: return MachineType::MIPS_LE_WCEv2;
			case IMAGE_FILE_MACHINE_ALPHA: return MachineType::Alpha_AXP;
			case IMAGE_FILE_MACHINE_ALPHA64: return MachineType::Alpha64;
		}

		return MachineType::Unknown;
	}

	bool hasInvalidNoOfDataDirectories()
	{
		switch (m_PEType)
		{
			case PEType::PE32:
				return (m_pNTHeaders32->OptionalHeader.NumberOfRvaAndSizes > IMAGE_NUMBEROF_DIRECTORY_ENTRIES);

			case PEType::PE64:
				return (m_pNTHeaders64->OptionalHeader.NumberOfRvaAndSizes > IMAGE_NUMBEROF_DIRECTORY_ENTRIES);
		}

		return true;
	}
	
	DWORD getNoOfDataDirectories_Corrected()
	{
		switch (m_PEType)
		{
			case PEType::PE32:
				return min(m_pNTHeaders32->OptionalHeader.NumberOfRvaAndSizes, IMAGE_NUMBEROF_DIRECTORY_ENTRIES);

			case PEType::PE64:
				return min(m_pNTHeaders64->OptionalHeader.NumberOfRvaAndSizes, IMAGE_NUMBEROF_DIRECTORY_ENTRIES);
		}

		return 0;
	}

	int getDataDirectory(int index, PIMAGE_DATA_DIRECTORY& ptr)
	{
		PIMAGE_DATA_DIRECTORY pDataDir = &getFirstDataDirectory()[index];
		if (!isMemoryReadable(pDataDir, sizeof(*pDataDir)))
			return errInvalidMemory;

		ptr = pDataDir;

		return noErr;
	}

	int getSectionHeader(int index, PIMAGE_SECTION_HEADER& ptr)
	{
		PIMAGE_SECTION_HEADER pSectionHeader = &getFirstSectionHeader()[index];
		if (!isMemoryReadable(pSectionHeader, sizeof(*pSectionHeader)))
			return errInvalidMemory;

		ptr = pSectionHeader;

		return noErr;
	}

#pragma region //////////////////////////////// Manifest functions ///////////////////////////////////
	wstring makeManifestFilename()
	{
		// External manifest files are required to be named
		// '<EXE or DLL name>.<Extension>.manifest'
		return m_filename + L".manifest";
	}

	ManifestSource getManifestSource()
	{
		// Figure out if resource section contains a manifest,
		// there is an external manifest file or there are no manifests
		// defined for this file
		
		// See if an internal manifest is defined
		if (hasResources())
		{
			PIMAGE_RESOURCE_DIRECTORY pResDir;
			int err = getRootResourceDirectory(std::ref(pResDir));
			if (err == noErr)
			{
				const DWORD NumberOfEntries = DWORD(pResDir->NumberOfNamedEntries) + DWORD(pResDir->NumberOfIdEntries);

				for (DWORD i = 0; i < NumberOfEntries; ++i)
				{
					PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry;
					err = getResourceDirectoryEntry(std::cref(pResDir), i, std::ref(pResDirEntry));
					if (err)
						break;

					if (pResDirEntry->NameIsString)
					{
						// Entry has a name string
						wstring EntryName;
						err = getResourceEntryName(std::cref(pResDirEntry), std::ref(EntryName));
						if (err)
							break;

						// Name string is case insensitive
						std::transform(EntryName.cbegin(), EntryName.cend(), EntryName.begin(), ::tolower);

						if (EntryName == L"manifest")
							return ManifestSource::Internal;
					}
					else
					{
						// Entry has an Id
						if (pResDirEntry->Id == ID_MANIFEST)
							return ManifestSource::Internal;
					}
				}
			}
		}

		if (PathFileExists(makeManifestFilename().c_str()))
			return ManifestSource::External;

		return ManifestSource::NotPresent;
	}
#pragma endregion

#pragma region //////////////////////////////// Import functions ///////////////////////////////////
	bool hasImports()
	{
		// Tests if either normal Imports or Delayed Imports are available
		return hasStaticImports() || hasDelayedImports();
	}

	bool isImportsAlreadyBound()
	{
		if (getNoOfDataDirectories_Corrected() >= IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT + 1)
		{
			PIMAGE_DATA_DIRECTORY pDataDir;
			int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT, std::ref(pDataDir));
			if (err)
				return false;

			return (pDataDir->Size != 0);
		}

		return false;
	}

	int getNoOfImportModules()
	{
		return (hasStaticImports() ? getNoOfStaticImportModules() : 0) + (hasDelayedImports() ? getNoOfDelayedImportModules() : 0);
	}
	
	bool hasStaticImports()
	{
		if (getNoOfDataDirectories_Corrected() >= IMAGE_DIRECTORY_ENTRY_IMPORT + 1)
		{
			PIMAGE_DATA_DIRECTORY pDataDir;
			int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT, std::ref(pDataDir));
			if (err)
				return false;

			return (pDataDir->Size != 0);
		}
			
		return false;
	}

	int getNoOfStaticImportModules()
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT, std::ref(pDataDir));
		if (err)
			return 0;

		const IMAGE_IMPORT_DESCRIPTOR NullTerminatorDescriptor = { 0 };
		PIMAGE_IMPORT_DESCRIPTOR pImportDesc = PIMAGE_IMPORT_DESCRIPTOR(getAddr(pDataDir->VirtualAddress));

		int Count = 0;
		while (isMemoryReadable(pImportDesc, sizeof(*pImportDesc)))
		{
			if (memcmp(pImportDesc, &NullTerminatorDescriptor, sizeof(*pImportDesc)) == 0)
				break;

			++Count;
			++pImportDesc;
		}

		return Count;
	}

	int getStaticModuleDescriptor(int i, PIMAGE_IMPORT_DESCRIPTOR& ptr)
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_IMPORT, std::ref(pDataDir));
		if (err)
			return errInvalidMemory;

		PIMAGE_IMPORT_DESCRIPTOR pImportDesc = PIMAGE_IMPORT_DESCRIPTOR(getAddr(pDataDir->VirtualAddress)) + i;
		if (!isMemoryReadable(pImportDesc, sizeof(*pImportDesc)))
			return errInvalidMemory;

		ptr = pImportDesc;

		return noErr;
	}

	wstring getStaticModuleName(const PIMAGE_IMPORT_DESCRIPTOR& pImportDesc)
	{
		return MultiByte_toString(LPSTR(getAddr(pImportDesc->Name)));
	}

	int getNoOfStaticModuleImports(const PIMAGE_IMPORT_DESCRIPTOR& pImportDesc)
	{
		int Count = 0;

		switch (m_PEType)
		{
			case PEType::PE32:
			{
				const IMAGE_THUNK_DATA32 NullTerminatorDescriptor = { 0 };
				PIMAGE_THUNK_DATA32 pThunkData = PIMAGE_THUNK_DATA32(getAddr(pImportDesc->OriginalFirstThunk));

				while (isMemoryReadable(pThunkData, sizeof(*pThunkData)))
				{
					if (memcmp(pThunkData, &NullTerminatorDescriptor, sizeof(NullTerminatorDescriptor)) == 0)
						break;

					++Count;
					++pThunkData;
				}
			}
			break;

			case PEType::PE64:
			{
				const IMAGE_THUNK_DATA64 NullTerminatorDescriptor = { 0 };
				PIMAGE_THUNK_DATA64 pThunkData = PIMAGE_THUNK_DATA64(getAddr(pImportDesc->OriginalFirstThunk));

				while (isMemoryReadable(pThunkData, sizeof(*pThunkData)))
				{
					if (memcmp(pThunkData, &NullTerminatorDescriptor, sizeof(NullTerminatorDescriptor)) == 0)
						break;

					++Count;
					++pThunkData;
				}
			}
			break;
		}

		return Count;
	}

	template<typename T>
	int getStaticModuleImport(const PIMAGE_IMPORT_DESCRIPTOR& pImportDesc,
							  int i,
							  std::reference_wrapper<T> ptr)
	{
		T pThunkData = T(getAddr(pImportDesc->OriginalFirstThunk)) + i;
		if (!isMemoryReadable(pThunkData, sizeof(*pThunkData)))
			return errInvalidMemory;

		ptr.get() = pThunkData;

		return noErr;
	}

	template<typename T>
	int getModuleImportOrdinalOrName(const reference_wrapper<T> pThunkData,
									 WORD& Ordinal,
									 WORD& Hint,
									 string& Name,
									 bool& hasOrdinal)
	{
		switch (m_PEType)
		{
			case PEType::PE32:
			{
				hasOrdinal = IMAGE_SNAP_BY_ORDINAL32(pThunkData.get()->u1.Ordinal);

				if (hasOrdinal)
					Ordinal = WORD(pThunkData.get()->u1.Ordinal & 0xFFFF);
				else
				{
					PIMAGE_IMPORT_BY_NAME pImportName = PIMAGE_IMPORT_BY_NAME(getAddr(pThunkData.get()->u1.AddressOfData & ~IMAGE_ORDINAL_FLAG32));
					if (!isMemoryReadable(pImportName, sizeof(*pImportName)))
						return errInvalidMemory;

					if (!isMemoryReadable(LPSTR(pImportName->Name), 2))
						return errInvalidMemory;

					Hint = pImportName->Hint;
					Name = LPSTR(pImportName->Name);
				}
			}
			break;

			case PEType::PE64:
			{
				hasOrdinal = IMAGE_SNAP_BY_ORDINAL64(pThunkData.get()->u1.Ordinal);

				if (hasOrdinal)
					Ordinal = WORD(pThunkData.get()->u1.Ordinal & 0xFFFF);
				else
				{
					PIMAGE_IMPORT_BY_NAME pImportName = PIMAGE_IMPORT_BY_NAME(getAddr(DWORD(pThunkData.get()->u1.AddressOfData & ~IMAGE_ORDINAL_FLAG64)));
					if (!isMemoryReadable(pImportName, sizeof(*pImportName)))
						return errInvalidMemory;

					if (!isMemoryReadable(LPSTR(pImportName->Name), 2))
						return errInvalidMemory;

					Hint = pImportName->Hint;
					Name = LPSTR(pImportName->Name);
				}
			}
			break;
		}

		return noErr;
	}

	bool hasDelayedImports()
	{
		if (getNoOfDataDirectories_Corrected() >= IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT + 1)
		{
			PIMAGE_DATA_DIRECTORY pDataDir;
			int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT, std::ref(pDataDir));
			if (err)
				return false;

			return (pDataDir->Size != 0);
		}

		return false;
	}

	int getNoOfDelayedImportModules()
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT, std::ref(pDataDir));
		if (err)
			return 0;

		const ImgDelayDescr NullTerminatorDescriptor = { 0 };
		PImgDelayDescr pImportDescriptor = PImgDelayDescr(getAddr(pDataDir->VirtualAddress));

		int Count = 0;
		while (isMemoryReadable(pImportDescriptor, sizeof(*pImportDescriptor)))
		{
			if (memcmp(pImportDescriptor, &NullTerminatorDescriptor, sizeof(*pImportDescriptor)) == 0)
				break;

			++Count;
			++pImportDescriptor;
		}

		return Count;
	}

	int getDelayedModuleDescriptor(int i, PImgDelayDescr& ptr)
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT, std::ref(pDataDir));
		if (err)
			return errInvalidMemory;

		PImgDelayDescr pImportDescriptor = PImgDelayDescr(getAddr(pDataDir->VirtualAddress)) + i;
		if (!isMemoryReadable(pImportDescriptor, sizeof(*pImportDescriptor)))
			return errInvalidMemory;

		ptr = pImportDescriptor;

		return noErr;
	}

	wstring getDelayedModuleName(const PImgDelayDescr& pImportDesc)
	{
		return MultiByte_toString(LPSTR(getAddr(pImportDesc->rvaDLLName)));
	}

	int getNoOfDelayedModuleImports(const PImgDelayDescr& pImportDesc)
	{
		int Count = 0;

		switch (m_PEType)
		{
			case PEType::PE32:
			{
				const IMAGE_THUNK_DATA32 NullTerminatorDescriptor = { 0 };
				PIMAGE_THUNK_DATA32 pThunkData = PIMAGE_THUNK_DATA32(getAddr(pImportDesc->rvaINT));

				while (isMemoryReadable(pThunkData, sizeof(*pThunkData)))
				{
					if (memcmp(pThunkData, &NullTerminatorDescriptor, sizeof(NullTerminatorDescriptor)) == 0)
						break;

					++Count;
					++pThunkData;
				}
			}
			break;

			case PEType::PE64:
			{
				const IMAGE_THUNK_DATA64 NullTerminatorDescriptor = { 0 };
				PIMAGE_THUNK_DATA64 pThunkData = PIMAGE_THUNK_DATA64(getAddr(pImportDesc->rvaINT));

				while (isMemoryReadable(pThunkData, sizeof(*pThunkData)))
				{
					if (memcmp(pThunkData, &NullTerminatorDescriptor, sizeof(NullTerminatorDescriptor)) == 0)
						break;

					++Count;
					++pThunkData;
				}
			}
			break;
		}

		return Count;
	}

	template<typename T>
	int getDelayedModuleImport(const PImgDelayDescr& pImportDesc, int i, std::reference_wrapper<T> ptr)
	{
		T pThunkData = T(getAddr(pImportDesc->rvaINT)) + i;
		if (!isMemoryReadable(pThunkData, sizeof(*pThunkData)))
			return errInvalidMemory;

		ptr.get() = pThunkData;

		return noErr;
	}
#pragma endregion

#pragma region //////////////////////////////// Export functions ///////////////////////////////////
	bool hasExports()
	{
		if (getNoOfDataDirectories_Corrected() >= IMAGE_DIRECTORY_ENTRY_EXPORT + 1)
		{
			PIMAGE_DATA_DIRECTORY pDataDir;
			int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT, std::ref(pDataDir));
			if (err)
				return false;

			return (pDataDir->Size != 0);
		}

		return false;
	}
	
	int getExportDirectory(PIMAGE_EXPORT_DIRECTORY& ptr)
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT, std::ref(pDataDir));
		if (err)
			return err;

		PIMAGE_EXPORT_DIRECTORY pExportDir = PIMAGE_EXPORT_DIRECTORY(getAddr(pDataDir->VirtualAddress));
		if (!isMemoryReadable(pExportDir, sizeof(*pExportDir)))
			return errInvalidMemory;

		ptr = pExportDir;

		return noErr;
	}

	int getExportImageName(const PIMAGE_EXPORT_DIRECTORY& pExportDir, wstring& name)
	{
		PCHAR pName = PCHAR(getAddr(pExportDir->Name));
		if (!isMemoryReadable(pName, 2))
			return errInvalidMemory;

		name = MultiByte_toString(pName);

		return noErr;
	}

	int getExportOrdinalsTable(const PIMAGE_EXPORT_DIRECTORY& pExportDir, LPWORD& ptr)
	{
		LPWORD pOrdinalsTable = LPWORD(getAddr(pExportDir->AddressOfNameOrdinals));
		if (!isMemoryReadable(pOrdinalsTable, sizeof(WORD) * pExportDir->NumberOfNames))
			return errInvalidMemory;

		ptr = pOrdinalsTable;

		return noErr;
	}

	WORD getExportOrdinal(LPWORD pOrdinalsTable, int index)
	{
		return pOrdinalsTable[index];
	}

	int getExportNamesTable(const PIMAGE_EXPORT_DIRECTORY& pExportDir, LPDWORD& ptr)
	{
		LPDWORD pNamesTable = LPDWORD(getAddr(pExportDir->AddressOfNames));
		if (!isMemoryReadable(pNamesTable, sizeof(DWORD) * pExportDir->NumberOfNames))
			return errInvalidMemory;

		ptr = pNamesTable;

		return noErr;
	}

	string getExportName(const LPDWORD& pNamesTable, int index)
	{
		return (char *)getAddr(pNamesTable[index]);
	}

	bool isExportForwarderName(DWORD rva)
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_EXPORT, std::ref(pDataDir));
		if (err)
			return false;

		return (rva >= pDataDir->VirtualAddress && rva < pDataDir->VirtualAddress + pDataDir->Size);
	}

	wstring getExportForwarderName(DWORD rva)
	{
		return MultiByte_toString((char *)getAddr(rva));
	}

	int getExportRVAs(const PIMAGE_EXPORT_DIRECTORY& pExportDir, LPDWORD& ptr)
	{
		LPDWORD pRVAs = LPDWORD(getAddr(pExportDir->AddressOfFunctions));
		if (!isMemoryReadable(pRVAs, sizeof(*pRVAs) * pExportDir->NumberOfFunctions))
			return errInvalidMemory;

		ptr = pRVAs;

		return noErr;
	}
#pragma endregion

#pragma region //////////////////////////// Load Configuration functions ///////////////////////////
	bool hasLoadConfigData()
	{
		if (getNoOfDataDirectories_Corrected() >= IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG + 1)
		{
			PIMAGE_DATA_DIRECTORY pDataDir;
			int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, std::ref(pDataDir));
			if (err)
				return false;

			return (pDataDir->Size != 0);
		}

		return false;
	}

	template<typename T>
	int getLoadConfigurationTable(reference_wrapper<T> ptr)
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, std::ref(pDataDir));
		if (err)
			return errInvalidMemory;

		T pLCT = T(getAddr(pDataDir->VirtualAddress));
		if (!isMemoryReadable(pLCT, sizeof(*pLCT)))
			return errInvalidMemory;

		ptr.get() = pLCT;

		return noErr;
	}

	int getLoadConfigurationTableSize()
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, std::ref(pDataDir));
		if (err)
			return 0;

		return pDataDir->Size;
	}

	DWORD getSecurityCookie(const PIMAGE_LOAD_CONFIG_DIRECTORY32& pLoadConfig32)
	{
		LPDWORD pSecurityCookie = LPDWORD(getAddr(VAToRVA(pLoadConfig32->SecurityCookie)));
		if (!isMemoryReadable(pSecurityCookie, sizeof(*pSecurityCookie)))
			return 0;

		return *pSecurityCookie;
	}

	ULONGLONG getSecurityCookie(const PIMAGE_LOAD_CONFIG_DIRECTORY64& pLoadConfig64)
	{
		PULONGLONG pSecurityCookie = PULONGLONG(getAddr(VAToRVA(pLoadConfig64->SecurityCookie)));
		if (!isMemoryReadable(pSecurityCookie, sizeof(*pSecurityCookie)))
			return 0;

		return *pSecurityCookie;
	}

	DWORD getStructuredExceptionHandler(const PIMAGE_LOAD_CONFIG_DIRECTORY32& pLoadConfig32, int i)
	{
		LPDWORD pSEH = &(LPDWORD(getAddr(VAToRVA(pLoadConfig32->SEHandlerTable)))[i]);
		if (!isMemoryReadable(pSEH, sizeof(*pSEH)))
			return 0;

		return *pSEH;
	}

	ULONGLONG getStructuredExceptionHandler(const PIMAGE_LOAD_CONFIG_DIRECTORY64& pLoadConfig64, int i)
	{
		PULONGLONG pSEH = &(PULONGLONG(getAddr(VAToRVA(pLoadConfig64->SEHandlerTable)))[i]);
		if (!isMemoryReadable(pSEH, sizeof(*pSEH)))
			return 0;

		return *pSEH;
	}
#pragma endregion

#pragma region //////////////////////////////// TLS functions //////////////////////////////////////
	bool hasTLSData()
	{
		if (getNoOfDataDirectories_Corrected() >= IMAGE_DIRECTORY_ENTRY_TLS + 1)
		{
			PIMAGE_DATA_DIRECTORY pDataDir;
			int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_TLS, std::ref(pDataDir));
			if (err)
				return false;

			return (pDataDir->Size != 0);
		}

		return false;
	}

	template<typename T>
	int getTLSDirectory(reference_wrapper<T> ptr)
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_TLS, std::ref(pDataDir));
		if (err)
			return err;

		T pTLS = T(getAddr(pDataDir->VirtualAddress));
		if (!isMemoryReadable(pTLS, sizeof(*pTLS)))
			return errInvalidMemory;

		ptr.get() = pTLS;

		return noErr;
	}

	int getTLSTableSize()
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_TLS, std::ref(pDataDir));
		if (err)
			return 0;

		return pDataDir->Size;
	}

	DWORD getTLSCallbackByIndex(const PIMAGE_TLS_DIRECTORY32& pTLSData, int i)
	{
		LPDWORD pTLSCallbacks = &(LPDWORD(getAddr(VAToRVA(pTLSData->AddressOfCallBacks)))[i]);
		if (!isMemoryReadable(pTLSCallbacks, sizeof(*pTLSCallbacks)))
			return 0;

		return *pTLSCallbacks;
	}

	ULONGLONG getTLSCallbackByIndex(const PIMAGE_TLS_DIRECTORY64& pTLSData, int i)
	{
		PULONGLONG pTLSCallbacks = &(PULONGLONG(getAddr(VAToRVA(pTLSData->AddressOfCallBacks)))[i]);
		if (!isMemoryReadable(pTLSCallbacks, sizeof(*pTLSCallbacks)))
			return 0; 

		return *pTLSCallbacks;
	}
#pragma endregion

#pragma region ///////////////////////////// Base Relocation functions /////////////////////////////
	bool hasBaseRelocationData()
	{
		if (getNoOfDataDirectories_Corrected() >= IMAGE_DIRECTORY_ENTRY_BASERELOC + 1)
		{
			PIMAGE_DATA_DIRECTORY pDataDir;
			int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_BASERELOC, std::ref(pDataDir));
			if (err)
				return false;

			return (pDataDir->Size != 0);
		}

		return false;
	}

	int getNoOfBaseRelocationTables()
	{
		PIMAGE_BASE_RELOCATION pBaseRelocTable = getFirstBaseRelocationTable();
		int Count = 0;

		while (isMemoryReadable(pBaseRelocTable, sizeof(*pBaseRelocTable)))
		{
			if (pBaseRelocTable->SizeOfBlock == 0)
				break;

			++Count;

			pBaseRelocTable = PIMAGE_BASE_RELOCATION(UINT_PTR(pBaseRelocTable) + pBaseRelocTable->SizeOfBlock);
		}

		return Count;
	}

	int getBaseRelocationTable(int index, PIMAGE_BASE_RELOCATION& ptr)
	{
		PIMAGE_BASE_RELOCATION pBaseRelocTable = getFirstBaseRelocationTable();
		if (!isMemoryReadable(pBaseRelocTable, sizeof(*pBaseRelocTable)))
			return errInvalidMemory;

		int Count = 0;

		while (Count != index)
		{
			if (!isMemoryReadable(pBaseRelocTable, sizeof(*pBaseRelocTable)))
				return errInvalidMemory;

			++Count;

			pBaseRelocTable = PIMAGE_BASE_RELOCATION(UINT_PTR(pBaseRelocTable) + pBaseRelocTable->SizeOfBlock);
		}

		ptr = pBaseRelocTable;

		return noErr;
	}

	int getNoOfBaseRelocationEntries(const PIMAGE_BASE_RELOCATION& pBaseRelocTable)
	{
		if (pBaseRelocTable->SizeOfBlock < sizeof(IMAGE_BASE_RELOCATION))
			return 0;

		return (pBaseRelocTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(IMAGE_FIXUP_ENTRY);
	}

	int getBaseRelocationEntry(const PIMAGE_BASE_RELOCATION& pBaseRelocTable, int rowindex, PIMAGE_FIXUP_ENTRY& ptr)
	{
		PIMAGE_FIXUP_ENTRY pBaseRelocEntry = &(PIMAGE_FIXUP_ENTRY(UINT_PTR(pBaseRelocTable) + sizeof(IMAGE_BASE_RELOCATION))[rowindex]);
		if (!isMemoryReadable(pBaseRelocEntry, sizeof(*pBaseRelocEntry)))
			return errInvalidMemory;

		ptr = pBaseRelocEntry;

		return noErr;
	}
#pragma endregion

#pragma region //////////////////////////////// Debug Info functions ///////////////////////////////
	bool hasDebugData()
	{
		if (getNoOfDataDirectories_Corrected() >= IMAGE_DIRECTORY_ENTRY_DEBUG + 1)
		{
			PIMAGE_DATA_DIRECTORY pDataDir;
			int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_DEBUG, std::ref(pDataDir));
			if (err)
				return false;

			return (pDataDir->Size != 0);
		}

		return false;
	}

	int getDebugDirectory(int index, PIMAGE_DEBUG_DIRECTORY& ptr)
	{
		PIMAGE_DEBUG_DIRECTORY pDebugDir = &getFirstDebugDirectory()[index];
		if (!isMemoryReadable(pDebugDir, sizeof(*pDebugDir)))
			return errInvalidMemory;

		ptr = pDebugDir;

		return noErr;
	}

	int getNoOfDebugDirs()
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_DEBUG, std::ref(pDataDir));
		if (err)
			return 0;

		if (pDataDir->Size == 0)
			return 0;

		return pDataDir->Size / sizeof(IMAGE_DEBUG_DIRECTORY);
	}

	wstring getDebugFilename(const PIMAGE_DEBUG_DIRECTORY& pDebugDir)
	{
		return (pDebugDir->Type == IMAGE_DEBUG_TYPE_MISC ?
				MultiByte_toString(PCHAR(getAddr(pDebugDir->AddressOfRawData))) : L"");
	}

	template<typename T>
	int getDebugData(const PIMAGE_DEBUG_DIRECTORY& pDebugDir, std::reference_wrapper<T> ptr)
	{
		T pDebugData = T(pDebugDir->AddressOfRawData != 0 ? getAddr(pDebugDir->AddressOfRawData) : getAddr(pDebugDir->PointerToRawData, true));
		if (!isMemoryReadable(pDebugData, sizeof(*pDebugData)))
			return errInvalidMemory;

		ptr.get() = pDebugData;

		return noErr;
	}

#pragma endregion

#pragma region /////////////////////////// Exception handling functions ////////////////////////////
	bool hasExceptionHandlingData()
	{
		if (getNoOfDataDirectories_Corrected() >= IMAGE_DIRECTORY_ENTRY_EXCEPTION + 1)
		{
			PIMAGE_DATA_DIRECTORY pDataDir;
			int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_EXCEPTION, std::ref(pDataDir));
			if (err)
				return false;

			return (pDataDir->Size != 0);
		}

		return false;
	}

	template <typename T>
	int getExceptionHandler(reference_wrapper<T> ptr, int index)
	{
		T pExceptionHandler = &(getFirstExceptionHandler<T>()[index]);
		if (!isMemoryReadable(pExceptionHandler, sizeof(*pExceptionHandler)))
			return errInvalidMemory;

		ptr.get() = pExceptionHandler;

		return noErr;
	}

	int getExceptionHandlingDataSize()
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_EXCEPTION, std::ref(pDataDir));
		if (err)
			return err;

		return pDataDir->Size;
	}

	int getNoOfExceptionHandlers()
	{
		switch (getMachineType())
		{
			case MachineType::Alpha_AXP:
				return (getExceptionHandlingDataSize() / sizeof(IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY));

			case MachineType::Alpha64:
				return (getExceptionHandlingDataSize() / sizeof(IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY));

			case MachineType::ARM_LE:
			case MachineType::PowerPC_LE:
			case MachineType::PowerPCFPU:
			case MachineType::HitachiSH3:
			case MachineType::HitachiSH3DSP:
			case MachineType::HitachiSH3_LE:
			case MachineType::HitachiSH4:
				return (getExceptionHandlingDataSize() / sizeof(IMAGE_CE_RUNTIME_FUNCTION_ENTRY));

			case MachineType::x64:
			case MachineType::Itanium:
				return (getExceptionHandlingDataSize() / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY));
		}

		return 0;
	}
#pragma endregion

#pragma region //////////////////////////// Generic resource functions /////////////////////////////
	bool hasResources()
	{
		if (getNoOfDataDirectories_Corrected() >= IMAGE_DIRECTORY_ENTRY_RESOURCE)
		{
			PIMAGE_DATA_DIRECTORY pDataDir;
			int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_RESOURCE, std::ref(pDataDir));
			if (err)
				return false;

			return (pDataDir->Size != 0);
		}

		return false;
	}

	int getRootResourceDirectory(PIMAGE_RESOURCE_DIRECTORY& ptr)
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_RESOURCE, std::ref(pDataDir));
		if (err)
			return err;

		PIMAGE_RESOURCE_DIRECTORY pResDir = PIMAGE_RESOURCE_DIRECTORY(getAddr(pDataDir->VirtualAddress));
		if (!isMemoryReadable(pResDir, sizeof(*pResDir)))
			return errInvalidMemory;

		ptr = pResDir;

		return noErr;
	}

	int getNextResourceDirectory(const PIMAGE_RESOURCE_DIRECTORY_ENTRY& pResDirEntry, PIMAGE_RESOURCE_DIRECTORY& ptr)
	{
		PIMAGE_RESOURCE_DIRECTORY pRootResDir;
		int err = getRootResourceDirectory(std::ref(pRootResDir));
		assert(err == noErr);

		PIMAGE_RESOURCE_DIRECTORY pResDir = PIMAGE_RESOURCE_DIRECTORY(UINT_PTR(pRootResDir) + pResDirEntry->OffsetToDirectory);
		if (!isMemoryReadable(pResDir, sizeof(*pResDir)))
			return errInvalidMemory;

		ptr = pResDir;

		return noErr;
	}

	int getResourceDirectoryEntry(const PIMAGE_RESOURCE_DIRECTORY& pResDir, int i, PIMAGE_RESOURCE_DIRECTORY_ENTRY& ptr)
	{
		PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry = PIMAGE_RESOURCE_DIRECTORY_ENTRY(pResDir + 1) + i;
		if (!isMemoryReadable(pResDirEntry, sizeof(*pResDirEntry)))
			return errInvalidMemory;

		ptr = pResDirEntry;

		return noErr;
	}

	int getResourceEntryName(const PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry, wstring& out)
	{
		assert(pResDirEntry->NameIsString);

		PIMAGE_RESOURCE_DIRECTORY pResDir;
		int err = getRootResourceDirectory(std::ref(pResDir));
		assert(err == noErr);

		PIMAGE_RESOURCE_DIR_STRING_U pResDirStr = PIMAGE_RESOURCE_DIR_STRING_U(UINT_PTR(pResDir) + pResDirEntry->NameOffset);
		if (!isMemoryReadable(pResDirStr, sizeof(*pResDirStr)))
			return errInvalidMemory;

		if (!isMemoryReadable(pResDirStr->NameString, pResDirStr->Length * sizeof(WCHAR)))
			return errInvalidMemory;

		out = wstring(pResDirStr->NameString, pResDirStr->Length);

		return noErr;
	}

	WORD getResourceEntryId(const PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry)
	{
		assert(!pResDirEntry->NameIsString);

		return pResDirEntry->Id;
	}

	int getResourceEntryData(const PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry, PIMAGE_RESOURCE_DATA_ENTRY& ptr)
	{
		PIMAGE_RESOURCE_DIRECTORY pResDir;
		int err = getRootResourceDirectory(std::ref(pResDir));
		assert(err == noErr);

		PIMAGE_RESOURCE_DATA_ENTRY pResData = PIMAGE_RESOURCE_DATA_ENTRY(UINT_PTR(pResDir) + pResDirEntry->OffsetToData);
		if (!isMemoryReadable(pResData, sizeof(*pResData)))
			return errInvalidMemory;

		ptr = pResData;

		return noErr;
	}

	int getResourceData(const PIMAGE_RESOURCE_DATA_ENTRY& pResData, LPBYTE& pData, DWORD& cData)
	{
		LPBYTE ptr = LPBYTE(getAddr(pResData->OffsetToData));
		if (!isMemoryReadable(ptr, pResData->Size))
			return errInvalidMemory;

		pData = ptr;
		cData = pResData->Size;

		return noErr;
	}
#pragma endregion

#pragma region /////////////////////////////////// CLR functions ///////////////////////////////////
	bool hasCLRData()
	{
		if (getNoOfDataDirectories_Corrected() >= IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR + 1)
		{
			PIMAGE_DATA_DIRECTORY pDataDir;
			int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR, std::ref(pDataDir));
			if (err)
				return false;

			return (pDataDir->Size != 0);
		}

		return false;
	}

	int getCLRHeader(PIMAGE_COR20_HEADER& ptr)
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR, std::ref(pDataDir));
		if (err)
			return err;

		PIMAGE_COR20_HEADER pCLRHeader = PIMAGE_COR20_HEADER(getAddr(pDataDir->VirtualAddress));
		if (!isMemoryReadable(pCLRHeader, sizeof(*pCLRHeader)))
			return errInvalidMemory;

		ptr = pCLRHeader;

		return noErr;
	}

	int getCLRHeaderSize()
	{
		PIMAGE_DATA_DIRECTORY pDataDir;
		int err = getDataDirectory(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR, std::ref(pDataDir));
		if (err)
			return 0;

		return pDataDir->Size;
	}

	bool hasCLRResources()
	{
		PIMAGE_COR20_HEADER pCLRHeader;
		int err = getCLRHeader(std::ref(pCLRHeader));
		if (err)
			false;

		return (pCLRHeader->Resources.VirtualAddress != 0 && pCLRHeader->Resources.Size > 0);
	}

	int getMetaDataHeader1(const PIMAGE_COR20_HEADER& pCLRData, PMETA_DATA_SECTION_HEADER1& ptr)
	{
		PMETA_DATA_SECTION_HEADER1 pMetaDataSectionHeader1 = PMETA_DATA_SECTION_HEADER1(getAddr(pCLRData->MetaData.VirtualAddress));
		if (!isMemoryReadable(pMetaDataSectionHeader1, sizeof(*pMetaDataSectionHeader1)))
			return errInvalidMemory;

		ptr = pMetaDataSectionHeader1;

		return noErr;
	}

	int getMetaDataHeader2(const PMETA_DATA_SECTION_HEADER1& pMetaData1, PMETA_DATA_SECTION_HEADER2& ptr)
	{
		PMETA_DATA_SECTION_HEADER2 pMetaDataSectionHeader2 = PMETA_DATA_SECTION_HEADER2(UINT_PTR(&pMetaData1->Name) + pMetaData1->Length);
		if (!isMemoryReadable(pMetaDataSectionHeader2, sizeof(*pMetaDataSectionHeader2)))
			return errInvalidMemory;

		ptr = pMetaDataSectionHeader2;

		return noErr;
	}

	int getFirstMetaStreamHeader(const PMETA_DATA_SECTION_HEADER2& pMetaData2, PMETA_STREAM_HEADER& ptr)
	{
		PMETA_STREAM_HEADER pMetaStreamHeader = PMETA_STREAM_HEADER(UINT_PTR(pMetaData2) + sizeof(META_DATA_SECTION_HEADER2));
		if (!isMemoryReadable(pMetaStreamHeader, sizeof(*pMetaStreamHeader)))
			return errInvalidMemory;

		ptr = pMetaStreamHeader;

		return noErr;
	}

	int getNextMetaStreamHeader(const PMETA_STREAM_HEADER& pMetaStreamHeader, PMETA_STREAM_HEADER& ptr)
	{
		static const int MAX_STREAM_NAME = 32;
		static const int NAME_STRIDE = 4;
		PMETA_STREAM_HEADER pNextMetaStreamHeader = pMetaStreamHeader;

		for (int i = 0; i < MAX_STREAM_NAME; i += NAME_STRIDE)
		{
			if (!isMemoryReadable(&pNextMetaStreamHeader->Name[i], NAME_STRIDE))
				return errInvalidMemory;

			bool bFoundNextMetaStreamHeader = false;
			for (int j = 0; j < NAME_STRIDE; ++j)
			{
				if (pNextMetaStreamHeader->Name[i + j] == '\0')
				{
					pNextMetaStreamHeader = PMETA_STREAM_HEADER(&pNextMetaStreamHeader->Name[i + NAME_STRIDE]);

					bFoundNextMetaStreamHeader = true;
					break;
				}
			}

			if (bFoundNextMetaStreamHeader)
				break;
		}

		if (!isMemoryReadable(pNextMetaStreamHeader, sizeof(*pNextMetaStreamHeader)))
			return errInvalidMemory;

		ptr = pNextMetaStreamHeader;

		return noErr;
	}

	int getMetaStreamName(const PMETA_STREAM_HEADER pMetaStreamHeader, wstring& name)
	{
		static const int MAX_STREAM_NAME = 32;
		static const int NAME_STRIDE = 4;

		bool bFoundEndOfStreamName = false;
		for (int i = 0; i < MAX_STREAM_NAME; i += NAME_STRIDE)
		{
			if (!isMemoryReadable(&pMetaStreamHeader->Name[i], NAME_STRIDE))
				return errInvalidMemory;

			for (int j = 0; j < NAME_STRIDE; ++j)
			{
				if (pMetaStreamHeader->Name[i + j] == '\0')
				{
					bFoundEndOfStreamName = true;
					break;
				}

				if (bFoundEndOfStreamName)
					break;
			}
		}

		if (!bFoundEndOfStreamName)
			return errInvalidMemory;

		name = MultiByte_toString(string(pMetaStreamHeader->Name).c_str());

		return noErr;
	}

	int getMetaStreamGUIDs(const PIMAGE_COR20_HEADER& pCLRData, const PMETA_STREAM_HEADER& pMetaStreamHeader, LPBYTE& ptr)
	{
		LPBYTE pGUIDs = LPBYTE(getAddr(pCLRData->MetaData.VirtualAddress + pMetaStreamHeader->Offset));
		if (!isMemoryReadable(pGUIDs, pMetaStreamHeader->Size))
			return errInvalidMemory;

		ptr = pGUIDs;

		return noErr;
	}

	int getMetaCompositeHeader(const PIMAGE_COR20_HEADER& pCLRData, const PMETA_STREAM_HEADER& pMetaStreamHeader, PMETA_COMPOSITE_HEADER& ptr)
	{
		PMETA_COMPOSITE_HEADER pMetaCompositeHeader = PMETA_COMPOSITE_HEADER(getAddr(pCLRData->MetaData.VirtualAddress + pMetaStreamHeader->Offset));
		if (!isMemoryReadable(pMetaCompositeHeader, sizeof(*pMetaCompositeHeader)))
			return errInvalidMemory;

		ptr = pMetaCompositeHeader;

		return noErr;
	}

	int getStrongNameSignatureHash(const PIMAGE_COR20_HEADER& pCLRData, PBYTE& ptr)
	{
		PBYTE pStrongNameHash = PBYTE(getAddr(pCLRData->StrongNameSignature.VirtualAddress));

		if (!isMemoryReadable(pStrongNameHash, pCLRData->StrongNameSignature.Size))
			return errInvalidMemory;

		ptr = pStrongNameHash;

		return noErr;
	}

	int getNoOfVTableFixupsDirectories(const PIMAGE_COR20_HEADER& pCLRData)
	{
		return (pCLRData->VTableFixups.Size / sizeof(IMAGE_COR_VTABLEFIXUP));
	}

	int getVTableFixupsDirectory(const PIMAGE_COR20_HEADER& pCLRData, int i, PIMAGE_COR_VTABLEFIXUP& ptr)
	{
		PIMAGE_COR_VTABLEFIXUP pVTableFixupDir = PIMAGE_COR_VTABLEFIXUP(getAddr(pCLRData->VTableFixups.VirtualAddress)) + i;
		if (!isMemoryReadable(pVTableFixupDir, sizeof(*pVTableFixupDir)))
			return errInvalidMemory;

		ptr = pVTableFixupDir;

		return noErr;
	}
#pragma endregion

#pragma region ///////////////////////////////// Utility functions /////////////////////////////////
	bool hasRichSignature(DWORD& nSignDwords)
	{
		if (!isMemoryReadable(getAddr(RICH_DATA_START_OFFSET, true), sizeof(DWORD) * 100))
			return false;

		if (m_pDOSHeader->e_lfanew <= sizeof(IMAGE_DOS_HEADER))
			return false;

		nSignDwords = 0;

		for (int i = 0; i < 100; i++)
		{
			LPDWORD pdw = LPDWORD(getAddr(RICH_DATA_START_OFFSET + i * sizeof(DWORD), true));
			if (!isMemoryReadable(pdw, sizeof(*pdw)))
				return false;

			switch (*pdw)
			{
				case 0: return false;
				case RICH_SIGNATURE:
				{
					nSignDwords = i;

					return true;
				}
			}
		}

		return (nSignDwords != 0);
	}

	int getRichVCToolsData(const DWORD nSignDwords, vector<RichSigVCCompilerInfo>& out)
	{
		vector<RichSigVCCompilerInfo> Data;

		LPDWORD pMask = LPDWORD(getAddr(DWORD(RICH_DATA_START_OFFSET + ((nSignDwords + 1) * sizeof(DWORD))), true));
		if (!isMemoryReadable(pMask, sizeof(*pMask)))
			return errInvalidMemory;

		for (size_t i = sizeof(DWORD); i < nSignDwords; i += 2)
		{
			LPDWORD pdw_withoutmask = LPDWORD(getAddr(DWORD(RICH_DATA_START_OFFSET + (i * sizeof(DWORD))), true));
			if (!isMemoryReadable(pdw_withoutmask, sizeof(*pdw_withoutmask)))
				return errInvalidMemory;

			LPDWORD pdw2_withoutmask = LPDWORD(getAddr(DWORD(RICH_DATA_START_OFFSET + DWORD((i + 1) * sizeof(DWORD))), true));
			if (!isMemoryReadable(pdw2_withoutmask, sizeof(*pdw2_withoutmask)))
				return errInvalidMemory;

			DWORD dw = *pdw_withoutmask ^ *pMask;
			DWORD dw2 = *pdw2_withoutmask ^ *pMask;

			Data.push_back({ (dw >> 16), (dw & 0xFFFFUL), dw2 });
		}

		out = std::move(Data);

		return noErr;
	}

	int disassembleMSDOSstub(wstring& out)
	{
		const uint8_t DISASSEMBLY_MODE = 16;			// Use 16-bit disassembly

		ud_t Disassembler;
		ud_init(&Disassembler);							// Initialize disassembler
		ud_set_mode(&Disassembler, DISASSEMBLY_MODE);
		wstring Disassembly;

		if (!isMemoryReadable(getAddr(m_pDOSHeader->e_ip, true), sizeof(DWORD)))
			return errInvalidMemory;

		// Set EIP
		ud_set_pc(&Disassembler, UINT_PTR(getAddr(m_pDOSHeader->e_ip, true)));
		ud_set_syntax(&Disassembler, UD_SYN_INTEL);		// Use Microsoft-Intel opcode mnemonics style

		if (m_pDOSHeader->e_lfanew > sizeof(IMAGE_DOS_HEADER))
		{
			ud_set_input_buffer(&Disassembler,
								reinterpret_cast<uint8_t *> (getAddr(sizeof(IMAGE_DOS_HEADER), true)),
								m_pDOSHeader->e_lfanew - sizeof(IMAGE_DOS_HEADER));

			while (ud_disassemble(&Disassembler))		// Disassembly each block and produce in text form
			{
				// Format for each line of disassembly will be
				//	<Hexadecimal assembly> <Tab space> <Instruction Mnemonic>
				// eg: 00 01 02		mov eax, 1
				const char *pBuffer = ud_insn_hex(&Disassembler);	// Get hexadecimal notation of opcode
				Disassembly += (pBuffer ? L"0x" + MultiByte_toString(pBuffer) : L"<Invalid>") + L'\t';

				pBuffer = ud_insn_asm(&Disassembler);		// Get mnemonic
				Disassembly += (pBuffer ? MultiByte_toString(pBuffer) : L"<Invalid>");
				Disassembly += L'\n';
			}
		}
		else
			Disassembly = L"File pointer to MSDOS stub is pointing into 'IMAGE_DOS_HEADER'! Skipping MSDOS disassembly step.";

		out = std::move(Disassembly);

		return noErr;
	}

	unsigned short getNoOfSections()
	{
		assert(m_pNTHeaders);
		return m_pNTHeaders->FileHeader.NumberOfSections;
	}

	bool verifyPEChecksum(DWORD& calculatedsum)
	{
		DWORD ChecksumInHeader, Calculatedchecksum;

		if (CheckSumMappedFile(m_pMapViewBaseAddress.get(),
							   getFileSize().LowPart,
							   &ChecksumInHeader,
							   &Calculatedchecksum))
		{
			calculatedsum = Calculatedchecksum;

			return (ChecksumInHeader == Calculatedchecksum);	// Return true if checksum
			                                                    // in PE header and calculated
															    // checksum matches
		}

		return false;
	}

	wstring getContainingSectionName(DWORD rva)
	{
		for (WORD i = 0; i < getNoOfSections(); ++i)
		{
			PIMAGE_SECTION_HEADER pSectionHeader = &getFirstSectionHeader()[i];
			if (!isMemoryReadable(pSectionHeader, sizeof(IMAGE_SECTION_HEADER)))
				break;

			if (rva >= pSectionHeader->VirtualAddress &&
				rva < pSectionHeader->VirtualAddress + (pSectionHeader->Misc.VirtualSize == 0 ? pSectionHeader->SizeOfRawData : pSectionHeader->Misc.VirtualSize))
				return ProperSectionName(pSectionHeader->Name);
		}

		return L"<Invalid>";
	}

	void computeHashes(wstring &SHA1, wstring &MD5)
	{
		static auto CryptProviderReleaser = [](HCRYPTPROV handle) { CryptReleaseContext(handle, 0); };
		static auto CryptHashReleaser = [](HCRYPTHASH handle) { CryptDestroyHash(handle); };

		// Initialize Windows Crypt API
		unique_handle<HCRYPTPROV, decltype(CryptProviderReleaser)> hCryptProv(NULL, CryptProviderReleaser);
		unique_handle<HCRYPTHASH, decltype(CryptHashReleaser)> hHash(NULL, CryptHashReleaser);

		if (CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0))
		{
			DWORD dwHashLen = 0;

			// Create SHA1 hash object
			if (CryptCreateHash(hCryptProv.get(), CALG_SHA1, 0, 0, &hHash))
				if (CryptHashData(hHash.get(), static_cast<const BYTE *>(getAddr(0, true)), getFileSize().LowPart, 0))
					if (CryptGetHashParam(hHash.get(), HP_HASHVAL, NULL, &dwHashLen, 0))
					{
						unique_ptr<BYTE> pHash(new BYTE[dwHashLen]);
						unique_ptr<WCHAR> szHash(new WCHAR[dwHashLen * 2 + 1]);

						CryptGetHashParam(hHash.get(), HP_HASHVAL, pHash.get(), &dwHashLen, 0);

						for (DWORD i = 0; i < dwHashLen; ++i)
							swprintf_s(&szHash.get()[i * 2], 3, L"%02x", pHash.get()[i]);

						// NOTE: 'szHash' is already null terminated
						SHA1 = szHash.get();
					}

			// Manually release handle to reuse 'hHash'
			hHash.~unique_handle();

			// Create MD5 hash object
			if (CryptCreateHash(hCryptProv.get(), CALG_MD5, 0, 0, &hHash))
				if (CryptHashData(hHash.get(), static_cast<const BYTE *>(getAddr(0, true)), getFileSize().LowPart, 0))
					if (CryptGetHashParam(hHash.get(), HP_HASHVAL, nullptr, &dwHashLen, 0))
					{
						unique_ptr<BYTE> pHash(new BYTE[dwHashLen]);
						unique_ptr<WCHAR> szHash(new WCHAR[dwHashLen * 2 + 1]);

						CryptGetHashParam(hHash.get(), HP_HASHVAL, pHash.get(), &dwHashLen, 0);

						for (DWORD i = 0; i < dwHashLen; ++i)
							swprintf_s(&szHash.get()[i * 2], 3, L"%02x", pHash.get()[i]);

						MD5 = szHash.get();
					}
		}
	}

	static bool isMicrosoftStyleMangledName(string input)
	{
		return input.at(0) == '?';
	}

	static string unmangleCPPNames(string input, SymbolPart part)
	{
		const size_t sizeUnmangledBuffer = 1024;
		CHAR szUnmangled[sizeUnmangledBuffer];

		// First try to unmangle using Microsoft's function
		if (isMicrosoftStyleMangledName(input))
		{
			if (UnDecorateSymbolName(input.c_str(), szUnmangled, DWORD(sizeUnmangledBuffer),
				  				     (part == SymbolPart::Name ? UNDNAME_NAME_ONLY : UNDNAME_COMPLETE)) != 0)
				return szUnmangled;
		}
		// If that failed, try to unmangle using GCC's function
		size_t _sizeUnmangledBuffer = sizeUnmangledBuffer;
		if (GCC_Unmangle(input.c_str(), szUnmangled, &_sizeUnmangledBuffer))
			return szUnmangled;

		// All failed, then return the original name
		return input;
	}

	ULONGLONG RVAToVA(DWORD rva)
	{
		switch (m_PEType)
		{
			case PEType::PE32: return (m_pNTHeaders32->OptionalHeader.ImageBase + rva);
			case PEType::PE64: return (m_pNTHeaders64->OptionalHeader.ImageBase + rva);
		}

		return 0ULL;
	}

	DWORD RVAToFileOffset(DWORD rva)
	{
		PIMAGE_SECTION_HEADER pSectionHeader;
		int err = getSectionHeader(0, std::ref(pSectionHeader));
		if (err)
			return 0;

		if (rva < pSectionHeader->VirtualAddress)
			return rva;

		for (WORD i = 0; i < getNoOfSections(); ++i)
		{
			err = getSectionHeader(i, std::ref(pSectionHeader));
			if (err)
				break;

			DWORD SectionStart = pSectionHeader->VirtualAddress;
			DWORD SectionEnd = SectionStart + (pSectionHeader->Misc.VirtualSize == 0 ? pSectionHeader->SizeOfRawData : pSectionHeader->Misc.VirtualSize);

			if (rva >= SectionStart && rva < SectionEnd)
			{
				rva -= pSectionHeader->VirtualAddress;
				rva += pSectionHeader->PointerToRawData;

				return rva;
			}
		}

		return 0;
	}

	DWORD VAToRVA(ULONGLONG va)
	{
		switch (m_PEType)
		{
			case PEType::PE32:
				return DWORD(DWORD(va) < m_pNTHeaders32->OptionalHeader.ImageBase ?
							 0 : va - m_pNTHeaders32->OptionalHeader.ImageBase);

			case PEType::PE64:
				return DWORD(va < m_pNTHeaders64->OptionalHeader.ImageBase ?
							 0 : va - m_pNTHeaders64->OptionalHeader.ImageBase);
		}

		return 0;
	}

	DWORD VAToFileOffset(ULONGLONG va)
	{
		switch (m_PEType)
		{
			case PEType::PE32:
				return DWORD(DWORD(va) < m_pNTHeaders32->OptionalHeader.ImageBase ?
							 0 : RVAToFileOffset(DWORD(va - m_pNTHeaders32->OptionalHeader.ImageBase)));

			case PEType::PE64:
				return DWORD(va < m_pNTHeaders64->OptionalHeader.ImageBase ?
							 0 : RVAToFileOffset(DWORD(va - m_pNTHeaders64->OptionalHeader.ImageBase)));
		}

		return 0;
	}

	DWORD FileOffsetToRVA(DWORD fileoffset)
	{
		PIMAGE_SECTION_HEADER pSectionHeader;
		int err = getSectionHeader(0, std::ref(pSectionHeader));
		if (err)
			return 0;

		if (fileoffset < pSectionHeader->PointerToRawData)
			return fileoffset;

		for (WORD i = 0; i < getNoOfSections(); ++i)
		{
			err = getSectionHeader(i, std::ref(pSectionHeader));
			if (err)
				break;

			DWORD SectionStart = pSectionHeader->PointerToRawData;
			DWORD SectionEnd = SectionStart + pSectionHeader->SizeOfRawData;

			if (fileoffset >= SectionStart && fileoffset < SectionEnd)
			{
				fileoffset -= pSectionHeader->PointerToRawData;
				fileoffset += pSectionHeader->VirtualAddress;

				return fileoffset;
			}
		}

		return 0;
	}

	ULONGLONG FileOffsetToVA(DWORD fileoffset)
	{
		DWORD RVA = FileOffsetToRVA(fileoffset);
		if (RVA == 0 && fileoffset != 0)
			return 0;

		switch (m_PEType)
		{
			case PEType::PE32:
				return (m_pNTHeaders32->OptionalHeader.ImageBase + RVA);

			case PEType::PE64:
				return (m_pNTHeaders64->OptionalHeader.ImageBase + RVA);
		}

		return 0;
	}

	int displayHexData(ULONGLONG address, DWORD fileOffset, DWORD size, HexViewType typeView, wstring& out)
	{
		if (fileOffset >= getFileSize().LowPart)
			return errInvalidMemory;

		const DWORD cData = min(getFileSize().LowPart - fileOffset, size);

		union
		{
			LPBYTE	pDataAsByte;
			LPWORD pDataAsWord;
			LPDWORD pDataAsDWord;
			PULONGLONG pDataAsQWord;
		};
		pDataAsByte = LPBYTE(getAddr(fileOffset, true));

		switch (typeView)
		{
			case HexViewType::Byte:
			{
				static const unsigned int DATA_PER_ROW = 8;

				for (DWORD i = 0; i < cData; i += DATA_PER_ROW)
				{
					out += (address > MAXDWORD ? DWORD_toString(DWORD(address), Hexadecimal) : QWORD_toString(address, Hexadecimal)) + L'\t';

					for (int j = 0; j < DATA_PER_ROW; ++j)
					{
						if (i + j >= cData)
							break;

						out += BYTE_toString(pDataAsByte[j], Hexadecimal, true, false) + L' ';
					}

					out += L'\t';

					for (int j = 0; j < DATA_PER_ROW; ++j)
					{
						if (i + j >= cData)
							break;

						out += isprint(pDataAsByte[j]) ? pDataAsByte[j] : L'.';
						out += L' ';
					}

					pDataAsByte += DATA_PER_ROW;
					address += DATA_PER_ROW;
					out += L'\n';
				}
			}
			break;

			case HexViewType::Word:
			{
				static const unsigned int DATA_PER_ROW = 4;

				for (DWORD i = 0; i < cData; i += DATA_PER_ROW)
				{
					out += (address > MAXDWORD ? DWORD_toString(DWORD(address), Hexadecimal) : QWORD_toString(address, Hexadecimal)) + L'\t';

					for (int j = 0; j < DATA_PER_ROW; ++j)
					{
						if (i + j >= cData)
							break;

						out += WORD_toString(pDataAsWord[j], Hexadecimal, true, false) + L' ';
					}

					out += L'\t';

					for (int j = 0; j < DATA_PER_ROW * sizeof(WORD); ++j)
					{
						if (i + j >= cData)
							break;

						out += isprint(pDataAsByte[j]) ? pDataAsByte[j] : L'.';
						out += L' ';
					}

					pDataAsWord += DATA_PER_ROW;
					address += DATA_PER_ROW * sizeof(WORD);
					out += L'\n';
				}
			}
			break;

			case HexViewType::DWord:
			{
				static const unsigned int BYTES_PER_VIEW = 2;

				for (DWORD i = 0; i < cData; i += BYTES_PER_VIEW)
				{
					out += (address > MAXDWORD ? DWORD_toString(DWORD(address), Hexadecimal) : QWORD_toString(address, Hexadecimal)) + L'\t';

					for (int j = 0; j < BYTES_PER_VIEW; ++j)
					{
						if (i + j >= cData)
							break;

						out += DWORD_toString(pDataAsDWord[j], Hexadecimal, true, false) + L' ';
					}

					out += L'\t';

					for (int j = 0; j < BYTES_PER_VIEW * sizeof(DWORD); ++j)
					{
						if (i + j >= cData)
							break;

						out += isprint(pDataAsByte[j]) ? pDataAsByte[j] : L'.';
						out += L' ';
					}

					pDataAsDWord += BYTES_PER_VIEW;
					address += BYTES_PER_VIEW * sizeof(DWORD);
					out += L'\n';
				}
			}
			break;

			case HexViewType::QWord:
			{
				static const unsigned int BYTES_PER_VIEW = 1;

				for (DWORD i = 0; i < cData; i += BYTES_PER_VIEW)
				{
					out += (address > MAXDWORD ? DWORD_toString(DWORD(address), Hexadecimal) : QWORD_toString(address, Hexadecimal)) + L'\t';

					for (int j = 0; j < BYTES_PER_VIEW; ++j)
					{
						if (i + j >= cData)
							break;

						out += QWORD_toString(pDataAsQWord[j], Hexadecimal, true, false) + L' ';
					}

					out += L'\t';

					for (int j = 0; j < BYTES_PER_VIEW * sizeof(ULONGLONG); ++j)
					{
						if (i + j >= cData)
							break;

						out += isprint(pDataAsByte[j]) ? pDataAsByte[j] : L'.';
						out += L' ';
					}

					pDataAsQWord += BYTES_PER_VIEW;
					address += BYTES_PER_VIEW * sizeof(ULONGLONG);
					out += L'\n';
				}
			}
			break;
		}

		return noErr;
	}

	int disassembleAt(ULONGLONG address, DWORD fileOffset, wstring& out)
	{
		if (fileOffset >= getFileSize().LowPart)
			return errInvalidMemory;

		uint8_t modeDisassembly;

		// Set disassembler mode
		switch (m_PEType)
		{
			case PEType::PE32:
				modeDisassembly = 32;
				break;

			case PEType::PE64:
				modeDisassembly = 64;
				break;

			default:
				assert(false);
		}

		ud_t Disassembler;
		ud_init(&Disassembler);						// Initialize disassembler
		ud_set_mode(&Disassembler, modeDisassembly); // Whether 32-bit/64-bit disassembly to use
		ud_set_syntax(&Disassembler, UD_SYN_INTEL);	// Use Microsoft-Intel opcode mnemonics

		const size_t cData = min(getFileSize().LowPart - fileOffset, 512);

		ud_set_input_buffer(&Disassembler, static_cast<uint8_t *> (getAddr(fileOffset)), cData);

		while (ud_disassemble(&Disassembler))		// Disassembly each block and produce mnemonic in text form
		{
			// First show address
			out += (address > MAXDWORD ? DWORD_toString(DWORD(address), Hexadecimal) : QWORD_toString(address, Hexadecimal)) + L'\t';

			// Then mnemonic
			const char *szMnemonic = ud_insn_asm(&Disassembler);		// Get mnemonic
			out += (szMnemonic ? MultiByte_toString(szMnemonic) : L"<Invalid>");
			out += L'\n';

			// Maintain address
			address += ud_insn_len(&Disassembler);
		}

		return noErr;
	}
#pragma endregion
};