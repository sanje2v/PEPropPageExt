#include "PropertyPageHandler.h"
#include <sstream>


PropertyPageHandler_Tools::PropertyPageHandler_Tools(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter)
{
	m_hComboConvertAddrFrom = GetDlgItem(m_hWnd, IDC_CMBCONVERTADDRFROM);
	m_hEditConvertAddrFrom = GetDlgItem(m_hWnd, IDC_EDITCONVERTADDRFROM);
	m_hComboConvertAddrTo = GetDlgItem(m_hWnd, IDC_CMBCONVERTADDRTO);
	m_hEditConvertAddrTo = GetDlgItem(m_hWnd, IDC_EDITCONVERTADDRTO);
	m_hBtnConvertAddr = GetDlgItem(m_hWnd, IDC_BTNCONVERTADDR);
	m_hEditSHA1Hash = GetDlgItem(m_hWnd, IDC_EDITSHA1HASH);
	m_hEditMD5Hash = GetDlgItem(m_hWnd, IDC_EDITMD5HASH);
	m_hEditVerifyHash = GetDlgItem(m_hWnd, IDC_EDITVERIFYHASH);

	// Set items for combo boxes
	ComboBox_AddString(m_hComboConvertAddrFrom, _T("RVA"));
	ComboBox_AddString(m_hComboConvertAddrFrom, _T("File offset"));
	ComboBox_AddString(m_hComboConvertAddrTo, _T("RVA"));
	ComboBox_AddString(m_hComboConvertAddrTo, _T("File offset"));

	// Select first items for combo boxes
	ComboBox_SetCurSel(m_hComboConvertAddrFrom, 0);
	ComboBox_SetCurSel(m_hComboConvertAddrTo, 1);

	// Set default text for converter edit boxes
	Edit_SetText(m_hEditConvertAddrFrom, _T("0x0"));
	Edit_SetText(m_hEditConvertAddrTo, _T("0x0"));
	Edit_SetText(m_hEditSHA1Hash, _T("Error calculating hash"));
	Edit_SetText(m_hEditMD5Hash, _T("Error calculating hash"));

	// Set tooltips
	Edit_SetCueBannerText(m_hEditVerifyHash, _T("Enter a SHA1/MD5 hash here to verify"));
}

void PropertyPageHandler_Tools::OnInitDialog()
{
	// Initialize Windows Crypt API
	HCRYPTPROV hCryptProv = NULL;
	HCRYPTHASH hHash = NULL;
	
	if (CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0))
	{
		DWORD dwHashLen = 0;

		// Create SHA1 hash object
		if (CryptCreateHash(hCryptProv, CALG_SHA1, 0, 0, &hHash))
			if (CryptHashData(hHash, (const BYTE *) m_PEReaderWriter.GetVA(0, true),
								m_PEReaderWriter.GetFileSize().LowPart, 0))
				if (CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwHashLen, 0))
				{
					unique_ptr<BYTE> pHash(new BYTE[dwHashLen]);
					unique_ptr<TCHAR> szHash(new TCHAR[dwHashLen * 2 + 1]);

					CryptGetHashParam(hHash, HP_HASHVAL, pHash.get(), &dwHashLen, 0);

					for (DWORD i = 0; i < dwHashLen; i++)
						_stprintf_s(&szHash.get()[i * 2], 3, _T("%02x"), pHash.get()[i]);

					// NOTE: 'szHash' is already null terminated

					Edit_SetText(m_hEditSHA1Hash, szHash.get());
				}

		// Release handle
		if (hHash) CryptDestroyHash(hHash);

		// Create MD5 hash object
		if (CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash))
			if (CryptHashData(hHash, (const BYTE *) m_PEReaderWriter.GetVA(0, true),
								m_PEReaderWriter.GetFileSize().LowPart, 0))
				if (CryptGetHashParam(hHash, HP_HASHVAL, NULL, &dwHashLen, 0))
				{
					unique_ptr<BYTE> pHash(new BYTE[dwHashLen]);
					unique_ptr<TCHAR> szHash(new TCHAR[dwHashLen * 2 + 1]);

					CryptGetHashParam(hHash, HP_HASHVAL, pHash.get(), &dwHashLen, 0);

					for (DWORD i = 0; i < dwHashLen; i++)
						_stprintf_s(&szHash.get()[i * 2], 3, _T("%02x"), pHash.get()[i]);

					Edit_SetText(m_hEditMD5Hash, szHash.get());
				}

		// Release handles
		if (hHash) CryptDestroyHash(hHash);
		if (hCryptProv) CryptReleaseContext(hCryptProv, 0);
	}

	// Prepare brushes
	m_hbrushGreen = CreateSolidBrush(RGB(0x00, 0xFF, 0x00));
	m_hbrushRed = CreateSolidBrush(RGB(0xFF, 0x00, 0x00));

	// Load bitmap
	m_hbitmapCorrect = LoadImage(GetWindowInstance(m_hWnd), MAKEINTRESOURCE(IDB_BITMAPCORRECT), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT);
	m_hbitmapIncorrect = LoadImage(GetWindowInstance(m_hWnd), MAKEINTRESOURCE(IDB_BITMAPINCORRECT), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT);
}

