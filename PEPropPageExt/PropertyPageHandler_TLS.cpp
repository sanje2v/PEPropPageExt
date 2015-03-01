#include "stdafx.h"
#include "PropertyPageHandler.h"


PropertyPageHandler_TLS::PropertyPageHandler_TLS(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, std::ref(PEReaderWriter))
{
	m_hListViewTLSData = GetDlgItem(m_hWnd, IDC_LISTTLSDATA);
	m_hListViewCallbacks = GetDlgItem(m_hWnd, IDC_LISTTLSCALLBACKS);

	// Setup controls with layout manager
	m_LayoutManager.AddChildConstraint(IDC_LISTTLSDATA, CWA_LEFTRIGHT, CWA_TOP);
	m_LayoutManager.AddChildConstraint(IDC_LISTTLSCALLBACKS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Set full row selection style for listviews
	ListView_SetExtendedListViewStyleEx(m_hListViewTLSData, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
}

void PropertyPageHandler_TLS::OnInitDialog()
{
	// Fill them with data
	vector<TextAndData> TLSItemsInfo;
	PEReadWrite::PEType PEType = m_PEReaderWriter.getPEType();

	union
	{
		PIMAGE_TLS_DIRECTORY32 pTLSData32;
		PIMAGE_TLS_DIRECTORY64 pTLSData64;
	};

	int err;
	switch (PEType)
	{
		case PEReadWrite::PEType::PE32:
		{
			err = m_PEReaderWriter.getTLSDirectory(std::ref(pTLSData32));
			if (err)
			{
				LogError(L"ERROR: Couldn't read 'IMAGE_TLS_DIRECTORY32' header. File is not valid.", true);
				return;
			}

			TLSItemsInfo =
			{
				{ L"Raw Data Start (VA)", DWORD_toString(pTLSData32->StartAddressOfRawData, Hexadecimal) },
				{ L"Raw Data End (VA)", DWORD_toString(pTLSData32->EndAddressOfRawData, Hexadecimal) },
				{ L"Addr of Index (VA)", DWORD_toString(pTLSData32->AddressOfIndex, Hexadecimal) },
				{ L"Addr of Callbacks (VA)", DWORD_toString(pTLSData32->AddressOfCallBacks, Hexadecimal) },
				{ L"Size of Zero Fill", DWORD_toString(pTLSData32->SizeOfZeroFill), FormattedBytesSize(pTLSData32->SizeOfZeroFill) },
				{ L"Characteristics", DWORD_toString(pTLSData32->Characteristics), L"Reserved, must be zero" }
			};
		}

		break;

		case PEReadWrite::PEType::PE64:
		{
			err = m_PEReaderWriter.getTLSDirectory(std::ref(pTLSData64));
			if (err)
			{
				LogError(L"ERROR: Couldn't read 'IMAGE_TLS_DIRECTORY64' header. File is not valid.", true);
				return;
			}

			TLSItemsInfo =
			{
				{ L"Raw Data Start (VA)", QWORD_toString(pTLSData64->StartAddressOfRawData, Hexadecimal) },
				{ L"Raw Data End (VA)", QWORD_toString(pTLSData64->EndAddressOfRawData, Hexadecimal) },
				{ L"Addr of Index (VA)", QWORD_toString(pTLSData64->AddressOfIndex, Hexadecimal) },
				{ L"Addr of Callbacks (VA)", QWORD_toString(pTLSData64->AddressOfCallBacks, Hexadecimal) },
				{ L"Size of Zero Fill", DWORD_toString(pTLSData64->SizeOfZeroFill), FormattedBytesSize(pTLSData64->SizeOfZeroFill) },
				{ L"Characteristics", DWORD_toString(pTLSData64->Characteristics), L"Reserved, must be zero" }
			};
		}

		break;

		default:
			LogError(L"ERROR: This type of Portable Executable doesn't have any Thread Local Storage data.", true);
			return;
	}

	// Insert ListView columns for 'hListViewTLSData'
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(column));

	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szGenericColumnText[i];
		ListView_InsertColumn(m_hListViewTLSData, i, &column);
	}

	// Insert ListView columns for 'hListViewCallbacks'
	static LPWSTR szCallbacksColumnText[] = { L"Index", L"RVA" };

	for (size_t i = 0; i < ARRAYSIZE(szCallbacksColumnText); ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szCallbacksColumnText[i];
		ListView_InsertColumn(m_hListViewCallbacks, i, &column);
	}

	// Insert ListView items for 'hListViewTLSData'
	LV_ITEM item;
	ZeroMemory(&item, sizeof(item));

	for (size_t i = 0; i < TLSItemsInfo.size(); ++i)
	{
		item.iItem = int(i);
		item.iSubItem = 0;
		item.mask = LVIF_TEXT;
		item.pszText = LPWSTR(_wcsdup(TLSItemsInfo[i].Text.c_str()));
		ListView_InsertItem(m_hListViewTLSData, &item);
		free(item.pszText);

		item.iSubItem = 1;
		item.pszText = LPWSTR(_wcsdup(TLSItemsInfo[i].Data.c_str()));
		ListView_SetItem(m_hListViewTLSData, &item);
		free(item.pszText);

		item.iSubItem = 2;
		item.pszText = LPWSTR(_wcsdup(TLSItemsInfo[i].Comments.c_str()));
		ListView_SetItem(m_hListViewTLSData, &item);
		free(item.pszText);
	}

	// Insert ListView items for 'hListViewCallbacks'
	int i = 0;
	ZeroMemory(&item, sizeof(item));

	switch (PEType)
	{
		case PEReadWrite::PEType::PE32:
		{
			DWORD Callback = m_PEReaderWriter.getTLSCallbackByIndex(pTLSData32, i);

			while (Callback)
			{
				item.iItem = i;
				item.iSubItem = 0;
				item.mask = LVIF_TEXT;
				item.pszText = LPWSTR(_wcsdup(DWORD_toString(i).c_str()));
				ListView_InsertItem(m_hListViewCallbacks, &item);
				free(item.pszText);

				item.iSubItem = 1;
				item.pszText = LPWSTR(_wcsdup(DWORD_toString(Callback, Hexadecimal).c_str()));
				ListView_SetItem(m_hListViewCallbacks, &item);
				free(item.pszText);

				Callback = m_PEReaderWriter.getTLSCallbackByIndex(pTLSData32, ++i);
			}
		}

		break;

		case PEReadWrite::PEType::PE64:
		{
			ULONGLONG Callback = m_PEReaderWriter.getTLSCallbackByIndex(pTLSData64, i);

			while (Callback)
			{
				item.iItem = i;
				item.iSubItem = 0;
				item.mask = LVIF_TEXT;
				item.pszText = LPWSTR(_wcsdup(QWORD_toString(i).c_str()));
				ListView_InsertItem(m_hListViewCallbacks, &item);
				free(item.pszText);

				item.iSubItem = 1;
				item.pszText = LPWSTR(_wcsdup(QWORD_toString(Callback, Hexadecimal).c_str()));
				ListView_SetItem(m_hListViewCallbacks, &item);
				free(item.pszText);

				Callback = m_PEReaderWriter.getTLSCallbackByIndex(pTLSData64, ++i);
			}
		}

		break;
	}

	// Resize columns for 'hListViewTLSData'
	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText); ++i)
		ListView_SetColumnWidth(m_hListViewTLSData,
		                        i,
								(i == ARRAYSIZE(szGenericColumnText) - 1 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));

	// Resize columns for 'hListViewCallbacks'
	for (size_t i = 0; i < ARRAYSIZE(szCallbacksColumnText); ++i)
		ListView_SetColumnWidth(m_hListViewCallbacks, i, LVSCW_AUTOSIZE_USEHEADER);
}