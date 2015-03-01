// This is the main DLL file.

#include "stdafx.h"
#include "winapis.h"
#include "ManagedFuncs.h"
#include <cassert>
#include <vcclr.h>
#include <memory>
#include <string>


namespace ManagedFuncs
{
	Resources::Resources(String^ assemblyPath)
	{
		// NOTE: The following will fail if a 32-bit assembly tries to load 64-bit assembly.
		//  This is a normal expected behaviour for now.
		m_assembly = Assembly::ReflectionOnlyLoadFrom(assemblyPath); // Loads assembly without execution permission
		m_streamNames = m_assembly->GetManifestResourceNames();
		m_isDirectResource = false;
		m_resourceReader = nullptr;
		m_dataEnumerator = nullptr;
	}

	Resources::~Resources()
	{
		if (m_resourceReader)
		{
			m_resourceReader->Close();
		}
	}

	array<String ^>^ Resources::getStreamNames()
	{
		return m_streamNames;
	}

	void Resources::selectStream(int idxStream)
	{
		String ^streamNameLowercase = m_streamNames[idxStream]->ToLower();
		Stream ^stream = m_assembly->GetManifestResourceStream(m_streamNames[idxStream]);

		if (streamNameLowercase->EndsWith(".resources"))
		{
			// There are further embedded resource streams under this
			m_isDirectResource = false;
			m_resourceReader = gcnew ResourceReader(stream);
			m_dataEnumerator = m_resourceReader->GetEnumerator();
		}
		else
		{
			// This is directly a resource data
			m_isDirectResource = true;
			m_resourceReader = nullptr;
			m_dataEnumerator = nullptr;

			// Identify resource type and read it appropriately
			try
			{
				if (streamNameLowercase->EndsWith(".ico") ||
					streamNameLowercase == "$this.icon")
				{
					// It's an icon
					Icon ^icon = gcnew Icon(stream);
					m_dataDirectResource = icon;
				}
				else if (streamNameLowercase->EndsWith(".cur"))
				{
					// It's an cursor
					Cursor ^cursor = gcnew Cursor(stream);
					m_dataDirectResource = cursor;
				}
				else if (streamNameLowercase->EndsWith(".bmp") ||
						 streamNameLowercase->EndsWith(".jpg") ||
						 streamNameLowercase->EndsWith(".png") ||
						 streamNameLowercase->EndsWith(".jpeg") ||
						 streamNameLowercase->EndsWith(".gif") ||
						 streamNameLowercase->EndsWith(".tiff"))
				{
					// It's a picture
					Bitmap ^bitmap = gcnew Bitmap(stream);
					m_dataDirectResource = bitmap;
				}
				else if (streamNameLowercase->EndsWith(".xml") ||
						 streamNameLowercase->EndsWith(".xaml"))
				{
					// It's a XML text
					XmlDocument ^xmlDoc = gcnew XmlDocument();
					xmlDoc->Load(stream);

					auto stringWriter = gcnew StringWriter();

					auto xmlTextWriter = XmlWriter::Create(stringWriter);
					xmlDoc->WriteTo(xmlTextWriter);
					xmlTextWriter->Flush();
					delete xmlTextWriter;

					m_dataDirectResource = stringWriter->GetStringBuilder()->ToString();
				}
				else
				{
					// It's some binary data/unknown format
					array<unsigned char> ^binary = gcnew array<unsigned char> (int(stream->Length));
					stream->Read(binary, 0, binary->Length);

					m_dataDirectResource = binary;
				}
			}
			catch (...)
			{
				m_dataDirectResource = gcnew String("<Unreadable>");
			}
		}
	}
	
