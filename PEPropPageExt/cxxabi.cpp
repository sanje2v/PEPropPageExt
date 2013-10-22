#include "cxxabi.h"
#include "stdafx.h"


char* __cxa_demangle(const char* __mangled_name, char* __output_buffer,
		 size_t* __length, int* __status)
{
	static HMODULE hLibrary = NULL;
	static p__cxa_demangle pFunc = NULL;

	if (!hLibrary)
	{
		hLibrary = LoadLibrary(TEXT("libstdc++-6.dll"));

		if (!hLibrary)
			return NULL;

		pFunc = (p__cxa_demangle) GetProcAddress(hLibrary, "__cxa_demangle");

		if (!pFunc)
			return NULL;
	}

	return (!pFunc ? NULL : pFunc(__mangled_name, __output_buffer, __length, __status));
}