void PropertyPageHandler_Tools::btnConvertAddr_OnClick()
{
	int ConvertAddrFrom_Index = ComboBox_GetCurSel(m_hComboConvertAddrFrom);
	int ConvertAddrTo_Index = ComboBox_GetCurSel(m_hComboConvertAddrTo);
	TCHAR szBufferAddrFrom[64];

	Edit_GetText(m_hEditConvertAddrFrom, szBufferAddrFrom, GetArraySize(szBufferAddrFrom));

	if (ConvertAddrFrom_Index == ConvertAddrTo_Index)
	{
		Edit_SetText(m_hComboConvertAddrTo, szBufferAddrFrom);

		return;
	}

	tstringstream ss;
	DWORD BufferAddrFrom;
	ss << std::hex << szBufferAddrFrom;
	ss >> BufferAddrFrom;
	ss.clear();

	switch (ConvertAddrFrom_Index)
	{
	case 0:	// From RVA
		// To File offset
		Edit_SetText(m_hEditConvertAddrTo, DWORD_toString(m_PEReaderWriter.RVAToFileOffset(BufferAddrFrom), Hexadecimal).c_str());

		break;

	case 1:	// From File offset
		// To RVA
		Edit_SetText(m_hEditConvertAddrTo, DWORD_toString(m_PEReaderWriter.FileOffsetToRVA(BufferAddrFrom), Hexadecimal).c_str());
	}
}

void PropertyPageHandler_Tools::txtVerifyHash_Changed()
{
	TCHAR szBuffer[512];
	tstring SourceHash, DestHash;

	// Get source hash
	Edit_GetText(m_hEditVerifyHash, szBuffer, GetArraySize(szBuffer));
	SourceHash = szBuffer;

	if (SourceHash.empty())
	{
		SendDlgItemMessage(m_hWnd, IDC_IMGVERIFY, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, NULL);

		return;
	}

	// Get SHA1 hash
	Edit_GetText(m_hEditSHA1Hash, szBuffer, GetArraySize(szBuffer));
	DestHash = szBuffer;

	if (SourceHash == DestHash)
	{
		SendDlgItemMessage(m_hWnd, IDC_IMGVERIFY, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) m_hbitmapCorrect);

		return;
	}

	// Get MD5 hash
	Edit_GetText(m_hEditMD5Hash, szBuffer, GetArraySize(szBuffer));
	DestHash = szBuffer;

	if (SourceHash == DestHash)
	{
		SendDlgItemMessage(m_hWnd, IDC_IMGVERIFY, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) m_hbitmapCorrect);

		return;
	}

	// Else
	SendDlgItemMessage(m_hWnd, IDC_IMGVERIFY, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) m_hbitmapIncorrect);
}