	ManagedFuncs::Resources::Pair<String ^>^ Resources::getSelectedStreamNextKeyAndData()
	{
		if (m_resourceReader == nullptr && m_dataEnumerator == nullptr && !m_isDirectResource)
			throw gcnew Exception("ERROR: No stream selected");

		if (m_isDirectResource)
		{
			if (m_dataDirectResource != nullptr)
			{
				auto dataDirectResource = m_dataDirectResource;
				m_dataDirectResource = nullptr;

				return gcnew ManagedFuncs::Resources::Pair<String ^> ("<Data>", dataDirectResource);
			}
			else
				return nullptr;
		}
		else
		{
			// NOTE: 'MoveNext()' must be called even before accessing the enumerator for the first time
			if (!m_dataEnumerator->MoveNext())
			{
				m_resourceReader->Close();

				m_isDirectResource = false;
				m_resourceReader = nullptr;
				m_dataEnumerator = nullptr;

				return nullptr;
			}

			// Identify resource type and read it appropriately
			Object ^data = nullptr;

			try
			{
				String ^dataKey = m_dataEnumerator->Key->ToString()->ToLower();
				Object ^Value = m_dataEnumerator->Value;
				Type ^typeData = m_dataEnumerator->Value->GetType();

				if (typeData == Icon::typeid)
				{
					// It's an icon
					data = static_cast<Icon ^> (Value);
				}
				else if (typeData == Cursor::typeid)
				{
					// It's a cursor
					data = static_cast<Cursor ^> (Value);
				}
				else if (typeData == Bitmap::typeid)
				{
					// It's a image
					data = static_cast<Bitmap ^> (Value);
				}
				else if (typeData == XmlDocument::typeid)
				{
					// It's a XML document
					XmlDocument ^xmlDoc = static_cast<XmlDocument ^> (Value);

					auto stringWriter = gcnew StringWriter();

					auto xmlTextWriter = XmlWriter::Create(stringWriter);
					xmlDoc->WriteTo(xmlTextWriter);
					xmlTextWriter->Flush();
					delete xmlTextWriter;

					data = stringWriter->GetStringBuilder()->ToString();
				}
				else if (typeData == Stream::typeid ||
						 typeData == UnmanagedMemoryStream::typeid)
				{
					Stream ^stream = static_cast<Stream ^> (Value);

					if (dataKey->EndsWith(".bmp") ||
						dataKey->EndsWith(".jpg") ||
						dataKey->EndsWith(".png") ||
						dataKey->EndsWith(".jpeg") ||
						dataKey->EndsWith(".gif") ||
						dataKey->EndsWith(".tiff"))
					{
						// It's an image
						data = static_cast<Bitmap ^>(gcnew Bitmap(stream));
					}
					else if (dataKey->EndsWith(".xml") ||
							 dataKey->EndsWith(".xaml"))
					{
						// It's a XML text
						XmlDocument ^xmlDoc = gcnew XmlDocument();
						xmlDoc->Load(static_cast<Stream ^> (Value));

						auto stringWriter = gcnew StringWriter();

						auto xmlTextWriter = XmlWriter::Create(stringWriter);
						xmlDoc->WriteTo(xmlTextWriter);
						xmlTextWriter->Flush();
						delete xmlTextWriter;

						data = stringWriter->GetStringBuilder()->ToString();
					}
					else
					{
						// It's some binary data/unknown format
						array<unsigned char> ^binary = gcnew array<unsigned char>(int(stream->Length));
						stream->Read(binary, 0, binary->Length);

						data = binary;
					}
				}

				if (data == nullptr)
					data = Value->ToString();
			}
			catch (TypeLoadException^)
			{
				data = gcnew String("<User defined type>");
			}
			catch (FileNotFoundException^)
			{
				data = gcnew String("<In external assembly>");
			}
			catch (...)
			{
				data = gcnew String("<Unreadable>");
			}

			return gcnew ManagedFuncs::Resources::Pair<String ^>(m_dataEnumerator->Key->ToString(), data);
		}
	}
}


