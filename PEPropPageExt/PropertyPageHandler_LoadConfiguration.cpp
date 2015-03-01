#include "stdafx.h"
#include "PropertyPageHandler.h"


PropertyPageHandler_LoadConfiguration::PropertyPageHandler_LoadConfiguration(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, std::ref(PEReaderWriter))
{
	m_hListViewLoadConfig = GetDlgItem(m_hWnd, IDC_LISTLOADCONFIG);
	m_hListViewSEH = GetDlgItem(m_hWnd, IDC_LISTSEH);

	// Setup controls with layout manager
	m_LayoutManager.AddChildConstraint(IDC_LISTLOADCONFIG, CWA_LEFTRIGHT, CWA_TOP);
	m_LayoutManager.AddChildConstraint(IDC_LISTSEH, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Set full row selection style for list views
	ListView_SetExtendedListViewStyleEx(m_hListViewLoadConfig,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);
	ListView_SetExtendedListViewStyleEx(m_hListViewSEH, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
}

void PropertyPageHandler_LoadConfiguration::OnInitDialog()
{
	// Fill them with data
	vector<wstring> SEHs;

	union
	{
		PIMAGE_LOAD_CONFIG_DIRECTORY32 pLoadConfig32;
		PIMAGE_LOAD_CONFIG_DIRECTORY64 pLoadConfig64;
	};

	PEReadWrite::PEType PEType = m_PEReaderWriter.getPEType();

	int err;
	switch (PEType)
	{
		case PEReadWrite::PEType::PE32:
		{
			RTTI::GetTooltipInfo(m_LoadConfigTooltipInfo, UINT_PTR(NULL), RTTI::RTTI_LOAD_CONFIG32);

			err = m_PEReaderWriter.getLoadConfigurationTable(std::ref(pLoadConfig32));
			if (err)
			{
				LogError(L"ERROR: Load configuration information is not complete. File is not valid.", true);
				return;
			}

			m_LoadConfigItemsInfo =
			{
				{ L"Size", DWORD_toString(pLoadConfig32->Size), FormattedBytesSize(pLoadConfig32->Size) },
				{ L"Time Date Stamp", DWORD_toString(pLoadConfig32->TimeDateStamp), TimeDateStamp_toString(pLoadConfig32->TimeDateStamp) },
				{ L"Version", VersionNums_toString(pLoadConfig32->MajorVersion, pLoadConfig32->MinorVersion) },
				{ L"Global Flags Clear", DWORD_toString(pLoadConfig32->GlobalFlagsClear, Hexadecimal) },
				{ L"Global Flags Set", DWORD_toString(pLoadConfig32->GlobalFlagsSet, Hexadecimal) },
				{ L"Critical Section Default Timeout", DWORD_toString(pLoadConfig32->CriticalSectionDefaultTimeout), L"msecs" },
				{
					L"Decommit Free Block Threshold",
					DWORD_toString(pLoadConfig32->DeCommitFreeBlockThreshold),
					FormattedBytesSize(pLoadConfig32->DeCommitFreeBlockThreshold)
				},
				{
					L"Decommit Total Free Threshold",
					DWORD_toString(pLoadConfig32->DeCommitTotalFreeThreshold),
					FormattedBytesSize(pLoadConfig32->DeCommitTotalFreeThreshold)
				},
				{
					L"Lock Prefix Table (VA)",
					DWORD_toString(pLoadConfig32->LockPrefixTable, Hexadecimal)
				},
				{
					L"Maximum Allocation Size",
					DWORD_toString(pLoadConfig32->MaximumAllocationSize),
					FormattedBytesSize(pLoadConfig32->MaximumAllocationSize)
				},
				{
					L"Virtual Memory Threshold",
					DWORD_toString(pLoadConfig32->VirtualMemoryThreshold),
					FormattedBytesSize(pLoadConfig32->VirtualMemoryThreshold)
				},
				{
					L"Process Affinity Mask",
					DWORD_toString(pLoadConfig32->ProcessAffinityMask, Hexadecimal),
					ProcessorAffinityMask_toString(pLoadConfig32->ProcessAffinityMask)
				},
				{
					L"Process Heap Flags",
					DWORD_toString(pLoadConfig32->ProcessHeapFlags, Hexadecimal),
					HeapFlags_toString(pLoadConfig32->ProcessHeapFlags)
				},
				{
					L"CSD Version",
					DWORD_toString(pLoadConfig32->CSDVersion)
				},
				{
					L"Reserved",
					DWORD_toString(pLoadConfig32->Reserved1),
					L"Reserved, must be zero"
				},
				{
					L"Edit List",
					DWORD_toString(pLoadConfig32->EditList),
					L"Reserved, for system"
				},
				{
					L"Ptr to Security Cookie (VA)",
					DWORD_toString(pLoadConfig32->SecurityCookie, Hexadecimal),
					(pLoadConfig32->SecurityCookie ? L'\"' + m_PEReaderWriter.getContainingSectionName(m_PEReaderWriter.VAToRVA(pLoadConfig32->SecurityCookie)) + L"\" Section" : L"")
				}
			};

			if (m_PEReaderWriter.getLoadConfigurationTableSize() > PEReadWrite::LCT32_WITHOUT_SE_HANDLER_SIZE)
			{
				m_LoadConfigItemsInfo.insert(m_LoadConfigItemsInfo.cend(),
				{
					{ L"SEH Table (VA)", DWORD_toString(pLoadConfig32->SEHandlerTable, Hexadecimal), L"See table below" },
					{ L"SEH Count", DWORD_toString(pLoadConfig32->SEHandlerCount) }
				});

				for (DWORD i = 0; i < pLoadConfig32->SEHandlerCount; ++i)
					SEHs.push_back(DWORD_toString(m_PEReaderWriter.getStructuredExceptionHandler(pLoadConfig32, i), Hexadecimal));
			}
		}

		break;

		case PEReadWrite::PEType::PE64:
		{
			RTTI::GetTooltipInfo(m_LoadConfigTooltipInfo, UINT_PTR(NULL), RTTI::RTTI_LOAD_CONFIG64);

			err = m_PEReaderWriter.getLoadConfigurationTable(std::ref(pLoadConfig64));
			if (err)
			{
				LogError(L"ERROR: Load configuration information is not complete. File is not valid.", true);
				return;
			}

			m_LoadConfigItemsInfo =
			{
				{ L"Size", DWORD_toString(pLoadConfig64->Size), FormattedBytesSize(pLoadConfig64->Size) },
				{ L"Time Date Stamp", DWORD_toString(pLoadConfig64->TimeDateStamp), TimeDateStamp_toString(pLoadConfig64->TimeDateStamp) },
				{ L"Version", VersionNums_toString(pLoadConfig64->MajorVersion, pLoadConfig64->MinorVersion) },
				{ L"Global Flags Clear", DWORD_toString(pLoadConfig64->GlobalFlagsClear, Hexadecimal) },
				{ L"Global Flags Set", DWORD_toString(pLoadConfig64->GlobalFlagsSet, Hexadecimal) },
				{ L"Critical Section Default Timeout", DWORD_toString(pLoadConfig64->CriticalSectionDefaultTimeout), L"msecs" },
				{
					L"Decommit Free Block Threshold",
					QWORD_toString(pLoadConfig64->DeCommitFreeBlockThreshold),
					FormattedBytesSize(pLoadConfig64->DeCommitFreeBlockThreshold)
				},
				{
					L"Decommit Total Free Threshold",
					QWORD_toString(pLoadConfig64->DeCommitTotalFreeThreshold),
					FormattedBytesSize(pLoadConfig64->DeCommitTotalFreeThreshold)
				},
				{
					L"Lock Prefix Table (VA)",
					QWORD_toString(pLoadConfig64->LockPrefixTable, Hexadecimal)
				},
				{
					L"Maximum Allocation Size",
					QWORD_toString(pLoadConfig64->MaximumAllocationSize),
					FormattedBytesSize(pLoadConfig64->MaximumAllocationSize)
				},
				{
					L"Virtual Memory Threshold",
					QWORD_toString(pLoadConfig64->VirtualMemoryThreshold),
					FormattedBytesSize(pLoadConfig64->VirtualMemoryThreshold)
				},
				{
					L"Process Affinity Mask",
					QWORD_toString(pLoadConfig64->ProcessAffinityMask, Hexadecimal),
					ProcessorAffinityMask_toString(pLoadConfig64->ProcessAffinityMask)
				},
				{
					L"Process Heap Flags",
					DWORD_toString(pLoadConfig64->ProcessHeapFlags, Hexadecimal),
					HeapFlags_toString(pLoadConfig64->ProcessHeapFlags)
				},
				{
					L"CSD Version",
					DWORD_toString(pLoadConfig64->CSDVersion)
				},
				{
					L"Reserved",
					DWORD_toString(pLoadConfig64->Reserved1), L"Reserved, must be zero"
				},
				{
					L"Edit List",
					QWORD_toString(pLoadConfig64->EditList), L"Reserved, for system"
				},
				{
					L"Ptr to Security Cookie (VA)",
					QWORD_toString(pLoadConfig64->SecurityCookie, Hexadecimal),
					(pLoadConfig64->SecurityCookie ? L'\"' + m_PEReaderWriter.getContainingSectionName(m_PEReaderWriter.VAToRVA(pLoadConfig64->SecurityCookie)) + L"\" Section" : L"")
				}
			};

			if (m_PEReaderWriter.getLoadConfigurationTableSize() > PEReadWrite::LCT64_WITHOUT_SE_HANDLER_SIZE)
			{
				m_LoadConfigItemsInfo.insert(m_LoadConfigItemsInfo.cend(),
				{
					{ L"SEH Table (VA)", QWORD_toString(pLoadConfig64->SEHandlerTable, Hexadecimal), L"See table below" },
					{ L"SEH Count",	QWORD_toString(pLoadConfig64->SEHandlerCount) }
				});

				for (DWORD i = 0; i < pLoadConfig64->SEHandlerCount; ++i)
					SEHs.push_back(QWORD_toString(m_PEReaderWriter.getStructuredExceptionHandler(pLoadConfig64, i), Hexadecimal));
			}
		}
		break;

		default:
			return;
	}

	// Insert ListView columns for 'hListViewLoadConfig'
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(column));

	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szGenericColumnText[i];
		ListView_InsertColumn(m_hListViewLoadConfig, i, &column);
	}

	// Insert ListView columns for 'hListViewSEH'
	static LPWSTR szSEHColumnText[] = { L"Index", L"RVA" };

	for (size_t i = 0; i < ARRAYSIZE(szSEHColumnText); ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szSEHColumnText[i];
		ListView_InsertColumn(m_hListViewSEH, i, &column);
	}

	// Insert ListView items for 'hListViewLoadConfig'
	LV_ITEM item;
	ZeroMemory(&item, sizeof(item));

	for (size_t i = 0; i < m_LoadConfigItemsInfo.size(); ++i)
	{
		item.iItem = int(i);
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = LPWSTR(_wcsdup(m_LoadConfigItemsInfo[i].Text.c_str()));
		ListView_InsertItem(m_hListViewLoadConfig, &item);
		free(item.pszText);

		item.iSubItem = 1;
		item.pszText = LPWSTR(_wcsdup(m_LoadConfigItemsInfo[i].Data.c_str()));
		ListView_SetItem(m_hListViewLoadConfig, &item);
		free(item.pszText);

		item.iSubItem = 2;
		item.pszText = LPWSTR(_wcsdup(m_LoadConfigItemsInfo[i].Comments.c_str()));
		ListView_SetItem(m_hListViewLoadConfig, &item);
		free(item.pszText);
	}

	// Insert ListView items for 'hListViewSEH'
	ZeroMemory(&item, sizeof(item));

	for (size_t i = 0; i < SEHs.size(); ++i)
	{
		item.iItem = int(i);
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = LPWSTR(_wcsdup(DWORD_toString(DWORD(i)).c_str()));
		ListView_InsertItem(m_hListViewSEH, &item);
		free(item.pszText);

		item.iSubItem = 1;
		item.pszText = LPWSTR(SEHs[i].c_str());
		ListView_SetItem(m_hListViewSEH, &item);
	}

	// Resize columns for 'hListViewLoadConfig'
	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
		ListView_SetColumnWidth(m_hListViewLoadConfig, i,
		                        (i == ARRAYSIZE(szGenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));

	// Resize columns for 'hListViewSEH'
	for (size_t i = 0; i < ARRAYSIZE(szSEHColumnText); ++i)
		ListView_SetColumnWidth(m_hListViewSEH, i, LVSCW_AUTOSIZE_USEHEADER);
}

wstring PropertyPageHandler_LoadConfiguration::lstLoadConfig_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(m_LoadConfigTooltipInfo, Index);
}

void PropertyPageHandler_LoadConfiguration::lstLoadConfig_OnContextMenu(LONG x, LONG y, int Index)
{
	Generic_OnContextMenu(m_LoadConfigTooltipInfo, m_LoadConfigItemsInfo, x, y, Index);
}