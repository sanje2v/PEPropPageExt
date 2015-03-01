#include "ManagedFuncs.h"
#include "MiscFuncs.h"
#include <delayimp.h>

using namespace ManagedFuncs;

// Delay load hook function
static const WCHAR PROJECT_OUTPUT_DLL_NAME[] = L"PEPropPageExt";
static const WCHAR DEPENDENCY_DLL_MODULENAME[] = L"ManagedFuncs";
static const CHAR DEPENDENCY_DLL_FILENAME[] = "ManagedFuncs.dll";

static UINT_PTR cDependencyRef = 0;
static PfnDliHook pfuncPrevDelayLoad = NULL;
FARPROC WINAPI funcCustomDelayLoad(unsigned dliNotify, PDelayLoadInfo pdli);


ManagedResourceReader::ManagedResourceReader()
	: m_hResourceReader(nullptr)
{
	if (GetModuleHandle(DEPENDENCY_DLL_MODULENAME) == NULL)
	{
		// NOTE: This section is needed because functionality to add new DLL
		//  search path is only available from Windows 7. Without this section,
		//  Windows DLL loader would look for 'ManagedFuncs.dll' in the 'Explorer.exe'
		//  search path. The dependency DLL is actually in the same path as the
		//  main plugin DLL.
		pfuncPrevDelayLoad = __pfnDliNotifyHook2;
		__pfnDliNotifyHook2 = funcCustomDelayLoad;
	}

	InterlockedIncrement(&cDependencyRef);
}

ManagedResourceReader::~ManagedResourceReader()
{
	if (m_hResourceReader)
		destroyResourceReader(m_hResourceReader), m_hResourceReader = NULL;

	// NOTE: Even when we have called delay module unloader function which
	//  in turn calls 'FreeLibrary()', CLR has held one more library reference
	//  preventing 'ManagedFuncs.dll' from being unloaded. Documentations of .NET,
	//  tell us that it cannot guarantee nor provide any mechanism for manually
	//  unloading managed DLLs. Hence, CLR will unload the module whenever it
	//  feels like it or when 'Explorer.exe', the parent process, terminates.
	if (InterlockedDecrement(&cDependencyRef) == 0)
		__FUnloadDelayLoadedDLL2(DEPENDENCY_DLL_FILENAME);
}

bool ManagedResourceReader::create(const wchar_t *szAssemblyName, bool& failedToLoadDLL)
{
	__try
	{
		m_hResourceReader = createResourceReader(szAssemblyName);
	}
	__except (GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND) ||
			  GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND))
	{
		failedToLoadDLL = true;
		return false;
	}
	
	failedToLoadDLL = false;
	return (m_hResourceReader != NULL);
}

int ManagedResourceReader::getNoOfStreamNames()
{
	int NoOfStreamNames;

	__try
	{
		NoOfStreamNames = getNoOfResourceStreamNames(m_hResourceReader);
	}
	__except (GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND) ||
			  GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND))
	{
		return 0;
	}

	return NoOfStreamNames;
}

bool ManagedResourceReader::getStreamName(int idxStream, wchar_t *szStreamName, int cStreamName)
{
	bool Success;

	__try
	{
		Success = getResourceStreamName(m_hResourceReader, idxStream, szStreamName, cStreamName);
	}
	__except (GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND) ||
			  GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND))
	{
		return false;
	}

	return Success;
}

bool ManagedResourceReader::selectStream(int idxStream)
{
	bool Success;

	__try
	{
		Success = selectResourceStream(m_hResourceReader, idxStream);
	}
	__except (GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND) ||
			  GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND))
	{
		return false;
	}

	return Success;
}

bool ManagedResourceReader::getNextSelectedStreamKeyAndValue(wchar_t* szKey, int cKey, DataType *ptypeData, void **pData, int *pcData)
{
	bool Success;

	__try
	{
		Success = getNextResourceSelectedStreamKeyAndValue(m_hResourceReader, szKey, cKey, ptypeData, pData, pcData);
	}
	__except (GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND) ||
			  GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND))
	{
		return false;
	}

	return Success;
}

FARPROC WINAPI funcCustomDelayLoad(unsigned dliNotify, PDelayLoadInfo pdli)
{
	if (dliNotify == dliNotePreLoadLibrary)
	{
		if (_stricmp(pdli->szDll, DEPENDENCY_DLL_FILENAME) == 0)
		{
			// Get handle of the main plugin dll
			HMODULE hMainModule = GetModuleHandle(PROJECT_OUTPUT_DLL_NAME);
			assert(hMainModule);
			if (!hMainModule)
				return 0;

			// Get path of the main plugin dll
			WCHAR szMainModulePath[MAX_PATH];
			DWORD cModulePath = GetModuleFileName(hMainModule, szMainModulePath, ARRAYSIZE(szMainModulePath));
			assert(cModulePath > 0);
			if (cModulePath <= 0)
				return 0;

			// Remove filename section from the full module filepath
			PathRemoveFileSpec(szMainModulePath);
			// Append dependency module file name
			wstring DependencyDLLFullPath = wstring(szMainModulePath) + L'\\' + MultiByte_toString(DEPENDENCY_DLL_FILENAME);

			// Before exiting this function procedure, restore original hook function
			__pfnDliNotifyHook2 = pfuncPrevDelayLoad;
			pfuncPrevDelayLoad = NULL;

			return FARPROC(LoadLibraryEx(DependencyDLLFullPath.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH));
		}
	}

	return 0;
}