// The following are the exports from this DLL
extern "C"
{
	void * __cdecl createResourceReader(wchar_t *szAssemblyName)
	{
		ManagedFuncs::Resources ^rsrcReader;

		try
		{
			rsrcReader = gcnew ManagedFuncs::Resources(gcnew String(szAssemblyName));
		}
		catch (...)
		{
			return nullptr;
		}

		return static_cast<IntPtr>(GCHandle::Alloc(rsrcReader)).ToPointer();
	}

	void __cdecl destroyResourceReader(void *hResourceReader)
	{
		if (hResourceReader == nullptr)
			return;

		try
		{
			ManagedFuncs::Resources ^rsrcReader = static_cast<ManagedFuncs::Resources ^>(static_cast<GCHandle>(IntPtr(hResourceReader)).Target);
			delete rsrcReader;

			static_cast<GCHandle>(IntPtr(hResourceReader)).Free();

			GC::Collect();  // NOTE: Force garbage collection
		}
		catch (...)	{}		
	}

	int __cdecl getNoOfResourceStreamNames(void *hResourceReader)
	{
		if (hResourceReader == nullptr)
			return 0;

		array<String ^> ^rsrcStreamNames;

		try
		{
			ManagedFuncs::Resources ^rsrcReader = static_cast<ManagedFuncs::Resources ^>(static_cast<GCHandle>(IntPtr(hResourceReader)).Target);
			assert(rsrcReader != nullptr);

			rsrcStreamNames = rsrcReader->getStreamNames();
			if (rsrcStreamNames == nullptr)
				return 0;
		}
		catch (...)
		{
			return 0;
		}

		return rsrcStreamNames->Length;
	}

	bool __cdecl getResourceStreamName(void *hResourceReader, int idxStream, wchar_t* szStreamName, int cStreamName)
	{
		if (hResourceReader == nullptr)
			return false;

		try
		{
			ManagedFuncs::Resources ^rsrcReader = static_cast<ManagedFuncs::Resources ^>(static_cast<GCHandle>(IntPtr(hResourceReader)).Target);
			assert(rsrcReader != nullptr);

			array<String ^> ^rsrcStreamNames = rsrcReader->getStreamNames();
			if (rsrcStreamNames == nullptr)
				return false;

			assert(idxStream < rsrcStreamNames->Length);

			pin_ptr<const wchar_t> pinStreamName = PtrToStringChars(rsrcStreamNames[idxStream]);
			wcscpy_s(szStreamName, cStreamName, pinStreamName);
		}
		catch (...)
		{
			return false;
		}
	
		return true;
	}

	bool __cdecl selectResourceStream(void *hResourceReader, int idxStream)
	{
		try
		{
			ManagedFuncs::Resources ^rsrcReader = static_cast<ManagedFuncs::Resources ^>(static_cast<GCHandle>(IntPtr(hResourceReader)).Target);
			assert(rsrcReader != nullptr);

			rsrcReader->selectStream(idxStream);
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

	bool getNextResourceSelectedStreamKeyAndValue(void *hResourceReader, wchar_t* szKey, int cKey, int *ptypeData, void **pszData, int *pcData)
	{
		assert(hResourceReader && szKey);

		try
		{
			ManagedFuncs::Resources ^rsrcReader = static_cast<ManagedFuncs::Resources ^> (static_cast<GCHandle> (IntPtr(hResourceReader)).Target);
			assert(rsrcReader != nullptr);

			auto KeyAndData = rsrcReader->getSelectedStreamNextKeyAndData();
			if (KeyAndData == nullptr)
				return false;

			// Copy key name
			pin_ptr<const wchar_t> pinKey = PtrToStringChars(KeyAndData->Item1);
			wcscpy_s(szKey, cKey, pinKey);

			if (ptypeData != NULL && pszData != NULL && pcData != NULL)
			{
				// Identify data type and copy it
				Type ^typeData = KeyAndData->Item2->GetType();

				if (typeData == Icon::typeid)
				{
					// It's an icon
					System::Drawing::Icon ^icon = static_cast<System::Drawing::Icon ^> (KeyAndData->Item2);
					HANDLE hIcon = CopyImage(icon->Handle.ToPointer(), IMAGE_ICON, icon->Size.Width, icon->Size.Height, 0);
					
					*ptypeData = static_cast<int> (ManagedFuncs::DataType::IconHandle);
					*pszData = new PTR((PTR)hIcon);
					*pcData = 1;
				}
				else if (typeData == Cursor::typeid)
				{
					// It's a cursor
					System::Windows::Forms::Cursor ^cursor = static_cast<System::Windows::Forms::Cursor ^> (KeyAndData->Item2);
					HANDLE hCursor = CopyImage(cursor->Handle.ToPointer(), IMAGE_CURSOR, 0, 0, 0);

					*ptypeData = static_cast<int> (ManagedFuncs::DataType::CursorHandle);
					*pszData = new PTR((PTR)hCursor);
					*pcData = 1;
				}
				else if (typeData == Bitmap::typeid)
				{
					// It's a image
					System::Drawing::Bitmap ^bitmap = static_cast<System::Drawing::Bitmap ^> (KeyAndData->Item2);
					HANDLE hBitmap = CopyImage(bitmap->GetHbitmap().ToPointer(), IMAGE_BITMAP, 0, 0, 0);

					*ptypeData = static_cast<int> (ManagedFuncs::DataType::BitmapHandle);
					*pszData = new PTR((PTR) hBitmap);
					*pcData = 1;
				}
				else if (typeData == String::typeid)
				{
					// It's string data
					pin_ptr<const wchar_t> pinStringData = PtrToStringChars(static_cast<String ^> (KeyAndData->Item2));
					int sizeStringData = int(wcslen(pinStringData)) + 1;	// CAUTION: Don't forget '+ 1' for null terminator

					*ptypeData = static_cast<int> (ManagedFuncs::DataType::String);
					*pszData = new wchar_t[sizeStringData];
					*pcData = sizeStringData;
					
					wcscpy_s((wchar_t *)(*pszData), *pcData, pinStringData);
				}
				else // It's binary data
				{
					auto arrBinaryData = static_cast<array<unsigned char> ^> (KeyAndData->Item2);
					pin_ptr<unsigned char> pinBinaryData = &arrBinaryData[0];	// Pin this managed array
					unsigned char *pBinaryData = new unsigned char[arrBinaryData->Length];
					memcpy_s(pBinaryData, arrBinaryData->Length, pinBinaryData, arrBinaryData->Length);

					*ptypeData = static_cast<int> (ManagedFuncs::DataType::Binary);
					*pszData = pBinaryData;
					*pcData = arrBinaryData->Length;
				}
			}
		}
		catch (...)
		{
			return false;
		}

		return true;
	}
}