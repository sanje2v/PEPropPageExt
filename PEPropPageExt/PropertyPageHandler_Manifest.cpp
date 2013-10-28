#include "PropertyPageHandler.h"


PropertyPageHandler_Manifest::PropertyPageHandler_Manifest(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter)
{
	m_hStaticManifestSource = GetDlgItem(m_hWnd, IDC_STATICMANIFESTSRC);
	m_hComboManifestName = GetDlgItem(m_hWnd, IDC_CMBMANIFESTNAME);
	m_hComboManifestLang = GetDlgItem(m_hWnd, IDC_CMBMANIFESTLANG);
	m_hEditManifest = GetDlgItem(m_hWnd, IDC_EDITMANIFEST);

	// Setup controls with layout manager
	m_pLayoutManager->AddChildConstraint(IDC_EDITMANIFEST, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
}

void PropertyPageHandler_Manifest::OnInitDialog()
{
	// Fill them with data
	vector<tstring> ManifestNames;
	vector<LPTSTR> ManifestRawNames;
	static const TCHAR szManifestSourceDesc[][40] = {_T("Manifest is embedded as resource data"),
													_T("Manifest is in an external file"),
													_T("Manifest was not found")};
	
	switch (m_PEReaderWriter.GetManifestSource())
	{
	case PEReadWrite::ManifestInternal:	// Manifest is embedded as resource in executable
		// Set text for the static control
		SetDlgItemText(m_hWnd, IDC_STATICMANIFESTSRC, szManifestSourceDesc[0]);

		// Enumerate manifest names and add them to combobox
		ManifestNames = m_PEReaderWriter.GetManifestNames();
		ManifestRawNames = m_PEReaderWriter.GetManifestRawNames();
			
		ComboBox_ResetContent(m_hComboManifestName);
		for (int i = 0; i < ManifestNames.size(); i++)
		{
			// Add new items to 'cmbManifestName'
			ComboBox_AddString(m_hComboManifestName, (LPSTR) ManifestNames[i].c_str());
			// Each combo item has its actual id assigned to it
			ComboBox_SetItemData(m_hComboManifestName, i, ManifestRawNames[i]);
		}

		// Select the first item on the combos
		ComboBox_SetCurSel(m_hComboManifestName, 0);
		ComboBox_SetCurSel(m_hComboManifestLang, 0);
		cmbManifestName_OnSelectionChanged(m_hComboManifestName, 0);
		
		break;

	case PEReadWrite::ManifestExternal:				// Manifest is in external file
		// Set static control text
		SetDlgItemText(m_hWnd, IDC_STATICMANIFESTSRC, szManifestSourceDesc[1]);

		// Read the manifest and show it in the edit box
		SetWindowText(m_hEditManifest, (LPTSTR) m_PEReaderWriter.GetManifest().c_str());

		break;

	case PEReadWrite::ManifestNotPresent:	// No manifest
		// Set static control text
		SetDlgItemText(m_hWnd, IDC_STATICMANIFESTSRC, szManifestSourceDesc[2]);
	}
}

void PropertyPageHandler_Manifest::cmbManifestName_OnSelectionChanged(HWND hControl, int SelectedIndex)
{
	// A new item from the dropdown combo box 'cmbManifestName' has been selected
	LPTSTR ManifestRawName = (LPTSTR) ComboBox_GetItemData(hControl, SelectedIndex);
	vector<WORD> ManifestLangs = m_PEReaderWriter.GetManifestLangsFromRawName(ManifestRawName);

	// Update language items in 'cmbManifestLang'
	ComboBox_ResetContent(m_hComboManifestLang);
	for (int i = 0; i < ManifestLangs.size(); i++)
	{
		ComboBox_AddString(m_hComboManifestLang, (LPTSTR) LCIDtoLangName(ManifestLangs[i]).c_str());
		ComboBox_SetItemData(m_hComboManifestLang, i, ManifestLangs[i]);
	}

	// Select the first language item
	ComboBox_SetCurSel(m_hComboManifestLang, 0);
	cmbManifestLang_OnSelectionChanged(m_hComboManifestLang, 0);
}

void PropertyPageHandler_Manifest::cmbManifestLang_OnSelectionChanged(HWND hControl, int SelectedIndex)
{
	// A new item from the dropdown combo box 'cmbManifestLang' has been selected
	LPTSTR ManifestRawName = (LPTSTR) ComboBox_GetItemData(m_hComboManifestName, ComboBox_GetCurSel(m_hComboManifestName));
	WORD ManifestLang = (WORD) ComboBox_GetItemData(hControl, SelectedIndex);

	// Show the new language manifest in the edit box
	SetWindowText(m_hEditManifest, (LPTSTR) m_PEReaderWriter.GetManifest(ManifestRawName, ManifestLang).c_str());
}