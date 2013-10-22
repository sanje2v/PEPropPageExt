#pragma once

enum SettingType
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

bool ReadSettings(SettingType type);