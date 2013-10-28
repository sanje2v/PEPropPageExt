#include "PropertyPageHandler.h"


PropertyPageHandler_TLS::PropertyPageHandler_TLS(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, PEReaderWriter)
{
	m_hListViewTLSData = GetDlgItem(m_hWnd, IDC_LISTTLSDATA);
	m_hListViewCallbacks = GetDlgItem(m_hWnd, IDC_LISTTLSCALLBACKS);

	// Setup controls with layout manager
	m_pLayoutManager->AddChildConstraint(IDC_LISTLOADCONFIG, CWA_LEFTRIGHT, CWA_TOP);
	m_pLayoutManager->AddChildConstraint(IDC_LISTTLSCALLBACKS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Set full row selection style for listviews
	ListView_SetExtendedListViewStyleEx(m_hListViewTLSData, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
}

void PropertyPageHandler_TLS::OnInitDialog()
{
	// Fill them with data
	vector<TextAndData> TLSItemsInfo;

	switch (m_PEReaderWriter.GetPEType())
	{
	case PEReadWrite::PE32:
		{
			PIMAGE_TLS_DIRECTORY32 pTLSData = (PIMAGE_TLS_DIRECTORY32) m_PEReaderWriter.GetTLSDirectory();

			FillData(TLSItemsInfo, _T("Raw Data Start (VA)"), DWORD_toString(pTLSData->StartAddressOfRawData, Hexadecimal));
			FillData(TLSItemsInfo, _T("Raw Data End (VA)"), DWORD_toString(pTLSData->EndAddressOfRawData, Hexadecimal));
			FillData(TLSItemsInfo, _T("Address of Index (VA)"), DWORD_toString(pTLSData->AddressOfIndex, Hexadecimal));
			FillData(TLSItemsInfo, _T("Address of Callbacks (VA)"), DWORD_toString(pTLSData->AddressOfCallBacks, Hexadecimal));
			FillData(TLSItemsInfo, _T("Size of Zero Fill"), DWORD_toString(pTLSData->SizeOfZeroFill), FormattedBytesSize(pTLSData->SizeOfZeroFill));
			FillData(TLSItemsInfo, _T("Characteristics"), DWORD_toString(pTLSData->Characteristics), _T("Reserved, must be zero"));
		}

		break;

	case PEReadWrite::PE64:
		{
			PIMAGE_TLS_DIRECTORY64 pTLSData = (PIMAGE_TLS_DIRECTORY64) m_PEReaderWriter.GetTLSDirectory();

			FillData(TLSItemsInfo, _T("Raw Data Start (VA)"), QWORD_toString(pTLSData->StartAddressOfRawData, Hexadecimal));
			FillData(TLSItemsInfo, _T("Raw Data End (VA)"), QWORD_toString(pTLSData->EndAddressOfRawData, Hexadecimal));
			FillData(TLSItemsInfo, _T("Address of Index (VA)"), QWORD_toString(pTLSData->AddressOfIndex, Hexadecimal));
			FillData(TLSItemsInfo, _T("Address of Callbacks (VA)"), QWORD_toString(pTLSData->AddressOfCallBacks, Hexadecimal));
			FillData(TLSItemsInfo, _T("Size of Zero Fill"), DWORD_toString(pTLSData->SizeOfZeroFill), FormattedBytesSize(pTLSData->SizeOfZeroFill));
			FillData(TLSItemsInfo, _T("Characteristics"), DWORD_toString(pTLSData->Characteristics), _T("Reserved, must be zero"));
		}

		break;
	}

	// Insert ListView columns for 'hListViewTLSData'
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(LV_COLUMN));

	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = GenericColumnText[i];
		ListView_InsertColumn(m_hListViewTLSData, i, &column);
	}

	// Insert ListView columns for 'hListViewCallbacks'
	static LPTSTR CallbacksColumnText[] = {_T("Index"), _T("RVA")};

	for (unsigned int i = 0; i < GetArraySize(CallbacksColumnText); i++)
	{
		column.mask = LVCF_TEXT;
		column.pszText = CallbacksColumnText[i];
		ListView_InsertColumn(m_hListViewCallbacks, i, &column);
	}

	// Insert ListView items for 'hListViewTLSData'
	LV_ITEM item;
	ZeroMemory(&item, sizeof(LV_ITEM));

	for (unsigned int i = 0; i < TLSItemsInfo.size(); i++)
	{
		item.iItem = i;
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = (LPTSTR) TLSItemsInfo[i].szText.c_str();
		ListView_InsertItem(m_hListViewTLSData, &item);

		item.iSubItem = 1;
		item.pszText = (LPTSTR) TLSItemsInfo[i].szData.c_str();
		ListView_SetItem(m_hListViewTLSData, &item);

		item.iSubItem = 2;
		item.pszText = (LPTSTR) TLSItemsInfo[i].szComments.c_str();
		ListView_SetItem(m_hListViewTLSData, &item);
	}

	// Insert ListView items for 'hListViewCallbacks'
	unsigned int i = 0;
	ZeroMemory(&item, sizeof(LV_ITEM));

	switch (m_PEReaderWriter.GetPEType())
	{
	case PEReadWrite::PE32:
		{
			PIMAGE_NT_HEADERS32 pNTheaders32 = m_PEReaderWriter.GetSecondaryHeader<PIMAGE_NT_HEADERS32>();
			PIMAGE_TLS_DIRECTORY32 pTLSData = (PIMAGE_TLS_DIRECTORY32) m_PEReaderWriter.GetTLSDirectory();
			DWORD Callback = ((LPDWORD) m_PEReaderWriter.GetVA(pTLSData->AddressOfCallBacks - pNTheaders32->OptionalHeader.ImageBase))[i];

			while (Callback)
			{
				tstring dummy;

				item.iItem = i;
				item.iSubItem = 0;
				item.mask = LVIF_TEXT;
				dummy = DWORD_toString(i);
				item.pszText = (LPTSTR) dummy.c_str();
				ListView_InsertItem(m_hListViewCallbacks, &item);

				item.iSubItem = 1;
				dummy = DWORD_toString(Callback, Hexadecimal);
				item.pszText = (LPTSTR) dummy.c_str();
				ListView_SetItem(m_hListViewCallbacks, &item);

				Callback = ((LPDWORD) m_PEReaderWriter.GetVA(pTLSData->AddressOfCallBacks - pNTheaders32->OptionalHeader.ImageBase))[++i];
			}
		}

		break;

	case PEReadWrite::PE64:
		{
			PIMAGE_NT_HEADERS64 pNTheaders64 = m_PEReaderWriter.GetSecondaryHeader<PIMAGE_NT_HEADERS64>();
			PIMAGE_TLS_DIRECTORY64 pTLSData = (PIMAGE_TLS_DIRECTORY64) m_PEReaderWriter.GetTLSDirectory();
			ULONGLONG Callback = ((PULONGLONG) m_PEReaderWriter.GetVA((DWORD) (pTLSData->AddressOfCallBacks - pNTheaders64->OptionalHeader.ImageBase)))[i];

			while (Callback)
			{
				tstring dummy;

				item.iItem = i;
				item.iSubItem = 0;
				item.mask = LVIF_TEXT;
				dummy = QWORD_toString(i);
				item.pszText = (LPTSTR) dummy.c_str();
				ListView_InsertItem(m_hListViewCallbacks, &item);

				item.iSubItem = 1;
				dummy = QWORD_toString(Callback, Hexadecimal);
				item.pszText = (LPTSTR) dummy.c_str();
				ListView_SetItem(m_hListViewCallbacks, &item);

				Callback = ((PULONGLONG) m_PEReaderWriter.GetVA((DWORD) (pTLSData->AddressOfCallBacks - pNTheaders64->OptionalHeader.ImageBase)))[++i];
			}
		}

		break;
	}

	// Resize columns for 'hListViewTLSData'
	for (unsigned int i = 0; i < GetArraySize(GenericColumnText); i++)
		ListView_SetColumnWidth(m_hListViewTLSData, i, i == GetArraySize(GenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE);

	// Resize columns for 'hListViewCallbacks'
	for (unsigned int i = 0; i < GetArraySize(CallbacksColumnText); i++)
		ListView_SetColumnWidth(m_hListViewCallbacks, i, LVSCW_AUTOSIZE_USEHEADER);
}