#include "cxxabi.h"


char* GCC_Unmangle(const char* mangledname, char* output_buffer, size_t* length)
{
	static bool bLastFailed = false;	// If module couldn't be loaded last time,
										//	there's no use checking everytime

	__try
	{
		// If that failed, try to unmangle using GCC's function
		if (!bLastFailed)
			if (__cxa_demangle(mangledname, output_buffer, length, NULL))
				return output_buffer;
	}
	__except (GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND) ||
			  GetExceptionCode() == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND))
	{
		bLastFailed = true;
	}

	return NULL;
}