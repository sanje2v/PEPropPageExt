#pragma once
#include "stdafx.h"

namespace ManagedFuncs
{
	enum class DataType : int
	{
		Unknown = 0,
		IconHandle,
		CursorHandle,
		BitmapHandle,
		String,		// String and XML string
		Binary,
	};

	extern "C"
	{
		void * __cdecl createResourceReader(const wchar_t *szAssemblyName);
		void __cdecl destroyResourceReader(void *hResourceReader);
		int __cdecl getNoOfResourceStreamNames(void *hResourceReader);
		bool __cdecl getResourceStreamName(void *hResourceReader, int idxStream, wchar_t *szStreamName, int cStreamName);
		bool __cdecl selectResourceStream(void *hResourceReader, int idxStream);
		bool __cdecl getNextResourceSelectedStreamKeyAndValue(void *hResourceReader, wchar_t* szKey, int cKey, DataType *ptypeData, void **pszData, int *pcData);
	}

	class ManagedResourceReader
	{
	private:
		void *m_hResourceReader;

	public:
		ManagedResourceReader();
		~ManagedResourceReader();

		bool create(const wchar_t *szAssemblyName, bool& failedToLoadDLL);
		int getNoOfStreamNames();
		bool getStreamName(int idxStream, wchar_t *szStreamName, int cStreamName);
		bool selectStream(int idxStream);
		bool getNextSelectedStreamKeyAndValue(wchar_t* szKey, int cKey, DataType *ptypeData = NULL, void **pszData = NULL, int *pcData = NULL);
	};
}