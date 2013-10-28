#include "PropertyPageHandler.h"
#include "CustomCorDefs.h"
#include <algorithm>


PropertyPageHandler_CLR::PropertyPageHandler_CLR(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter)
{
	m_hTabsCLRData = GetDlgItem(m_hWnd, IDC_TABSCLRDATA);
	m_hListViewCLRData = GetDlgItem(m_hWnd, IDC_LISTCLRDATA);
	m_hEditCLRData = GetDlgItem(m_hWnd, IDC_EDITCLRDATA);

	// Setup controls with layout manager
	m_pLayoutManager->AddChildConstraint(IDC_TABSCLRDATA, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_pLayoutManager->AddChildConstraint(IDC_LISTCLRDATA, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_pLayoutManager->AddChildConstraint(IDC_EDITCLRDATA, CWA_LEFTRIGHT, CWA_BOTTOM);
	// Make ZOrder: List view (On top) and Tab Control (On bottom)
	SetWindowPos(m_hTabsCLRData, m_hListViewCLRData, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	SetWindowPos(m_hTabsCLRData, m_hEditCLRData, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	// NOTE: ZOrder for 'hEditCLRData' is moved to 'tabsCLRData_OnTabChanged' due to repaint issue
	
	// Set full row selection style for list views
	ListView_SetExtendedListViewStyleEx(m_hListViewCLRData,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);
	
	// Set tab stop for rich edit
	DWORD cTabs = 85;
	Edit_SetTabStops(m_hEditCLRData, 1, &cTabs);
}

void PropertyPageHandler_CLR::OnInitDialog()
{
	// Insert ListView columns for 'hListViewCLRData'
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = GenericColumnText[i];
		ListView_InsertColumn(m_hListViewCLRData, i, &column);
	}

	// Insert Tabs
	static LPTSTR SEHColumnText[] = {_T("CLR Header"),
										_T("Metadata"),
										_T("Resources"),
										_T("Strong Name Signature"),
										_T("VTable Fixup Directory")};
	TCITEM item;
	ZeroMemory(&item, sizeof(TCITEM));

    for (int i = 0; i < GetArraySize(SEHColumnText); i++)
    {
        item.mask = TCIF_TEXT;
        item.pszText = (LPTSTR) SEHColumnText[i];

        TabCtrl_InsertItem(m_hTabsCLRData, i, &item);
    }

	// Select the first tab
	tabsCLRData_OnTabChanged(m_hListViewCLRData, 0);
}

void PropertyPageHandler_CLR::tabsCLRData_OnTabChanged(HWND hControl, int SelectedIndex)
{
	vector<TextAndData> CLRDataItemsInfo;
	PIMAGE_COR20_HEADER pCLRData = m_PEReaderWriter.GetCLRHeader();

	ListView_DeleteAllItems(m_hListViewCLRData);

	switch (SelectedIndex)
	{
	case 0:	// CLR Headers
		{
			// Fill them with data
			FillData(CLRDataItemsInfo, _T("Size"), DWORD_toString(pCLRData->cb), FormattedBytesSize(pCLRData->cb));
			FillData(CLRDataItemsInfo, _T("Runtime Version"), VersionNums_toString(pCLRData->MajorRuntimeVersion, pCLRData->MinorRuntimeVersion));
			FillData(CLRDataItemsInfo, _T("Metadata Address"), DWORD_toString(pCLRData->MetaData.VirtualAddress, Hexadecimal));
			FillData(CLRDataItemsInfo, _T("Metadata Size"), DWORD_toString(pCLRData->MetaData.Size), FormattedBytesSize(pCLRData->MetaData.Size));
			FillData(CLRDataItemsInfo, _T("Flags"), DWORD_toString(pCLRData->Flags, Hexadecimal), CorImageFlags_toString(pCLRData->Flags));
			FillData(CLRDataItemsInfo, _T("Entry Point"), DWORD_toString(pCLRData->EntryPointRVA, Hexadecimal), (TestFlag(pCLRData->Flags, COMIMAGE_FLAGS_NATIVE_ENTRYPOINT) ?
																													_T("Native RVA entry point") : _T("Managed token entry point")));
			FillData(CLRDataItemsInfo, _T("Resources Address"), DWORD_toString(pCLRData->Resources.VirtualAddress, Hexadecimal));
			FillData(CLRDataItemsInfo, _T("Resources Size"), DWORD_toString(pCLRData->Resources.Size), FormattedBytesSize(pCLRData->Resources.Size));
			FillData(CLRDataItemsInfo, _T("Strong Name Address"), DWORD_toString(pCLRData->StrongNameSignature.VirtualAddress, Hexadecimal));
			FillData(CLRDataItemsInfo, _T("Strong Name Size"), DWORD_toString(pCLRData->StrongNameSignature.Size), FormattedBytesSize(pCLRData->StrongNameSignature.Size));
			FillData(CLRDataItemsInfo, _T("Code Manager Address"), DWORD_toString(pCLRData->CodeManagerTable.VirtualAddress, Hexadecimal), _T("Deprecated"));
			FillData(CLRDataItemsInfo, _T("Code Manager Size"), DWORD_toString(pCLRData->CodeManagerTable.Size), FormattedBytesSize(pCLRData->CodeManagerTable.Size));
			FillData(CLRDataItemsInfo, _T("VTable Fixups Address"), DWORD_toString(pCLRData->VTableFixups.VirtualAddress, Hexadecimal));
			FillData(CLRDataItemsInfo, _T("VTable Fixups Size"), DWORD_toString(pCLRData->VTableFixups.Size), FormattedBytesSize(pCLRData->VTableFixups.Size));
			FillData(CLRDataItemsInfo, _T("Export Table Address"), DWORD_toString(pCLRData->ExportAddressTableJumps.VirtualAddress, Hexadecimal));
			FillData(CLRDataItemsInfo, _T("Export Table Size"), DWORD_toString(pCLRData->ExportAddressTableJumps.Size), FormattedBytesSize(pCLRData->ExportAddressTableJumps.Size));
			FillData(CLRDataItemsInfo, _T("Native Header Address"), DWORD_toString(pCLRData->ManagedNativeHeader.VirtualAddress, Hexadecimal));
			FillData(CLRDataItemsInfo, _T("Native Header Size"), DWORD_toString(pCLRData->ManagedNativeHeader.Size), FormattedBytesSize(pCLRData->ManagedNativeHeader.Size));
		}

		break;

	case 1:	// Metadata
		{
			PMETA_DATA_SECTION_HEADER1 pMetaData1 = (PMETA_DATA_SECTION_HEADER1) m_PEReaderWriter.GetVA(pCLRData->MetaData.VirtualAddress);
			PMETA_DATA_SECTION_HEADER2 pMetaData2 = (PMETA_DATA_SECTION_HEADER2) (((UINT_PTR) &pMetaData1->Name) + pMetaData1->Length);

			// Fill with data
			FillData(CLRDataItemsInfo, _T("Signature"), DWORD_toString(pMetaData1->Signature, Hexadecimal), (pMetaData1->Signature == CLR_META_DATA_SIGNATURE ?
																																_T("'BJSB'") : _T("Invalid Signature, must be 'BJSB'")));
			FillData(CLRDataItemsInfo, _T("Version"), VersionNums_toString(pMetaData1->MajorVersion, pMetaData1->MinorVersion));
			FillData(CLRDataItemsInfo, _T("Extra Data Offset"), DWORD_toString(pMetaData1->Reserved, Hexadecimal), _T("Reserved, must be zero"));
			FillData(CLRDataItemsInfo, _T("Version String Length"), DWORD_toString(pMetaData1->Length));
			FillData(CLRDataItemsInfo, _T("Version String"), _T("\"") + MultiByte_toString(&pMetaData1->Name) + _T("\""));
			FillData(CLRDataItemsInfo, _T("Flags"), DWORD_toString(pMetaData2->Flags, Hexadecimal), _T("Reserved, must be zero"));
			FillData(CLRDataItemsInfo, _T("No. of Streams"), DWORD_toString(pMetaData2->NoOfStreams));
			
			PMETA_STREAM_HEADER pMetaStreamHeader = (PMETA_STREAM_HEADER) (((UINT_PTR) pMetaData2) + sizeof(META_DATA_SECTION_HEADER2));
			tstring EditBoxAnnotation;

			for (WORD i = 1; i <= pMetaData2->NoOfStreams; i++)
			{
				static const int MaxStreamName = 32;
				char temp[MaxStreamName];

				FillData(CLRDataItemsInfo, _T("Stream ") + DWORD_toString(i) + _T(" Offset"), DWORD_toString(pMetaStreamHeader->Offset, Hexadecimal));
				FillData(CLRDataItemsInfo, _T("Stream ") + DWORD_toString(i) + _T(" Size"), DWORD_toString(pMetaStreamHeader->Size), FormattedBytesSize(pMetaStreamHeader->Size));

				for (int j = 0; j < MaxStreamName; j += 4)
				{
					CopyMemory(&temp[j], &pMetaStreamHeader->Name[j], 4);

					if (pMetaStreamHeader->Name[j + 3] == '\0')
					{
						tstring StreamNameAnnotation;
						tstring StreamName = MultiByte_toString(temp);

						if (StreamName == _T("#Strings"))
							StreamNameAnnotation = _T("Contains ASCII strings");
						if (StreamName == _T("#US"))
							StreamNameAnnotation = _T("Contains UNICODE user strings");
						else if (StreamName == _T("#Blob"))
							StreamNameAnnotation = _T("Contains data");
						else if (StreamName == _T("#GUID"))
						{
							LPBYTE pGUIDs = (LPBYTE) m_PEReaderWriter.GetVA(pCLRData->MetaData.VirtualAddress + pMetaStreamHeader->Offset);

							EditBoxAnnotation += _T("----------------------------------- 128-bit GUIDs -----------------------------------\n");
							EditBoxAnnotation += CorGUIDs_toString(pGUIDs, pMetaStreamHeader->Size);
							EditBoxAnnotation += _T("--------------------------------------------------------------------------------------------\n");

							StreamNameAnnotation = _T("Contains 128-bit GUIDs");
						}
						else if (StreamName == _T("#~"))
						{
							PMETA_COMPOSITE_HEADER pMetaCompositeHeader = (PMETA_COMPOSITE_HEADER) m_PEReaderWriter.GetVA(pCLRData->MetaData.VirtualAddress + pMetaStreamHeader->Offset);
							EditBoxAnnotation += _T("------------------------ MetaData Composite Header (#~) ------------------------\n")
												_T("Field:\tData:\tAnnotation:\n");

							EditBoxAnnotation += _T("Reserved\t") + DWORD_toString(pMetaCompositeHeader->Reserved) + _T("\tReserved, must be 0\n");
							EditBoxAnnotation += _T("Version\t") + DWORD_toString(pMetaCompositeHeader->MajorVersion) + _T(".") + DWORD_toString(pMetaCompositeHeader->MinorVersion) + _T("\n");
							EditBoxAnnotation += _T("HeapSizes\t") + DWORD_toString(pMetaCompositeHeader->HeapSizes, Hexadecimal) + _T("\t") + CorHeapSizes_toString(pMetaCompositeHeader->HeapSizes) + _T("\n");
							EditBoxAnnotation += _T("Padding/Reserved\t") + DWORD_toString(pMetaCompositeHeader->Padding) + _T("\tReserved, must be 1\n");
							EditBoxAnnotation += _T("Valid\t") + QWORD_toString(pMetaCompositeHeader->Valid, Hexadecimal) + _T("\tValid tables summarized below\n");
							EditBoxAnnotation += _T("Sorted\t") + QWORD_toString(pMetaCompositeHeader->Sorted, Hexadecimal) + _T("\tTable sorting summarized below\n");
							EditBoxAnnotation += _T("Rows\tn/a\tNo. of row entries summarized below\n\n");
							EditBoxAnnotation += _T("Tables:\nName\tSorted?\tRow entries\n") + CorMetadataTablesSummary_toString(pMetaCompositeHeader->Valid,
													pMetaCompositeHeader->Sorted, pMetaCompositeHeader->Rows);
							EditBoxAnnotation += _T("--------------------------------------------------------------------------------------------\n\n");

							StreamNameAnnotation = _T("Contains metadata composite header");
						}
						
						FillData(CLRDataItemsInfo, _T("Stream ") + DWORD_toString(i) + _T(" Name"), _T("\"") + StreamName + _T("\""), StreamNameAnnotation);
						pMetaStreamHeader = (PMETA_STREAM_HEADER) &pMetaStreamHeader->Name[j + 4];

						break;
					}
				}
			}

			Edit_SetText(m_hEditCLRData, (LPTSTR) EditBoxAnnotation.c_str());
		}

		break;

	case 2:	// Resources
		{
			//
		}

		break;

	case 3:	// Strong Name Signature
		{
			//
		}

		break;

	case 4: // VTable Fixup Directory
		{
			PIMAGE_COR_VTABLEFIXUP pVTableFixups = (PIMAGE_COR_VTABLEFIXUP) &pCLRData->VTableFixups;

			// Fill data
			FillData(CLRDataItemsInfo, _T("RVA of VTable"), DWORD_toString(pVTableFixups->RVA, Hexadecimal));
			FillData(CLRDataItemsInfo, _T("Size"), DWORD_toString(pVTableFixups->Count));
			FillData(CLRDataItemsInfo, _T("Type"), DWORD_toString(pVTableFixups->Type), CorVTableFlags_toString(pVTableFixups->Type));

			if (pVTableFixups->RVA == NULL)
				Edit_SetText(m_hEditCLRData, NULL);
		}

		break;
	}

	// Insert ListView items for 'hListViewCLRData'
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < CLRDataItemsInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) CLRDataItemsInfo[i].szText.c_str();

		ListView_InsertItem(m_hListViewCLRData, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) CLRDataItemsInfo[i].szData.c_str();

		ListView_SetItem(m_hListViewCLRData, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) CLRDataItemsInfo[i].szComments.c_str();

		ListView_SetItem(m_hListViewCLRData, &item);
	}

	// Resize columns for 'hListViewCLRData'
	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
		ListView_SetColumnWidth(m_hListViewCLRData, i, 
								i == GetArraySize(GenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);
}