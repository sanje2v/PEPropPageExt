#include "PropertyPageHandler.h"
#include "CustomCodeViewDefs.h"


#define FillData2(stringobj, label, value)	(stringobj).append(label _T(": ") + tstring(value) + _T('\n'))


PropertyPageHandler_Debug::PropertyPageHandler_Debug(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, PEReaderWriter)
{
	m_hTabsDebugDirs = GetDlgItem(m_hWnd, IDC_TABSDEBUGDIRS);
	m_hListViewDebugDir = GetDlgItem(m_hWnd, IDC_LISTDEBUGDIR);
	m_hEditDebugData = GetDlgItem(m_hWnd, IDC_EDITDEBUGDATA);

	// Get tooltip data
	RTTI::GetTooltipInfo(m_DebugDirTooltipInfo, 0, RTTI::RTTI_DEBUG_DIRECTORY);

	// Setup controls
	m_pLayoutManager->AddChildConstraint(IDC_TABSDEBUGDIRS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_pLayoutManager->AddChildConstraint(IDC_LISTDEBUGDIR, CWA_LEFTRIGHT, CWA_TOP);
	m_pLayoutManager->AddChildConstraint(IDC_EDITDEBUGDATA, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
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
	for (int i = 0; i < m_PEReaderWriter.GetNoOfDebugDirs(); i++)
	{
		tstring temp = _T("Debug Dir ") + DWORD_toString(i + 1);
		item.mask = TCIF_TEXT;
		item.pszText = (LPTSTR) temp.c_str();

		TabCtrl_InsertItem(m_hTabsDebugDirs, i, &item);
	}

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = GenericColumnText[i];
		ListView_InsertColumn(m_hListViewDebugDir, i, &column);
	}

	tabsDebugDirs_OnTabChanged(m_hTabsDebugDirs, 0);
}

tstring PropertyPageHandler_Debug::lstDebugDir_OnGetTooltip(int Index)
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
	m_DebugDirInfo.clear();

	IMAGE_DEBUG_DIRECTORY& DebugDir = *m_PEReaderWriter.GetDebugDirectory(SelectedIndex);

	FillData(m_DebugDirInfo, _T("Characteristics"), DWORD_toString(DebugDir.Characteristics), _T("Reserved, must be zero"));
	FillData(m_DebugDirInfo, _T("Time Date Stamp"), DWORD_toString(DebugDir.TimeDateStamp), TimeDateStamp_toString(DebugDir.TimeDateStamp));
	FillData(m_DebugDirInfo, _T("Version"), VersionNums_toString(DebugDir.MajorVersion, DebugDir.MinorVersion));
	FillData(m_DebugDirInfo, _T("Type"), DWORD_toString(DebugDir.Type, Hexadecimal), DebugType_toString(DebugDir.Type));
	FillData(m_DebugDirInfo, _T("Debug Data Size"), DWORD_toString(DebugDir.SizeOfData), FormattedBytesSize(DebugDir.SizeOfData));
	FillData(m_DebugDirInfo, _T("Raw Data Address (RVA)"), DWORD_toString(DebugDir.AddressOfRawData, Hexadecimal),
				DebugDir.Type == IMAGE_DEBUG_TYPE_MISC ? MultiByte_toString((char *) m_PEReaderWriter.GetVA(DebugDir.AddressOfRawData)) : _T(""));
	FillData(m_DebugDirInfo, _T("Raw Data Offset"), DWORD_toString(DebugDir.PointerToRawData, Hexadecimal));

	tstring buffer;
	void *pDebugData = DebugDir.AddressOfRawData != 0 ? m_PEReaderWriter.GetVA(DebugDir.AddressOfRawData) :
															m_PEReaderWriter.GetVA(DebugDir.PointerToRawData, true);

	switch(DebugDir.Type)
	{
	case IMAGE_DEBUG_TYPE_CODEVIEW:
		{
			// Information from: http://www.debuginfo.com/examples/debugdir.html
			DWORD CvSignature = *(LPDWORD) pDebugData;

			switch (CvSignature)
			{
			case CV_SIGNATURE_NB10:
				{
					CV_INFO_PDB20 *pCvData = (CV_INFO_PDB20 *) pDebugData;

					FillData2(buffer, _T("Signature"), DWORD_toString(pCvData->Header.CvSignature, Hexadecimal) + _T("\t\"NB10\""));
					FillData2(buffer, _T("Offset"), DWORD_toString(pCvData->Header.Offset, Hexadecimal));
					FillData2(buffer, _T("Time date stamp"), DWORD_toString(pCvData->Signature, Hexadecimal) +
																			_T("\t") + TimeDateStamp_toString(pCvData->Signature));
					FillData2(buffer, _T("Age"), DWORD_toString(pCvData->Age));
					FillData2(buffer, _T("Pdb filename"), MultiByte_toString((char *) pCvData->PdbFileName));
				}

				break;

			case CV_SIGNATURE_RSDS:
				{
					CV_INFO_PDB70 *pCvData = (CV_INFO_PDB70 *) pDebugData;

					FillData2(buffer, _T("Signature"), DWORD_toString(pCvData->CvSignature, Hexadecimal) + _T("\t\"RSDS\""));
					FillData2(buffer, _T("GUID"), GUID_toString(pCvData->Signature));
					FillData2(buffer, _T("Age"), DWORD_toString(pCvData->Age));
					FillData2(buffer, _T("Pdb filename"), MultiByte_toString((char *) pCvData->PdbFileName));
				}

				break;

			default:
				{
					CHAR *pData = (CHAR*) &CvSignature;

					FillData2(buffer, _T("Signature"), Signature_toString(*(LPDWORD) pData));

					if ((pData[0] == 'N') && (pData[1] == 'B')) // One of NBxx formats 
					{
						CV_HEADER* pCvHeader = (CV_HEADER*) pDebugData; 

						FillData2(buffer, _T("CodeView information offset"), DWORD_toString(pCvHeader->Offset)); 
					}
				}
			}
		}

		break;

	case IMAGE_DEBUG_TYPE_FPO:
		{
			PFPO_DATA pFPOData = (PFPO_DATA) pDebugData;

			FillData2(buffer, _T("Function code offset"), DWORD_toString(pFPOData->ulOffStart, Hexadecimal));
			FillData2(buffer, _T("Size of function"), DWORD_toString(pFPOData->cbProcSize) + _T(" ") + FormattedBytesSize(pFPOData->cbProcSize));
			FillData2(buffer, _T("Size of locals"), DWORD_toString(pFPOData->cdwLocals) + _T(" ") + FormattedBytesSize(pFPOData->cdwLocals));
			FillData2(buffer, _T("Size of params"), DWORD_toString(pFPOData->cdwParams) + _T(" ") + FormattedBytesSize(pFPOData->cdwParams));
			FillData2(buffer, _T("Size of prolog"), DWORD_toString(pFPOData->cbProlog) + _T(" ") + FormattedBytesSize(pFPOData->cbProlog));
			FillData2(buffer, _T("No. of saved registers"), DWORD_toString(pFPOData->cbRegs) + _T(" ") + DWORD_toString(pFPOData->cbRegs));
			FillData2(buffer, _T("Has SEH"), DWORD_toString(pFPOData->fHasSEH) + (pFPOData->fHasSEH ? _T(", Yes") : _T(", No")));
			FillData2(buffer, _T("EBP Allocated"), DWORD_toString(pFPOData->fUseBP) + (pFPOData->fUseBP ? _T(", Yes") : _T(", No")));
			FillData2(buffer, _T("Reserved"), DWORD_toString(pFPOData->reserved));

			switch (pFPOData->cbFrame)
			{
			case FRAME_FPO:
				FillData2(buffer, _T("Frame type"), _T("Frame ptr ommission"));

				break;

			case FRAME_TRAP:
				FillData2(buffer, _T("Frame type"), _T("Trap"));

				break;

			case FRAME_TSS:
				FillData2(buffer, _T("Frame type"), _T("Tss"));

				break;

			case FRAME_NONFPO:
				FillData2(buffer, _T("Frame type"), _T("No Frame ptr ommission"));
			}
		}

		break;

	case IMAGE_DEBUG_TYPE_MISC:
		{
			PIMAGE_DEBUG_MISC pMiscData = (PIMAGE_DEBUG_MISC) pDebugData;

			FillData2(buffer, _T("Data type"), DWORD_toString(pMiscData->DataType) + _T("\t") + DebugMiscDataType_toString(pMiscData->DataType));
			FillData2(buffer, _T("Length"), DWORD_toString(pMiscData->Length) + _T("\t") + FormattedBytesSize(pMiscData->Length) + _T(", aligned to 4 bytes"));
			FillData2(buffer, _T("Is UNICODE"), DWORD_toString(pMiscData->Unicode));
			FillData2(buffer, _T("Reserved"), DWORD_toString(pMiscData->Reserved[0]));
			FillData2(buffer, _T("Reserved"), DWORD_toString(pMiscData->Reserved[1]));
			FillData2(buffer, _T("Reserved"), DWORD_toString(pMiscData->Reserved[2]));
			FillData2(buffer, _T("Data"), MultiByte_toString((char *) pMiscData->Data, pMiscData->Unicode != 1));
		}

		break;

	default:
		buffer += _T("The debug type in header has no publicized definitions.");
	}

	Edit_SetText(m_hEditDebugData, (LPTSTR) buffer.c_str());

	// Insert ListView items for Debug Directory view
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < m_DebugDirInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) m_DebugDirInfo[i].szText.c_str();
		ListView_InsertItem(m_hListViewDebugDir, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) m_DebugDirInfo[i].szData.c_str();
		ListView_SetItem(m_hListViewDebugDir, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) m_DebugDirInfo[i].szComments.c_str();
		ListView_SetItem(m_hListViewDebugDir, &item);
	}

	// Resize columns
	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
		ListView_SetColumnWidth(m_hListViewDebugDir, i,
											i == GetArraySize(GenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);
}