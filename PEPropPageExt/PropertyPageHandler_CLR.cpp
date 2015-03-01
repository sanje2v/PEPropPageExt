#include "stdafx.h"
#include "PropertyPageHandler.h"
#include "CustomCorDefs.h"
#include <algorithm>


PropertyPageHandler_CLR::PropertyPageHandler_CLR(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, std::ref(PEReaderWriter))
{
	m_hTabsCLRData = GetDlgItem(m_hWnd, IDC_TABSCLRDATA);
	m_hListViewCLRData = GetDlgItem(m_hWnd, IDC_LISTCLRDATA);
	m_hEditCLRData = GetDlgItem(m_hWnd, IDC_EDITCLRDATA);

	// Setup controls with layout manager
	m_LayoutManager.AddChildConstraint(IDC_TABSCLRDATA, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_LayoutManager.AddChildConstraint(IDC_LISTCLRDATA, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_LayoutManager.AddChildConstraint(IDC_EDITCLRDATA, CWA_LEFTRIGHT, CWA_BOTTOM);
	
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
	ZeroMemory(&column, sizeof(column));

	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szGenericColumnText[i];
		ListView_InsertColumn(m_hListViewCLRData, i, &column);
	}

	// Insert Tabs and ignore unnecessary once
	static LPWSTR szSEHColumnText[] =
	{
		L"CLR Header",
		L"Metadata",
		L"Strong Name Signature",
		L"VTable Fixup Directory"
	};
	PIMAGE_COR20_HEADER pCLRData;
	int err = m_PEReaderWriter.getCLRHeader(std::ref(pCLRData));
	if (err)
	{
		LogError(L"ERROR: Incomplete 'IMAGE_COR20_HEADER' data. File is not valid.", true);
		return;
	}

	int itemCount = 0;
	TCITEM item;
	ZeroMemory(&item, sizeof(item));

	for (size_t i = 0; i < ARRAYSIZE(szSEHColumnText); ++i)
    {
        item.mask = TCIF_TEXT | TCIF_PARAM;
        item.pszText = LPWSTR(szSEHColumnText[i]);
		item.lParam = LPARAM(i);

		// See if this tab should be shown or not
		switch (static_cast<Tabs>(i))
		{
		case tabMetadata:		// Metadata
			if (pCLRData->MetaData.VirtualAddress == 0 ||
				pCLRData->MetaData.Size == 0)
				continue;

			break;

		case tabStrongNameSig:		// Strong Name Signature
			if (pCLRData->StrongNameSignature.VirtualAddress == 0 ||
				pCLRData->StrongNameSignature.Size == 0)
				continue;

			break;

		case tabVTableFixups:		// VTable entries
			if (pCLRData->VTableFixups.VirtualAddress == 0 ||
				pCLRData->VTableFixups.Size == 0)
				continue;

			break;
		}

		TabCtrl_InsertItem(m_hTabsCLRData, ++itemCount, &item);
    }

	// Select the first tab
	tabsCLRData_OnTabChanged(m_hListViewCLRData, 0);
}

void PropertyPageHandler_CLR::tabsCLRData_OnTabChanged(HWND hControl, int SelectedIndex)
{
	vector<TextAndData> CLRDataItemsInfo;
	PIMAGE_COR20_HEADER pCLRData;
	int err = m_PEReaderWriter.getCLRHeader(std::ref(pCLRData));
	if (err)
	{
		LogError(L"ERROR: Incomplete 'IMAGE_COR20_HEADER' data. File is not valid.", true);
		return;
	}

	ListView_DeleteAllItems(m_hListViewCLRData);
	Edit_SetText(m_hEditCLRData, NULL);

	TCITEM tabItem;
	ZeroMemory(&tabItem, sizeof(tabItem));
	tabItem.mask = TCIF_PARAM;
	TabCtrl_GetItem(m_hTabsCLRData, SelectedIndex, &tabItem);

	switch (static_cast<Tabs>(tabItem.lParam))
	{
		case tabCLRHeader:	// CLR Header
		{
			// Fill them with data
			CLRDataItemsInfo =
			{
				{ L"Size", DWORD_toString(pCLRData->cb), FormattedBytesSize(pCLRData->cb) },
				{ L"Runtime Version", VersionNums_toString(pCLRData->MajorRuntimeVersion, pCLRData->MinorRuntimeVersion) },
				{ L"Metadata Address", DWORD_toString(pCLRData->MetaData.VirtualAddress, Hexadecimal) },
				{ L"Metadata Size", DWORD_toString(pCLRData->MetaData.Size), FormattedBytesSize(pCLRData->MetaData.Size) },
				{ L"Flags", DWORD_toString(pCLRData->Flags, Hexadecimal), CorImageFlags_toString(pCLRData->Flags) },
				{ L"Entry Point", DWORD_toString(pCLRData->EntryPointRVA, Hexadecimal), (TestFlag(pCLRData->Flags, COMIMAGE_FLAGS_NATIVE_ENTRYPOINT) ? L"Native RVA entry point" : L"Managed token entry point") },
				{ L"Resources Address", DWORD_toString(pCLRData->Resources.VirtualAddress, Hexadecimal), (pCLRData->Resources.VirtualAddress > 0 && pCLRData->Resources.Size > 0 ? L"See Resources tab" : L"") },
				{ L"Resources Size", DWORD_toString(pCLRData->Resources.Size), FormattedBytesSize(pCLRData->Resources.Size) },
				{ L"Strong Name Address", DWORD_toString(pCLRData->StrongNameSignature.VirtualAddress, Hexadecimal) },
				{ L"Strong Name Size", DWORD_toString(pCLRData->StrongNameSignature.Size), FormattedBytesSize(pCLRData->StrongNameSignature.Size) },
				{ L"Code Manager Address", DWORD_toString(pCLRData->CodeManagerTable.VirtualAddress, Hexadecimal), L"Deprecated" },
				{ L"Code Manager Size", DWORD_toString(pCLRData->CodeManagerTable.Size), FormattedBytesSize(pCLRData->CodeManagerTable.Size) },
				{ L"VTable Fixups Address", DWORD_toString(pCLRData->VTableFixups.VirtualAddress, Hexadecimal) },
				{ L"VTable Fixups Size", DWORD_toString(pCLRData->VTableFixups.Size), FormattedBytesSize(pCLRData->VTableFixups.Size) },
				{ L"Export Table Address", DWORD_toString(pCLRData->ExportAddressTableJumps.VirtualAddress, Hexadecimal) },
				{ L"Export Table Size", DWORD_toString(pCLRData->ExportAddressTableJumps.Size), FormattedBytesSize(pCLRData->ExportAddressTableJumps.Size) },
				{ L"Native Header Address", DWORD_toString(pCLRData->ManagedNativeHeader.VirtualAddress, Hexadecimal) },
				{ L"Native Header Size", DWORD_toString(pCLRData->ManagedNativeHeader.Size), FormattedBytesSize(pCLRData->ManagedNativeHeader.Size) }
			};
		}
		break;

		case tabMetadata:	// Metadata
		{
			PMETA_DATA_SECTION_HEADER1 pMetaData1;
			err = m_PEReaderWriter.getMetaDataHeader1(std::cref(pCLRData), std::ref(pMetaData1));
			if (err)
			{
				LogError(L"ERROR: Incomplete 'META_DATA_SECTION_HEADER' for metadata. File is not valid.", true);
				break;
			}

			PMETA_DATA_SECTION_HEADER2 pMetaData2;
			err = m_PEReaderWriter.getMetaDataHeader2(std::cref(pMetaData1), std::ref(pMetaData2));
			if (err)
			{
				LogError(L"ERROR: Incomplete 'META_DATA_SECTION_HEADER' for metadata. File is not valid.", true);
				break;
			}

			// Fill with data
			CLRDataItemsInfo =
			{
				{ L"Signature", DWORD_toString(pMetaData1->Signature, Hexadecimal), (pMetaData1->Signature == CLR_META_DATA_SIGNATURE ? L"\"BJSB\"" : L"Invalid Signature, must be \"BJSB\"") },
				{ L"Version", VersionNums_toString(pMetaData1->MajorVersion, pMetaData1->MinorVersion) },
				{ L"Extra Data Offset", DWORD_toString(pMetaData1->Reserved, Hexadecimal), L"Reserved, must be zero" },
				{ L"Version String Length", DWORD_toString(pMetaData1->Length) },
				{ L"Version String", L'\"' + MultiByte_toString(&pMetaData1->Name) + L'\"' },
				{ L"Flags", DWORD_toString(pMetaData2->Flags, Hexadecimal), L"Reserved, must be zero" },
				{ L"No. of Streams", DWORD_toString(pMetaData2->NoOfStreams) }
			};

			wstring EditBoxAnnotation;
			// NOTE: According to .NET tool 'IL DISAM', we count stream nos. from 1
			for (WORD i = 0; i < pMetaData2->NoOfStreams; ++i)
			{
				PMETA_STREAM_HEADER pMetaStreamHeader;
				if (i == 0)
				{
					err = m_PEReaderWriter.getFirstMetaStreamHeader(std::cref(pMetaData2), std::ref(pMetaStreamHeader));
					if (err)
					{
						LogError(L"ERROR: Incomplete 'META_STREAM_HEADER' for stream 1. File is not valid.", true);
						break;
					}
				}
				else
				{
					err = m_PEReaderWriter.getNextMetaStreamHeader(std::cref(pMetaStreamHeader), std::ref(pMetaStreamHeader));
					if (err)
					{
						LogError(L"ERROR: Incomplete 'META_STREAM_HEADER' for stream " + DWORD_toString(i + 1) + L". File is not valid.", true);
						break;
					}
				}

				CLRDataItemsInfo.insert(CLRDataItemsInfo.cend(),
				{
					{ L"Stream " + DWORD_toString(i + 1) + L" Offset", DWORD_toString(pMetaStreamHeader->Offset, Hexadecimal) },
					{ L"Stream " + DWORD_toString(i + 1) + L" Size", DWORD_toString(pMetaStreamHeader->Size), FormattedBytesSize(pMetaStreamHeader->Size) }
				});

				wstring StreamName;
				err = m_PEReaderWriter.getMetaStreamName(std::cref(pMetaStreamHeader), std::ref(StreamName));
				if (err)
				{
					LogError(L"ERROR: Name field of 'META_STREAM_HEADER' is not readable for stream " + DWORD_toString(i + 1) + L". File is not valid.", true);
					break;
				}

				wstring StreamNameAnnotation;
				if (StreamName == L"#Strings")
					StreamNameAnnotation = L"Contains ASCII strings";
				if (StreamName == L"#US")
					StreamNameAnnotation = L"Contains UNICODE user strings";
				else if (StreamName == L"#Blob")
					StreamNameAnnotation = L"Contains data";
				else if (StreamName == L"#GUID")
				{
					LPBYTE pGUIDs;
					err = m_PEReaderWriter.getMetaStreamGUIDs(std::cref(pCLRData), std::cref(pMetaStreamHeader), std::ref(pGUIDs));
					if (err)
					{
						LogError(L"ERROR: One or more CLR GUIDs values are readable. File is not valid.", true);
						break;
					}
					
					EditBoxAnnotation += L"----------------------------------- 128-bit GUIDs -----------------------------------\n";
					EditBoxAnnotation += CorGUIDs_toString(pGUIDs, pMetaStreamHeader->Size);
					EditBoxAnnotation += L"--------------------------------------------------------------------------------------------\n";

					StreamNameAnnotation = L"Contains 128-bit GUIDs";
				}
				else if (StreamName == L"#~")
				{
					PMETA_COMPOSITE_HEADER pMetaCompositeHeader;
					err = m_PEReaderWriter.getMetaCompositeHeader(std::cref(pCLRData), std::cref(pMetaStreamHeader), std::ref(pMetaCompositeHeader));
					if (err)
					{
						LogError(L"ERROR: CLR Meta composite header is not complete. File is not valid.", true);
						break;
					}
					
					EditBoxAnnotation += L"------------------------ MetaData Composite Header (#~) ------------------------\n"
				  						 L"Field:\tData:\tAnnotation:\n";

					EditBoxAnnotation += L"Reserved\t" + DWORD_toString(pMetaCompositeHeader->Reserved) + L"\tReserved, must be 0\n";
					EditBoxAnnotation += L"Version\t" + DWORD_toString(pMetaCompositeHeader->MajorVersion) + L"." + DWORD_toString(pMetaCompositeHeader->MinorVersion) + L"\n";
					EditBoxAnnotation += L"HeapSizes\t" + DWORD_toString(pMetaCompositeHeader->HeapSizes, Hexadecimal) + L"\t" + CorHeapSizes_toString(pMetaCompositeHeader->HeapSizes) + L"\n";
					EditBoxAnnotation += L"Padding/Reserved\t" + DWORD_toString(pMetaCompositeHeader->Padding) + L"\tReserved, must be 1\n";
					EditBoxAnnotation += L"Valid\t" + QWORD_toString(pMetaCompositeHeader->Valid, Hexadecimal) + L"\tValid tables summarized below\n";
					EditBoxAnnotation += L"Sorted\t" + QWORD_toString(pMetaCompositeHeader->Sorted, Hexadecimal) + L"\tTable sorting summarized below\n";
					EditBoxAnnotation += L"Rows\tn/a\tNo. of row entries summarized below\n\n";
					EditBoxAnnotation += L"Tables:\nName\tSorted?\tRow entries\n" + CorMetadataTablesSummary_toString(pMetaCompositeHeader->Valid,
											pMetaCompositeHeader->Sorted, pMetaCompositeHeader->Rows);
					EditBoxAnnotation += L"--------------------------------------------------------------------------------------------\n\n";

					StreamNameAnnotation = L"Contains metadata composite header";
				}

				CLRDataItemsInfo.insert(CLRDataItemsInfo.cend(),
				{
					{ L"Stream " + DWORD_toString(i + 1) + L" Name", L'\"' + StreamName + L'\"', StreamNameAnnotation }
				});
			}

			Edit_SetText(m_hEditCLRData, LPWSTR(EditBoxAnnotation.c_str()));
		}
		break;

	case tabStrongNameSig:	// Strong Name Signature
		{
			wstring HashHex;
			PBYTE pStrongNameHash;
			err = m_PEReaderWriter.getStrongNameSignatureHash(std::cref(pCLRData), std::ref(pStrongNameHash));
			if (err)
			{
				LogError(L"ERROR: Strong name signature hash is unreadable. File is not valid.", true);
				break;
			}

			for (DWORD i = 0; i < pCLRData->StrongNameSignature.Size; ++i)
				HashHex += DWORD_toString(pStrongNameHash[i], Hexadecimal, false, false) + L' ';

			Edit_SetText(m_hEditCLRData, LPWSTR(HashHex.c_str()));
		}
		break;

	case tabVTableFixups: // VTable Fixup Directory
		{
			PIMAGE_COR_VTABLEFIXUP pVTableFixupsDir;
			const int NoOfVTableFixupsDirectories = m_PEReaderWriter.getNoOfVTableFixupsDirectories(std::cref(pCLRData));

			for (int i = 0; i < NoOfVTableFixupsDirectories; ++i)
			{
				err = m_PEReaderWriter.getVTableFixupsDirectory(std::cref(pCLRData), i, std::ref(pVTableFixupsDir));
				if (err)
				{
					LogError(L"ERROR: CLR's virtual fixup directory at index " + DWORD_toString(i) + L" is incomplete. File is not valid.", true);
					break;
				}

				// Fill data
				CLRDataItemsInfo.insert(CLRDataItemsInfo.cend(),
				{
					{ L"RVA of VTable", DWORD_toString(pVTableFixupsDir->RVA, Hexadecimal) },
					{ L"Size", DWORD_toString(pVTableFixupsDir->Count), L"No. of entries" },
					{ L"Type", DWORD_toString(pVTableFixupsDir->Type), CorVTableType_toString(pVTableFixupsDir->Type) }
				});
			}
		}
		break;
	}

	// Insert ListView items for 'hListViewCLRData'
	LV_ITEM lvItem;
	ZeroMemory(&lvItem, sizeof(lvItem));

	for (size_t i = 0; i < CLRDataItemsInfo.size(); ++i)
	{
		lvItem.iItem = int(i);
		lvItem.iSubItem = 0;
		lvItem.mask = LVIF_TEXT;
		lvItem.pszText = LPWSTR(_wcsdup(CLRDataItemsInfo[i].Text.c_str()));
		ListView_InsertItem(m_hListViewCLRData, &lvItem);
		free(lvItem.pszText);

		lvItem.iSubItem = 1;
		lvItem.pszText = LPWSTR(_wcsdup(CLRDataItemsInfo[i].Data.c_str()));
		ListView_SetItem(m_hListViewCLRData, &lvItem);
		free(lvItem.pszText);

		lvItem.iSubItem = 2;
		lvItem.pszText = LPWSTR(_wcsdup(CLRDataItemsInfo[i].Comments.c_str()));
		ListView_SetItem(m_hListViewCLRData, &lvItem);
		free(lvItem.pszText);
	}

	// Resize columns for 'hListViewCLRData'
	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
		ListView_SetColumnWidth(m_hListViewCLRData, i, 
		                        (i == ARRAYSIZE(szGenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));
}