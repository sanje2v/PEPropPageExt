#include "stdafx.h"
#include "PropertyPageHandler.h"


PropertyPageHandler_Manifest::PropertyPageHandler_Manifest(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, std::ref(PEReaderWriter))
{
	m_hImgInfo = GetDlgItem(m_hWnd, IDC_IMGINFO);
	m_hStaticManifestSource = GetDlgItem(m_hWnd, IDC_STATICMANIFESTSRC);
	m_hComboManifestName = GetDlgItem(m_hWnd, IDC_CMBMANIFESTNAME);
	m_hComboManifestLang = GetDlgItem(m_hWnd, IDC_CMBMANIFESTLANG);
	m_hEditManifest = GetDlgItem(m_hWnd, IDC_EDITMANIFEST);

	// Setup controls with layout manager
	m_LayoutManager.AddChildConstraint(IDC_EDITMANIFEST, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// In the following code, we try to use 'SHGetStockIconInfo()' to load an OEM
	//  information icon. This function is only available in Windows Vista and later.
	//  Traditionally, we would use 'LoadImage()' to do the same thing as this function.
	//  But 'LoadImage()' tends to give us a squished icon from system shared icon store.
	//  If the following code fails (in older Windows version), it's still ok because in
	//  dialog editor we have made it so that Windows Resource loader uses
	//  the 'LoadImage()' to load OEM information icon.
	typedef struct
	{
		DWORD cbSize;
		HICON hIcon;
		int   iSysImageIndex;
		int   iIcon;
		WCHAR szPath[MAX_PATH];
	} SHSTOCKICONINFO;
	typedef HRESULT(WINAPI* pSHGetStockIconInfo)(int siid, UINT uFlags, SHSTOCKICONINFO *psii);
	const int SIID_INFO = 79;
	const UINT SHGSI_ICON = 0x100;
	
	HMODULE hModShell32 = GetModuleHandle(L"shell32.dll");
	if (!hModShell32)
		return;

	pSHGetStockIconInfo SHGetStockIconInfo = pSHGetStockIconInfo(GetProcAddress(hModShell32, "SHGetStockIconInfo"));
	if (!SHGetStockIconInfo)
		return;

	// Get OEM icon resource
	SHSTOCKICONINFO psii = { 0 };
	psii.cbSize = sizeof(psii);
	SHGetStockIconInfo(SIID_INFO, SHGSI_ICON, &psii);

	// Give it to static control
	Static_SetIcon(m_hImgInfo, psii.hIcon);
}

void PropertyPageHandler_Manifest::OnInitDialog()
{
	// Fill them with data
	enum DescFor
	{
		InternalManifest = 0,
		ExternalManifest = 1,
		NoManifest = 2
	};
	static const WCHAR szManifestSourceDesc[][64] =
	{
		L"Manifest is embedded as resource data",
		L"Manifest is in an external file",
		L"No manifest was found"
	};
	
	switch (m_PEReaderWriter.getManifestSource())
	{
		case PEReadWrite::ManifestSource::Internal:	// Manifest is embedded as resource in executable
		{
			// Set text for the static control
			SetWindowText(m_hStaticManifestSource, szManifestSourceDesc[DescFor::InternalManifest]);

			// Enumerate manifest names and add them to combobox
			PIMAGE_RESOURCE_DIRECTORY pResDir;
			int err = m_PEReaderWriter.getRootResourceDirectory(std::ref(pResDir));
			if (err)
			{
				LogError(L"ERROR: Couldn't read root resource directory. Manifest is corrupted.", true);
				return;
			}

			bool bFoundManifestEntry = false;
			PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry;
			const DWORD NumberOfEntries = DWORD(pResDir->NumberOfNamedEntries) + DWORD(pResDir->NumberOfIdEntries);
			for (DWORD i = 0; i < NumberOfEntries; ++i)
			{
				err = m_PEReaderWriter.getResourceDirectoryEntry(std::cref(pResDir), i, std::ref(pResDirEntry));
				if (err)
					return;

				if (pResDirEntry->NameIsString)
				{
					// Entry has a name string
					wstring EntryName;
					err = m_PEReaderWriter.getResourceEntryName(std::cref(pResDirEntry), std::ref(EntryName));
					if (err)
						return;

					// Name string is case insensitive
					std::transform(EntryName.cbegin(), EntryName.cend(), EntryName.begin(), ::tolower);

					if (EntryName == L"manifest")
					{
						bFoundManifestEntry = true;
						break;
					}
				}
				else
				{
					if (pResDirEntry->Id == PEReadWrite::ID_MANIFEST)
					{
						bFoundManifestEntry = true;
						break;
					}
				}
			}

			if (bFoundManifestEntry)
			{
				// We need to transverse the found node to find out about names and languages
				if (pResDirEntry->DataIsDirectory)
				{
					PIMAGE_RESOURCE_DIRECTORY pManifestResDir;
					err = m_PEReaderWriter.getNextResourceDirectory(std::cref(pResDirEntry), std::ref(pManifestResDir));
					if (err)
						return;

					PIMAGE_RESOURCE_DIRECTORY_ENTRY pManifestResDirEntry;
					const DWORD NumberOfManifestEntries = DWORD(pManifestResDir->NumberOfNamedEntries) + DWORD(pManifestResDir->NumberOfIdEntries);
					for (DWORD i = 0; i < NumberOfManifestEntries; ++i)
					{
						err = m_PEReaderWriter.getResourceDirectoryEntry(std::cref(pManifestResDir), i, std::ref(pManifestResDirEntry));
						if (err)
							return;

						int index = CB_ERR;
						if (pManifestResDirEntry->NameIsString)
						{
							wstring EntryName;
							err = m_PEReaderWriter.getResourceEntryName(std::cref(pManifestResDirEntry), std::ref(EntryName));
							if (err)
								return;

							index = ComboBox_AddString(m_hComboManifestName, EntryName.c_str());
						}
						else
							index = ComboBox_AddString(m_hComboManifestName, WORD_toString(pManifestResDirEntry->Id).c_str());

						// Add associated index data
						if (index > CB_ERR)
							ComboBox_SetItemData(m_hComboManifestName, index, pManifestResDirEntry);
					}

					// Select first item in combo
					ComboBox_SetCurSel(m_hComboManifestName, 0);
					cmbManifestName_OnSelectionChanged(m_hComboManifestName, 0);
				}
			}
		}
		break;

		case PEReadWrite::ManifestSource::External:				// Manifest is in external file
		{
			// Set static control text
			SetWindowText(m_hStaticManifestSource, szManifestSourceDesc[DescFor::ExternalManifest]);

			// Read the manifest and show it in the edit box
			HANDLE hManifestFile = CreateFile(m_PEReaderWriter.makeManifestFilename().c_str(),
											  GENERIC_READ,
											  FILE_SHARE_READ,
											  NULL,
											  OPEN_EXISTING,
											  FILE_ATTRIBUTE_NORMAL,
											  NULL);
			if (hManifestFile == INVALID_HANDLE_VALUE)
			{
				LogError(L"ERROR: Couldn't open external manifest file. Another process might have opened it with share lock.", true);
				return;
			}

			DWORD sizeManifestFile = GetFileSize(hManifestFile, NULL);
			if (sizeManifestFile == 0)
			{
				CloseHandle(hManifestFile);
				return;
			}

			unique_ptr<BYTE> pManifestBuffer(new BYTE[sizeManifestFile]{0});
			DWORD cBytesRead;
			if (ReadFile(hManifestFile, pManifestBuffer.get(), sizeManifestFile, &cBytesRead, NULL) == TRUE)
				Edit_SetText(m_hEditManifest, MultiByte_toString((const char *)(pManifestBuffer.get()), false, sizeManifestFile).c_str());

			CloseHandle(hManifestFile);
		}
		break;

		case PEReadWrite::ManifestSource::NotPresent:	// No manifest
			// Set static control text
			SetWindowText(m_hStaticManifestSource, szManifestSourceDesc[DescFor::NoManifest]);

			break;
	}
}

void PropertyPageHandler_Manifest::cmbManifestName_OnSelectionChanged(HWND hControl, int SelectedIndex)
{
	// A new item from the dropdown combo box 'cmbManifestName' has been selected
	ComboBox_ResetContent(m_hComboManifestLang);
	Edit_SetText(m_hEditManifest, L"");
	
	// Transverse the resource directory associated with the selected item and enumerate languages for the given Id/Name
	PIMAGE_RESOURCE_DIRECTORY_ENTRY pManifestResDirEntry = PIMAGE_RESOURCE_DIRECTORY_ENTRY(ComboBox_GetItemData(m_hComboManifestName, SelectedIndex));
	if (pManifestResDirEntry == NULL)
		return;

	if (pManifestResDirEntry->DataIsDirectory)
	{
		PIMAGE_RESOURCE_DIRECTORY pManifestIdResDir;
		int err = m_PEReaderWriter.getNextResourceDirectory(std::cref(pManifestResDirEntry), std::ref(pManifestIdResDir));
		if (err)
			return;

		const DWORD NumberOfLangEntries = DWORD(pManifestIdResDir->NumberOfNamedEntries) + DWORD(pManifestIdResDir->NumberOfIdEntries);
		for (DWORD i = 0; i < NumberOfLangEntries; ++i)
		{
			PIMAGE_RESOURCE_DIRECTORY_ENTRY pManifestIdResDirEntry;
			err = m_PEReaderWriter.getResourceDirectoryEntry(std::cref(pManifestIdResDir), i, std::ref(pManifestIdResDirEntry));
			if (err)
				return;

			if (!pManifestIdResDirEntry->DataIsDirectory)
			{
				int index = CB_ERR;
				if (pManifestIdResDirEntry->NameIsString)
				{
					wstring EntryName;
					err = m_PEReaderWriter.getResourceEntryName(std::cref(pManifestResDirEntry), std::ref(EntryName));
					if (err)
						continue;

					index = ComboBox_AddString(m_hComboManifestName, EntryName.c_str());
				}
				else
				{
					wstring LocaleName = LCID_toLocaleName(pManifestIdResDirEntry->Id);

					if (LocaleName.empty())
						index = ComboBox_AddString(m_hComboManifestLang, WORD_toString(pManifestIdResDirEntry->Id).c_str());
					else
						index = ComboBox_AddString(m_hComboManifestLang, LocaleName.c_str());
				}
				
				PIMAGE_RESOURCE_DATA_ENTRY pManifestResData;
				err = m_PEReaderWriter.getResourceEntryData(std::cref(pManifestIdResDirEntry), std::ref(pManifestResData));
				if (err)
					continue;

				ComboBox_SetItemData(m_hComboManifestLang, index, pManifestResData);
			}
		}

		// Select the first language item
		ComboBox_SetCurSel(m_hComboManifestLang, 0);
		cmbManifestLang_OnSelectionChanged(m_hComboManifestLang, 0);
	}
}

void PropertyPageHandler_Manifest::cmbManifestLang_OnSelectionChanged(HWND hControl, int SelectedIndex)
{
	// A new item from the dropdown combo box 'cmbManifestLang' has been selected
	Edit_SetText(m_hEditManifest, L""); // In case we fail later, clear the edit box

	// Get the resource data structure that is associated with selected language item and read the manifest into edit box
	PIMAGE_RESOURCE_DATA_ENTRY pManifestResData = PIMAGE_RESOURCE_DATA_ENTRY(ComboBox_GetItemData(m_hComboManifestLang, SelectedIndex));
	if (pManifestResData == NULL)
		return;

	LPBYTE pManifestData;
	DWORD cManifestData;
	int err = m_PEReaderWriter.getResourceData(std::cref(pManifestResData), std::ref(pManifestData), std::ref(cManifestData));
	if (err)
	{
		LogError(L"ERROR: Failed to read manifest for selected Id and language. Resource is corrupted.", true);
		return;
	}

	Edit_SetText(m_hEditManifest, MultiByte_toString((const char *)(pManifestData), false, cManifestData).c_str());
}