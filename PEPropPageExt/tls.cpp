#include "tls.h"
#include "PEReadWrite.h"
#include <list>
#include <algorithm>
#include <Windows.h>


// TLS specific part
// Manually add a TLS initialize call back
#if defined (_M_IX86)	// Intel x86 processors
	#pragma comment(linker, "/INCLUDE:__tls_used")
#else					// AMD/Intel x64 processors
	#pragma comment(linker, "/INCLUDE:_tls_used")
#endif

#pragma section(".CRT$XLB", read)	// Open the CRT section with TLS data

// Insert our TLS call back handler function pointer in the section
extern "C" __declspec(allocate(".CRT$XLB"))
  const PIMAGE_TLS_CALLBACK _xl_b  = tls_callback;


// All thread shared global variables and objects
extern THREAD_ISOLATED_STORAGE PEReadWrite *pPEReaderWriter;
static list<DWORD> ThreadIds;


bool ThreadTLSConstructor()
{
	// This function is used to correctly initialize TLS objects
	return pPEReaderWriter = new PEReadWrite(), pPEReaderWriter != NULL;
}

void ThreadTLSDestructor()
{
	SAFE_RELEASE(pPEReaderWriter);
}

void RegisterThreadId()
{
	ThreadIds.push_back(GetCurrentThreadId());
}

void UnregisterThreadId()
{
	ThreadIds.remove(GetCurrentThreadId());
}

bool IsMyThread(DWORD threadID)
{
	return find(ThreadIds.begin(), ThreadIds.end(), 
				GetCurrentThreadId()) != ThreadIds.end();
}

VOID NTAPI tls_callback(PVOID DllHandle, DWORD Reason, PVOID Reserved)
{
	switch (Reason)
	{
	case DLL_THREAD_DETACH:
		if (!IsMyThread(GetCurrentThreadId()))
			break;

	case DLL_PROCESS_DETACH:
		UnregisterThreadId();
		ThreadTLSDestructor();
	}
}