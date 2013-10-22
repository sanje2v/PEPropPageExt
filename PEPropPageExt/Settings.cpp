#include "StdAfx.h"
#include "Settings.h"


bool ReadSettings(SettingType type)
{
	HKEY hRegSettings;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\SWTBASE\\PEPropExt\\Settings"), 0, KEY_READ, &hRegSettings))
		return false;
	
	LPTSTR szValueName;
	switch (type)
	{
	case Hide_AllTabs: szValueName = _T("Hide_AllTabs"); break;
	case Hide_MSDOSHeaderTab: szValueName = _T("Hide_MSDOSHeaderTab"); break;
	case Hide_PEHeadersTab: szValueName = _T("Hide_PEHeadersTab"); break;
	case Hide_SectionsTab: szValueName = _T("Hide_SectionsTab"); break;
	case Hide_ManifestTab: szValueName = _T("Hide_ManifestTab"); break;
	case Hide_ImportsTab: szValueName = _T("Hide_ImportsTab"); break;
	case Hide_ExportsTab: szValueName = _T("Hide_ExportsTab"); break;
	case Hide_ResourcesTab: szValueName = _T("Hide_ResourcesTab"); break;
	case Hide_ExceptionTab: szValueName = _T("Hide_ExceptionTab"); break;
	case Hide_BaseRelocTab: szValueName = _T("Hide_BaseRelocTab"); break;
	case Hide_DebugTab: szValueName = _T("Hide_DebugTab"); break;
	case Hide_LoadConfigTab: szValueName = _T("Hide_LoadConfigTab"); break;
	case Hide_TLSTab: szValueName = _T("Hide_TLSTab"); break;
	case Hide_CLRTab: szValueName = _T("Hide_CLRTab"); break;
	case Hide_OverviewTab: szValueName = _T("Hide_OverviewTab"); break;
	case Hide_ToolsTab: szValueName = _T("Hide_ToolsTab"); break;
	default: szValueName = _T("");
	}

	DWORD Value = 0;
	DWORD cbValue = sizeof(Value);
	RegQueryValueEx(hRegSettings, szValueName, NULL, NULL, (LPBYTE) &Value, &cbValue);
	RegCloseKey(hRegSettings);

	return Value != 0;	// Apparently, casting to 'bool' gives C4800 warning
}
