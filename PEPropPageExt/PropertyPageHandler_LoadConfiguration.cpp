#include "PropertyPageHandler.h"


void PropertyPageHandler_LoadConfiguration::OnInitDialog()
{
	hListViewLoadConfig = GetDlgItem(m_hWnd, IDC_LISTLOADCONFIG);
	hListViewSEH = GetDlgItem(m_hWnd, IDC_LISTSEH);

	// Setup controls with layout manager
	m_pLayoutManager->AddChildConstraint(IDC_LISTLOADCONFIG, CWA_LEFTRIGHT, CWA_TOP);
	m_pLayoutManager->AddChildConstraint(IDC_LISTSEH, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Set full row selection style for list views
	ListView_SetExtendedListViewStyleEx(hListViewLoadConfig,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);
	ListView_SetExtendedListViewStyleEx(hListViewSEH, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	// Fill them with data
	vector<tstring> SEHs;

	PIMAGE_LOAD_CONFIG_DIRECTORY pLoadConfig = m_PEReaderWriter.GetLoadConfigurationTable<PIMAGE_LOAD_CONFIG_DIRECTORY>();

	FillData(LoadConfigItemsInfo, _T("Size"), DWORD_toString(pLoadConfig->Size), FormattedBytesSize(pLoadConfig->Size));
	FillData(LoadConfigItemsInfo, _T("Time Date Stamp"), DWORD_toString(pLoadConfig->TimeDateStamp),
																							TimeDateStamp_toString(pLoadConfig->TimeDateStamp));
	FillData(LoadConfigItemsInfo, _T("Version"), VersionNums_toString(pLoadConfig->MajorVersion, pLoadConfig->MinorVersion));
	FillData(LoadConfigItemsInfo, _T("Global Flags Clear"), DWORD_toString(pLoadConfig->GlobalFlagsClear, Hexadecimal));
	FillData(LoadConfigItemsInfo, _T("Global Flags Set"), DWORD_toString(pLoadConfig->GlobalFlagsSet, Hexadecimal));
	FillData(LoadConfigItemsInfo, _T("Critical Section Default Timeout"), DWORD_toString(pLoadConfig->CriticalSectionDefaultTimeout), _T("msecs"));

	switch (m_PEReaderWriter.GetPEType())
	{
	case PEReadWrite::PE32:
		{
			RTTI::GetTooltipInfo(LoadConfigTooltipInfo, 0, RTTI::RTTI_LOAD_CONFIG32);

			PIMAGE_NT_HEADERS32 pNTheaders32 = m_PEReaderWriter.GetSecondaryHeader<PIMAGE_NT_HEADERS32>();
			PIMAGE_LOAD_CONFIG_DIRECTORY32 pLoadConfig32 = m_PEReaderWriter.GetLoadConfigurationTable<PIMAGE_LOAD_CONFIG_DIRECTORY32>();

			FillData(LoadConfigItemsInfo, _T("Decommit Free Block Threshold"), DWORD_toString(pLoadConfig32->DeCommitFreeBlockThreshold),
																					FormattedBytesSize(pLoadConfig32->DeCommitFreeBlockThreshold));
			FillData(LoadConfigItemsInfo, _T("Decommit Total Free Threshold"), DWORD_toString(pLoadConfig32->DeCommitTotalFreeThreshold),
																					FormattedBytesSize(pLoadConfig32->DeCommitTotalFreeThreshold));
			FillData(LoadConfigItemsInfo, _T("Lock Prefix Table (VA)"), DWORD_toString(pLoadConfig32->LockPrefixTable, Hexadecimal));
			FillData(LoadConfigItemsInfo, _T("Maximum Allocation Size"), DWORD_toString(pLoadConfig32->MaximumAllocationSize),
																						FormattedBytesSize(pLoadConfig32->MaximumAllocationSize));
			FillData(LoadConfigItemsInfo, _T("Virtual Memory Threshold"), DWORD_toString(pLoadConfig32->VirtualMemoryThreshold),
																						FormattedBytesSize(pLoadConfig32->VirtualMemoryThreshold));
			FillData(LoadConfigItemsInfo, _T("Process Affinity Mask"), DWORD_toString(pLoadConfig32->ProcessAffinityMask, Hexadecimal),
																				ProcessorAffinityMask_toString(pLoadConfig32->ProcessAffinityMask));
			FillData(LoadConfigItemsInfo, _T("Process Heap Flags"), DWORD_toString(pLoadConfig32->ProcessHeapFlags, Hexadecimal),
																							HeapFlags_toString(pLoadConfig32->ProcessHeapFlags));
			FillData(LoadConfigItemsInfo, _T("CSD Version"), DWORD_toString(pLoadConfig32->CSDVersion));
			FillData(LoadConfigItemsInfo, _T("Reserved"), DWORD_toString(pLoadConfig32->Reserved1), _T("Reserved, must be zero"));
			FillData(LoadConfigItemsInfo, _T("Edit List"), DWORD_toString(pLoadConfig32->EditList), _T("Reserved, for use by system"));
			FillData(LoadConfigItemsInfo, _T("Ptr to Security Cookie (VA)"), DWORD_toString(pLoadConfig32->SecurityCookie, Hexadecimal),
																																_T("Value: ") +
																					DWORD_toString(*((LPDWORD) m_PEReaderWriter.GetVA(
														pLoadConfig32->SecurityCookie - pNTheaders32->OptionalHeader.ImageBase)), Hexadecimal));

			if (m_PEReaderWriter.GetLoadConfigurationTableSize() > WITHOUT_SE_HANDLER32_SIZE)
			{
				FillData(LoadConfigItemsInfo, _T("SEH Table (VA)"), DWORD_toString(pLoadConfig32->SEHandlerTable, Hexadecimal));
				FillData(LoadConfigItemsInfo, _T("SEH Count"), DWORD_toString(pLoadConfig32->SEHandlerCount));

				for (unsigned int i = 0; i < pLoadConfig32->SEHandlerCount; i++)
					SEHs.push_back(DWORD_toString(((LPDWORD) m_PEReaderWriter.GetVA(
														pLoadConfig32->SEHandlerTable - pNTheaders32->OptionalHeader.ImageBase))[i], Hexadecimal));
			}
		}

		break;

	case PEReadWrite::PE64:
		{
			RTTI::GetTooltipInfo(LoadConfigTooltipInfo, 0, RTTI::RTTI_LOAD_CONFIG64);

			PIMAGE_NT_HEADERS64 pNTheaders64 = m_PEReaderWriter.GetSecondaryHeader<PIMAGE_NT_HEADERS64>();
			PIMAGE_LOAD_CONFIG_DIRECTORY64 pLoadConfig64 = m_PEReaderWriter.GetLoadConfigurationTable<PIMAGE_LOAD_CONFIG_DIRECTORY64>();

			FillData(LoadConfigItemsInfo, _T("Decommit Free Block Threshold"), QWORD_toString(pLoadConfig64->DeCommitFreeBlockThreshold),
																					FormattedBytesSize(pLoadConfig64->DeCommitFreeBlockThreshold));
			FillData(LoadConfigItemsInfo, _T("Decommit Total Free Threshold"), QWORD_toString(pLoadConfig64->DeCommitTotalFreeThreshold),
																					FormattedBytesSize(pLoadConfig64->DeCommitTotalFreeThreshold));
			FillData(LoadConfigItemsInfo, _T("Lock Prefix Table (VA)"), QWORD_toString(pLoadConfig64->LockPrefixTable, Hexadecimal));
			FillData(LoadConfigItemsInfo, _T("Maximum Allocation Size"), QWORD_toString(pLoadConfig64->MaximumAllocationSize),
																						FormattedBytesSize(pLoadConfig64->MaximumAllocationSize));
			FillData(LoadConfigItemsInfo, _T("Virtual Memory Threshold"), QWORD_toString(pLoadConfig64->VirtualMemoryThreshold),
																						FormattedBytesSize(pLoadConfig64->VirtualMemoryThreshold));
			FillData(LoadConfigItemsInfo, _T("Process Affinity Mask"), QWORD_toString(pLoadConfig64->ProcessAffinityMask, Hexadecimal),
																			ProcessorAffinityMask_toString(pLoadConfig64->ProcessAffinityMask));
			FillData(LoadConfigItemsInfo, _T("Process Heap Flags"), DWORD_toString(pLoadConfig64->ProcessHeapFlags, Hexadecimal),
																							HeapFlags_toString(pLoadConfig64->ProcessHeapFlags));
			FillData(LoadConfigItemsInfo, _T("CSD Version"), DWORD_toString(pLoadConfig64->CSDVersion));
			FillData(LoadConfigItemsInfo, _T("Reserved"), DWORD_toString(pLoadConfig64->Reserved1), _T("Reserved, must be zero"));
			FillData(LoadConfigItemsInfo, _T("Edit List"), QWORD_toString(pLoadConfig64->EditList), _T("Reserved, for use by system"));
			FillData(LoadConfigItemsInfo, _T("Ptr to Security Cookie (VA)"), QWORD_toString(pLoadConfig64->SecurityCookie, Hexadecimal),
																																_T("Value: ") +
																							QWORD_toString((ULONGLONG) m_PEReaderWriter.GetVA(
												(DWORD) (pLoadConfig64->SecurityCookie - pNTheaders64->OptionalHeader.ImageBase)), Hexadecimal));

			if (m_PEReaderWriter.GetLoadConfigurationTableSize() > WITHOUT_SE_HANDLER64_SIZE)
			{
				FillData(LoadConfigItemsInfo, _T("SEH Table (VA)"), QWORD_toString(pLoadConfig64->SEHandlerTable, Hexadecimal),
																										_T("Virtual Address, see table below"));
				FillData(LoadConfigItemsInfo, _T("SEH Count"), QWORD_toString(pLoadConfig64->SEHandlerCount));

				for (unsigned int i = 0; i < pLoadConfig64->SEHandlerCount; i++)
					SEHs.push_back(DWORD_toString(((LPDWORD) m_PEReaderWriter.GetVA(
											(DWORD) (pLoadConfig64->SEHandlerTable - pNTheaders64->OptionalHeader.ImageBase)))[i], Hexadecimal));
			}
		}
	}

	// Insert ListView columns for 'hListViewLoadConfig'
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = GenericColumnText[i];
		ListView_InsertColumn(hListViewLoadConfig, i, &column);
	}

	// Insert ListView columns for 'hListViewSEH'
	static LPTSTR SEHColumnText[] = {_T("Index"), _T("RVA")};

	for (unsigned int i = 0; i < GetArraySize(SEHColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = SEHColumnText[i];
		ListView_InsertColumn(hListViewSEH, i, &column);
	}

	// Insert ListView items for 'hListViewLoadConfig'
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < LoadConfigItemsInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) LoadConfigItemsInfo[i].szText.c_str();
		ListView_InsertItem(hListViewLoadConfig, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) LoadConfigItemsInfo[i].szData.c_str();
		ListView_SetItem(hListViewLoadConfig, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) LoadConfigItemsInfo[i].szComments.c_str();
		ListView_SetItem(hListViewLoadConfig, &item);
	}

	// Insert ListView items for 'hListViewSEH'
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < SEHs.size(); i++)
	{
		tstring dummy;

		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		dummy = DWORD_toString(i).c_str();
		item.pszText = (LPTSTR) dummy.c_str();
		ListView_InsertItem(hListViewSEH, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) SEHs[i].c_str();
		ListView_SetItem(hListViewSEH, &item);
	}

	// Resize columns for 'hListViewLoadConfig'
	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
		ListView_SetColumnWidth(hListViewLoadConfig, i,
								i == GetArraySize(GenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);

	// Resize columns for 'hListViewSEH'
	for (unsigned int i = 0; i < GetArraySize(SEHColumnText); i++)
		ListView_SetColumnWidth(hListViewSEH, i, LVSCW_AUTOSIZE_USEHEADER);
}

tstring PropertyPageHandler_LoadConfiguration::lstLoadConfig_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(LoadConfigTooltipInfo, Index);
}

void PropertyPageHandler_LoadConfiguration::lstLoadConfig_OnContextMenu(LONG x, LONG y, int Index)
{
	Generic_OnContextMenu(LoadConfigTooltipInfo, LoadConfigItemsInfo, x, y, Index);
}