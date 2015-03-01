#pragma once
#include "unique_handle.h"
#include <Windows.h>

#define SETTINGS_REGISTRY_KEY		L"Software\\SWTBASE\\PEPropPageExt\\Settings"


enum class SettingType
{
	Hide_AllTabs,
	Hide_MSDOSHeaderTab,
	Hide_PEHeadersTab,
	Hide_SectionsTab,
	Hide_ManifestTab,
	Hide_ImportsTab,
	Hide_ExportsTab,
	Hide_ResourcesTab,
	Hide_ExceptionTab,
	Hide_BaseRelocTab,
	Hide_DebugTab,
	Hide_LoadConfigTab,
	Hide_TLSTab,
	Hide_CLRTab,
	Hide_OverviewTab,
	Hide_ToolsTab
};

bool ReadSettings(SettingType type)
{
	auto funcRegKeyReleaser = [](HKEY hKey) { RegCloseKey(hKey); };
	unique_handle<HKEY, decltype(funcRegKeyReleaser)> hRegSettings(HKEY(NULL), funcRegKeyReleaser);

	if (RegOpenKeyEx(HKEY_CURRENT_USER, SETTINGS_REGISTRY_KEY, 0, KEY_READ, &hRegSettings))
		return false;

	LPWSTR szValueName;

	switch (type)
	{
		case SettingType::Hide_AllTabs: szValueName = L"Hide_AllTabs"; break;
		case SettingType::Hide_MSDOSHeaderTab: szValueName = L"Hide_MSDOSHeaderTab"; break;
		case SettingType::Hide_PEHeadersTab: szValueName = L"Hide_PEHeadersTab"; break;
		case SettingType::Hide_SectionsTab: szValueName = L"Hide_SectionsTab"; break;
		case SettingType::Hide_ManifestTab: szValueName = L"Hide_ManifestTab"; break;
		case SettingType::Hide_ImportsTab: szValueName = L"Hide_ImportsTab"; break;
		case SettingType::Hide_ExportsTab: szValueName = L"Hide_ExportsTab"; break;
		case SettingType::Hide_ResourcesTab: szValueName = L"Hide_ResourcesTab"; break;
		case SettingType::Hide_ExceptionTab: szValueName = L"Hide_ExceptionTab"; break;
		case SettingType::Hide_BaseRelocTab: szValueName = L"Hide_BaseRelocTab"; break;
		case SettingType::Hide_DebugTab: szValueName = L"Hide_DebugTab"; break;
		case SettingType::Hide_LoadConfigTab: szValueName = L"Hide_LoadConfigTab"; break;
		case SettingType::Hide_TLSTab: szValueName = L"Hide_TLSTab"; break;
		case SettingType::Hide_CLRTab: szValueName = L"Hide_CLRTab"; break;
		case SettingType::Hide_OverviewTab: szValueName = L"Hide_OverviewTab"; break;
		case SettingType::Hide_ToolsTab: szValueName = L"Hide_ToolsTab"; break;
		default: szValueName = L"";
	}

	DWORD Value = FALSE;
	DWORD cbValue = sizeof(Value);

	RegQueryValueEx(hRegSettings.get(), szValueName, NULL, NULL, LPBYTE(&Value), &cbValue);

	return (Value != FALSE);	// Apparently, casting to 'bool' gives C4800 warning
}