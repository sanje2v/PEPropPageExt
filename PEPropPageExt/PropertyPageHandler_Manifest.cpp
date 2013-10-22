#include "PropertyPageHandler.h"


THREAD_ISOLATED_STORAGE static HWND hStaticManifestSource;
THREAD_ISOLATED_STORAGE static HWND hComboManifestName;
THREAD_ISOLATED_STORAGE static HWND hComboManifestLang;
THREAD_ISOLATED_STORAGE static HWND hEditManifest;


void PropertyPageHandler_Manifest::OnInitDialog()
{
	hStaticManifestSource = GetDlgItem(m_hWnd, IDC_STATICSRC);
	hComboManifestName = GetDlgItem(m_hWnd, IDC_CMBMANIFESTNAME);
	hComboManifestLang = GetDlgItem(m_hWnd, IDC_CMBMANIFESTLANG);
	hEditManifest = GetDlgItem(m_hWnd, IDC_EDITMANIFEST);

	// Setup controls with layout manager
	m_pLayoutManager->AddChildConstraint(IDC_EDITMANIFEST, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	// Set background color for static control
	SetClassLongPtr(hStaticManifestSource, GCLP_HBRBACKGROUND, GetClassLongPtr(m_hWnd, GCLP_HBRBACKGROUND));
	// Set window procedure for static control
	//SetWindowLongPtr(hStaticManifestSource, GWLP_WNDPROC, (LONG_PTR) StaticsWndProc);

	// Fill them with data
	vector<tstring> ManifestNames;
	vector<LPTSTR> ManifestRawNames;
	
	switch (m_PEReaderWriter.GetManifestSource())
	{
	case PEReadWrite::ManifestInternal:	// Manifest is embedded as resource in executable
		// Set text for the static control
		//SetWindowLongPtr(hStaticManifestSource, GWLP_USERDATA,
							//(LONG_PTR) _T("Manifest is embedded as resource data."));

		// Enumerate manifest names and add them to combobox
		ManifestNames = m_PEReaderWriter.GetManifestNames();
		ManifestRawNames = m_PEReaderWriter.GetManifestRawNames();
			
		ComboBox_ResetContent(hComboManifestName);
		for (int i = 0; i < ManifestNames.size(); i++)
		{
			// Add new items to 'cmbManifestName'
			ComboBox_AddString(hComboManifestName, (LPSTR) ManifestNames[i].c_str());
			// Each combo item has its actual id assigned to it
			ComboBox_SetItemData(hComboManifestName, i, ManifestRawNames[i]);
		}

		// Select the first item on the combos
		ComboBox_SetCurSel(hComboManifestName, 0);
		ComboBox_SetCurSel(hComboManifestLang, 0);
		cmbManifestName_OnSelectionChanged(hComboManifestName, 0);
		
		break;

	case PEReadWrite::ManifestExternal:				// Manifest is in external file
		// Set static control text
		SetWindowLongPtr(hStaticManifestSource, GWLP_USERDATA,
							(LONG_PTR) _T("Manifest is in an external file."));

		// Read the manifest and show it in the edit box
		SetWindowText(hEditManifest, (LPTSTR) m_PEReaderWriter.GetManifest().c_str());

		break;

	case PEReadWrite::ManifestNotPresent:	// No manifest
		// Set static control text
		SetWindowLongPtr(hStaticManifestSource, GWLP_USERDATA,
							(LONG_PTR) _T("Manifest was not found."));
	}
}

void PropertyPageHandler_Manifest::cmbManifestName_OnSelectionChanged(HWND hControl, int SelectedIndex)
{
	// A new item from the dropdown combo box 'cmbManifestName' has been selected
	LPTSTR ManifestRawName = (LPTSTR) ComboBox_GetItemData(hControl, SelectedIndex);
	vector<WORD> ManifestLangs = m_PEReaderWriter.GetManifestLangsFromRawName(ManifestRawName);

	// Update language items in 'cmbManifestLang'
	ComboBox_ResetContent(hComboManifestLang);
	for (int i = 0; i < ManifestLangs.size(); i++)
	{
		ComboBox_AddString(hComboManifestLang, (LPTSTR) LCIDtoLangName(ManifestLangs[i]).c_str());
		ComboBox_SetItemData(hComboManifestLang, i, ManifestLangs[i]);
	}

	// Select the first language item
	ComboBox_SetCurSel(hComboManifestLang, 0);
	cmbManifestLang_OnSelectionChanged(hComboManifestLang, 0);
}

void PropertyPageHandler_Manifest::cmbManifestLang_OnSelectionChanged(HWND hControl, int SelectedIndex)
{
	// A new item from the dropdown combo box 'cmbManifestLang' has been selected
	LPTSTR ManifestRawName = (LPTSTR) ComboBox_GetItemData(hComboManifestName, ComboBox_GetCurSel(hComboManifestName));
	WORD ManifestLang = (WORD) ComboBox_GetItemData(hControl, SelectedIndex);

	// Show the new language manifest in the edit box
	SetWindowText(hEditManifest, (LPTSTR) m_PEReaderWriter.GetManifest(ManifestRawName, ManifestLang).c_str());
}