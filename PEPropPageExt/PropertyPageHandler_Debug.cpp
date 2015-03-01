#include "stdafx.h"
#include "PropertyPageHandler.h"
#include "CustomCodeViewDefs.h"


PropertyPageHandler_Debug::PropertyPageHandler_Debug(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, std::ref(PEReaderWriter))
{
	m_hTabsDebugDirs = GetDlgItem(m_hWnd, IDC_TABSDEBUGDIRS);
	m_hListViewDebugDir = GetDlgItem(m_hWnd, IDC_LISTDEBUGDIR);
	m_hEditDebugData = GetDlgItem(m_hWnd, IDC_EDITDEBUGDATA);

	// Get tooltip data
	RTTI::GetTooltipInfo(m_DebugDirTooltipInfo, 0, RTTI::RTTI_DEBUG_DIRECTORY);

	// Setup controls
	m_LayoutManager.AddChildConstraint(IDC_TABSDEBUGDIRS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_LayoutManager.AddChildConstraint(IDC_LISTDEBUGDIR, CWA_LEFTRIGHT, CWA_TOP);
	m_LayoutManager.AddChildConstraint(IDC_EDITDEBUGDATA, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	SetWindowPos(m_hTabsDebugDirs, m_hListViewDebugDir, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(m_hTabsDebugDirs, m_hEditDebugData, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	// Set full row selection style for list view
	ListView_SetExtendedListViewStyleEx(m_hListViewDebugDir,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);
}

void PropertyPageHandler_Debug::OnInitDialog()
{
	// Insert Tabs
	TCITEM item;

	for (int i = 0; i < m_PEReaderWriter.getNoOfDebugDirs(); ++i)
	{
		item.mask = TCIF_TEXT;
		item.pszText = _wcsdup(wstring(L"Debug Dir " + DWORD_toString(i + 1)).c_str());
		TabCtrl_InsertItem(m_hTabsDebugDirs, i, &item);
		free(item.pszText);
	}

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szGenericColumnText[i];
		ListView_InsertColumn(m_hListViewDebugDir, i, &column);
	}

	tabsDebugDirs_OnTabChanged(m_hTabsDebugDirs, 0);
}

wstring PropertyPageHandler_Debug::lstDebugDir_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(m_DebugDirTooltipInfo, Index);
}

void PropertyPageHandler_Debug::lstDebugDir_OnContextMenu(LONG x, LONG y, int Index)
{
	Generic_OnContextMenu(m_DebugDirTooltipInfo, m_DebugDirInfo, x, y, Index);
}

void PropertyPageHandler_Debug::tabsDebugDirs_OnTabChanged(HWND hControl, int SelectedIndex)
{
	// Delete all items in ListViewImportsFuncsAndDirTable
	ListView_DeleteAllItems(m_hListViewDebugDir);

	// Fill control with data
	PIMAGE_DEBUG_DIRECTORY pDebugDir;
	int err = m_PEReaderWriter.getDebugDirectory(SelectedIndex, std::ref(pDebugDir));
	if (err)
	{
		LogError(L"ERROR: Couldn't read 'IMAGE_DEBUG_DIRECTORY'. File's debugging information is missing.", true);
		return;
	}

	m_DebugDirInfo =
	{
		{ L"Characteristics", DWORD_toString(pDebugDir->Characteristics), L"Reserved, must be zero" },
		{ L"Time Date Stamp", DWORD_toString(pDebugDir->TimeDateStamp), TimeDateStamp_toString(pDebugDir->TimeDateStamp) },
		{ L"Version", VersionNums_toString(pDebugDir->MajorVersion, pDebugDir->MinorVersion) },
		{ L"Type", DWORD_toString(pDebugDir->Type, Hexadecimal), DebugType_toString(pDebugDir->Type) },
		{ L"Debug Data Size", DWORD_toString(pDebugDir->SizeOfData), FormattedBytesSize(pDebugDir->SizeOfData) },
		{
			L"Raw Data Addr (RVA)",
			DWORD_toString(pDebugDir->AddressOfRawData, Hexadecimal),
			m_PEReaderWriter.getDebugFilename(std::cref(pDebugDir))
		},
		{ L"Raw Data Offset", DWORD_toString(pDebugDir->PointerToRawData, Hexadecimal) }
	};

	wstring DebugDataInfo;

	static auto funcMakeDebugDesc = [](wstring& DebugDataInfo, wstring label, wstring value) -> void
	{
		DebugDataInfo.append(label + L": " + value + L'\n');
	};

	switch (pDebugDir->Type)
	{
		case IMAGE_DEBUG_TYPE_CODEVIEW:
		{
			// Information from: http://www.debuginfo.com/examples/debugdir.html
			LPDWORD pCvSignature;
			err = m_PEReaderWriter.getDebugData(std::cref(pDebugDir), std::ref(pCvSignature));
			if (err)
			{
				LogError(L"WARNING: Signature for CodeView debug type at index " + DWORD_toString(SelectedIndex) + L" is missing. This debug information is not valid.", true);
				break;
			}

			switch (*pCvSignature)
			{
				case CV_SIGNATURE_NB10:
				{
					CV_INFO_PDB20 *pCvData;
					err = m_PEReaderWriter.getDebugData(std::cref(pDebugDir), std::ref(pCvData));
					if (err)
					{
						LogError(L"WARNING: Debug data at index " + DWORD_toString(SelectedIndex) + L" for 'CodeView->NB10' is incomplete. This debug information is not valid.", true);
						break;
					}

					funcMakeDebugDesc(DebugDataInfo, L"Signature", DWORD_toString(pCvData->Header.CvSignature, Hexadecimal) + L"\t\"NB10\"");
					funcMakeDebugDesc(DebugDataInfo, L"Offset", DWORD_toString(pCvData->Header.Offset, Hexadecimal));
					funcMakeDebugDesc(DebugDataInfo, L"Time date stamp", DWORD_toString(pCvData->Signature, Hexadecimal) +
																				L'\t' + TimeDateStamp_toString(pCvData->Signature));
					funcMakeDebugDesc(DebugDataInfo, L"Age", DWORD_toString(pCvData->Age));
					funcMakeDebugDesc(DebugDataInfo, L"Pdb filename", MultiByte_toString(reinterpret_cast<char *>(pCvData->PdbFileName)));
				}
				break;
				
				case CV_SIGNATURE_RSDS:
				{
					CV_INFO_PDB70 *pCvData;
					err = m_PEReaderWriter.getDebugData(std::cref(pDebugDir), std::ref(pCvData));
					if (err)
					{
						LogError(L"WARNING: Debug data at index " + DWORD_toString(SelectedIndex) + L" for 'CodeView->RSDS' is incomplete. This debug information is not valid.", true);
						break;
					}

					funcMakeDebugDesc(DebugDataInfo, L"Signature", DWORD_toString(pCvData->CvSignature, Hexadecimal) + L"\t\"RSDS\"");
					funcMakeDebugDesc(DebugDataInfo, L"GUID", GUID_toString(pCvData->Signature));
					funcMakeDebugDesc(DebugDataInfo, L"Age", DWORD_toString(pCvData->Age));
					funcMakeDebugDesc(DebugDataInfo, L"Pdb filename", MultiByte_toString(reinterpret_cast<char *>(pCvData->PdbFileName)));
				}
				break;
				
				default:
				{
					funcMakeDebugDesc(DebugDataInfo, L"Signature", Signature_toString(*pCvSignature));

					if ((reinterpret_cast<CHAR *> (pCvSignature)[0] == 'N') &&
						(reinterpret_cast<CHAR *> (pCvSignature)[1] == 'B')) // One of NBxx formats 
					{
						CV_HEADER* pCvHeader;
						err = m_PEReaderWriter.getDebugData(std::cref(pDebugDir), std::ref(pCvHeader));
						if (err)
						{
							LogError(L"WARNING: Debug data at index " + DWORD_toString(SelectedIndex) + L" for 'CV_HEADER' is incomplete. This debug information is not valid.", true);
							break;
						}

						funcMakeDebugDesc(DebugDataInfo, L"CodeView information offset", DWORD_toString(pCvHeader->Offset));
					}
				}
				break;
			}
		}
		break;
		
		case IMAGE_DEBUG_TYPE_FPO:
		{
			PFPO_DATA pFPOData;
			err = m_PEReaderWriter.getDebugData(std::cref(pDebugDir), std::ref(pFPOData));
			if (err)
			{
				LogError(L"WARNING: Debug data at index " + DWORD_toString(SelectedIndex) + L" for 'FPO_DATA' is missing. This debug information is not valid.", true);
				break;
			}

			funcMakeDebugDesc(DebugDataInfo, L"Function code offset", DWORD_toString(pFPOData->ulOffStart, Hexadecimal));
			funcMakeDebugDesc(DebugDataInfo, L"Size of function", DWORD_toString(pFPOData->cbProcSize) + L" " + FormattedBytesSize(pFPOData->cbProcSize));
			funcMakeDebugDesc(DebugDataInfo, L"Size of locals", DWORD_toString(pFPOData->cdwLocals) + L" " + FormattedBytesSize(pFPOData->cdwLocals));
			funcMakeDebugDesc(DebugDataInfo, L"Size of params", DWORD_toString(pFPOData->cdwParams) + L" " + FormattedBytesSize(pFPOData->cdwParams));
			funcMakeDebugDesc(DebugDataInfo, L"Size of prolog", DWORD_toString(pFPOData->cbProlog) + L" " + FormattedBytesSize(pFPOData->cbProlog));
			funcMakeDebugDesc(DebugDataInfo, L"No. of saved registers", DWORD_toString(pFPOData->cbRegs) + L" " + DWORD_toString(pFPOData->cbRegs));
			funcMakeDebugDesc(DebugDataInfo, L"Has SEH", DWORD_toString(pFPOData->fHasSEH) + (pFPOData->fHasSEH ? L", Yes" : L", No"));
			funcMakeDebugDesc(DebugDataInfo, L"EBP Allocated", DWORD_toString(pFPOData->fUseBP) + (pFPOData->fUseBP ? L", Yes" : L", No"));
			funcMakeDebugDesc(DebugDataInfo, L"Reserved", DWORD_toString(pFPOData->reserved));

			switch (pFPOData->cbFrame)
			{
				case FRAME_FPO:
					funcMakeDebugDesc(DebugDataInfo, L"Frame type", L"Frame ptr ommission");
					break;
				
				case FRAME_TRAP:
					funcMakeDebugDesc(DebugDataInfo, L"Frame type", L"Trap");
					break;
				
				case FRAME_TSS:
					funcMakeDebugDesc(DebugDataInfo, L"Frame type", L"Tss");
					break;
				
				case FRAME_NONFPO:
					funcMakeDebugDesc(DebugDataInfo, L"Frame type", L"No Frame ptr ommission");
					break;
			}
		}
		break;

	case IMAGE_DEBUG_TYPE_MISC:
		{
			PIMAGE_DEBUG_MISC pMiscData;
			err = m_PEReaderWriter.getDebugData(std::cref(pDebugDir), std::ref(pMiscData));
			if (err)
			{
				LogError(L"WARNING: Debug data at index " + DWORD_toString(SelectedIndex) + L" for 'IMAGE_DEBUG_MISC' is missing. This debug information is not valid.", true);
				break;
			}

			funcMakeDebugDesc(DebugDataInfo, L"Data type", DWORD_toString(pMiscData->DataType) + L'\t' + DebugMiscDataType_toString(pMiscData->DataType));
			funcMakeDebugDesc(DebugDataInfo, L"Length", DWORD_toString(pMiscData->Length) + L'\t' + FormattedBytesSize(pMiscData->Length) + L", aligned to 4 bytes");
			funcMakeDebugDesc(DebugDataInfo, L"Is UNICODE", DWORD_toString(pMiscData->Unicode));
			funcMakeDebugDesc(DebugDataInfo, L"Reserved", DWORD_toString(pMiscData->Reserved[0]));
			funcMakeDebugDesc(DebugDataInfo, L"Reserved", DWORD_toString(pMiscData->Reserved[1]));
			funcMakeDebugDesc(DebugDataInfo, L"Reserved", DWORD_toString(pMiscData->Reserved[2]));
			funcMakeDebugDesc(DebugDataInfo, L"Data", MultiByte_toString(reinterpret_cast<char *>(pMiscData->Data), pMiscData->Unicode != 1));
		}
		break;

	default:
		DebugDataInfo += L"This debug type has no publicized definitions.";
		break;
	}

	Edit_SetText(m_hEditDebugData, DebugDataInfo.c_str());

	// Insert ListView items for Debug Directory view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (size_t i = 0; i < m_DebugDirInfo.size(); ++i)
	{
		item.iItem = int(i);
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = LPWSTR(_wcsdup(m_DebugDirInfo[i].Text.c_str()));
		ListView_InsertItem(m_hListViewDebugDir, &item);
		free(item.pszText);

		item.iSubItem = 1;
		item.pszText = LPWSTR(_wcsdup(m_DebugDirInfo[i].Data.c_str()));
		ListView_SetItem(m_hListViewDebugDir, &item);
		free(item.pszText);

		item.iSubItem = 2;
		item.pszText = LPWSTR(_wcsdup(m_DebugDirInfo[i].Comments.c_str()));
		ListView_SetItem(m_hListViewDebugDir, &item);
		free(item.pszText);
	}

	// Resize columns
	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
		ListView_SetColumnWidth(m_hListViewDebugDir,
								i,
								(i == ARRAYSIZE(szGenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));
}