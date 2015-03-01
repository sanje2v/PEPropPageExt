// ManagedFuncs.h
#pragma once

using namespace System;
using namespace System::IO;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Xml;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;
using namespace System::Resources;

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

	public ref class Resources : public IDisposable
	{
	private:
		Assembly ^m_assembly;
		array<String ^> ^m_streamNames;
		
		// Direct resource
		bool m_isDirectResource;
		Object ^m_dataDirectResource;

		// Indirect resource
		ResourceReader ^m_resourceReader;
		IDictionaryEnumerator ^m_dataEnumerator;

	public:
		generic<typename T>
		ref struct Pair
		{
			T Item1;
			Object ^Item2;

			Pair(T item1, Object ^item2)
				: Item1(item1), Item2(item2) {}
		};

		Resources(String ^assembly);
		~Resources();
		array<String ^> ^getStreamNames();
		void selectStream(int idxStream);
		Pair<String ^> ^getSelectedStreamNextKeyAndData();
	};
}
