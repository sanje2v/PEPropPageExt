#include "stdafx.h"
#include "PropertyPageHandler.h"
#include <algorithm>
#include <sstream>
#include <udis86.h>
#include <Uxtheme.h>


extern CPEPropPageExtModule g_PEPropPageExtModule;

PropertyPageHandler_Tools::PropertyPageHandler_Tools(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, std::ref(PEReaderWriter)),
	m_hTooltipDisassemble(NULL, funcDestroyWindow)
{
	// Index of combo items
	m_hComboConvertAddrFrom = GetDlgItem(m_hWnd, IDC_CMBCONVERTADDRFROM);
	m_hEditConvertAddrFrom = GetDlgItem(m_hWnd, IDC_EDITCONVERTADDRFROM);
	m_hComboConvertAddrTo = GetDlgItem(m_hWnd, IDC_CMBCONVERTADDRTO);
	m_hEditConvertAddrTo = GetDlgItem(m_hWnd, IDC_EDITCONVERTADDRTO);
	m_hBtnConvertAddr = GetDlgItem(m_hWnd, IDC_BTNCONVERTADDR);
	m_hEditSHA1Hash = GetDlgItem(m_hWnd, IDC_EDITSHA1HASH);
	m_hEditMD5Hash = GetDlgItem(m_hWnd, IDC_EDITMD5HASH);
	m_hEditVerifyHash = GetDlgItem(m_hWnd, IDC_EDITVERIFYHASH);
	m_hStaticVerify = GetDlgItem(m_hWnd, IDC_STATICVERIFY);
	m_hComboHexViewerAddr = GetDlgItem(m_hWnd, IDC_CMBHEXVIEWADDRTYPE);
	m_hComboHexViewerDataSize = GetDlgItem(m_hWnd, IDC_CMBHEXVIEWDATASIZE);
	m_hEditHexViewerAddr = GetDlgItem(m_hWnd, IDC_EDITHEXVIEWADDR);
	m_hCheckBoxDisassemble = GetDlgItem(m_hWnd, IDC_CHKHEXVIEWDISASSEMBLE);
	m_hEditHexViewer = GetDlgItem(m_hWnd, IDC_EDITHEXVIEWER);

	// Setup controls with layout manager
	m_LayoutManager.AddChildConstraint(IDC_GRPADDRCONV, CWA_LEFTRIGHT, CWA_TOP);
	m_LayoutManager.AddChildConstraint(IDC_GRPHASHES, CWA_LEFTRIGHT, CWA_TOP);
	m_LayoutManager.AddChildConstraint(IDC_GRPHEXVIEWER, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
	m_LayoutManager.AddChildConstraint(IDC_EDITHEXVIEWER, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Set items for combo boxes
	ComboBox_AddString(m_hComboConvertAddrFrom, L"RVA");
	ComboBox_AddString(m_hComboConvertAddrFrom, L"VA");
	ComboBox_AddString(m_hComboConvertAddrFrom, L"File offset");
	ComboBox_AddString(m_hComboConvertAddrTo, L"RVA");
	ComboBox_AddString(m_hComboConvertAddrTo, L"VA");
	ComboBox_AddString(m_hComboConvertAddrTo, L"File offset");
	ComboBox_AddString(m_hComboHexViewerAddr, L"RVA");
	ComboBox_AddString(m_hComboHexViewerAddr, L"VA");
	ComboBox_AddString(m_hComboHexViewerAddr, L"File offset");

	ComboBox_AddString(m_hComboHexViewerDataSize, L"1-byte");
	ComboBox_AddString(m_hComboHexViewerDataSize, L"2-bytes");
	ComboBox_AddString(m_hComboHexViewerDataSize, L"4-bytes");
	ComboBox_AddString(m_hComboHexViewerDataSize, L"8-bytes");

	// Select first items for combo boxes
	enum class AddressTypeIndex : int
	{
		IDX_RVA = 0,
		IDX_VA = 1,
		IDX_FILEOFFSET = 2
	};
	ComboBox_SetCurSel(m_hComboConvertAddrFrom, AddressTypeIndex::IDX_FILEOFFSET);
	ComboBox_SetCurSel(m_hComboConvertAddrTo, AddressTypeIndex::IDX_RVA);
	ComboBox_SetCurSel(m_hComboHexViewerAddr, AddressTypeIndex::IDX_RVA);
	ComboBox_SetCurSel(m_hComboHexViewerDataSize, AddressTypeIndex::IDX_RVA);

	// Set maximum no. of characters that can be input
	Edit_SetLimitText(m_hEditConvertAddrFrom, 18); // 18 = Two characters for '0x' and 16 characters for 64-bits numbers
	Edit_SetLimitText(m_hEditConvertAddrTo, 18);
	Edit_SetLimitText(m_hEditVerifyHash, 40);      // 40 = Size of SHA1 hash
	Edit_SetLimitText(m_hEditHexViewerAddr, 18);

	// Set default text for converter edit boxes
	Edit_SetText(m_hEditSHA1Hash, L"Error calculating hash");
	Edit_SetText(m_hEditMD5Hash, L"Error calculating hash");

	// Set cue text for edit boxes
	Edit_SetCueBannerText(m_hEditConvertAddrFrom, L"Address in hex");
	Edit_SetCueBannerText(m_hEditConvertAddrTo, L"Address in hex");
	Edit_SetCueBannerText(m_hEditVerifyHash, L"Enter a SHA1/MD5 hash here to verify");
	Edit_SetCueBannerText(m_hEditHexViewerAddr, L"Address in hex here");

	// Set default view for hex viewer
	m_View = Viewer::Hex;

	// Tooltip is not exposed for checkboxes (which actually is a command control)
	//	so we create a tooltip window and attach it to 'Disassemble' checkbox
	m_hTooltipDisassemble = Tooltip_Create(hWnd, g_PEPropPageExtModule.getInstance());
	if (m_hTooltipDisassemble.get())
	{
		// Set this tooltip as topmost window, required as specified in documentation
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		TOOLINFO ToolInfo;
		ZeroMemory(&ToolInfo, sizeof(ToolInfo));

		ToolInfo.cbSize = sizeof(ToolInfo);
		ToolInfo.uFlags = TTF_SUBCLASS | TTF_IDISHWND;	// Let the tooltip subclass the attached window
		ToolInfo.hwnd = hWnd;
		ToolInfo.uId = UINT_PTR(m_hCheckBoxDisassemble);
		ToolInfo.lpszText = L"Disassembles according to architecture specified in PE Optional Header";
		Tooltip_AddTool(m_hTooltipDisassemble.get(), &ToolInfo);
	}

	// Set font for rich edit
	CHARFORMAT CharFormat;
	ZeroMemory(&CharFormat, sizeof(CharFormat));

	CharFormat.cbSize = sizeof(CharFormat);
	CharFormat.dwMask = CFM_FACE | CFM_SIZE;
	wcscpy_s(CharFormat.szFaceName, L"Fixedsys"); // NOTE: We set a monospaced font
	CharFormat.yHeight = 180;
	SendMessage(m_hEditHexViewer, EM_SETCHARFORMAT, SCF_DEFAULT, LPARAM(&CharFormat));

	// Set tab stop  for rich edit
	DWORD arrcTabs[] = { 40, 60 };
	Edit_SetTabStops(m_hEditHexViewer, ARRAYSIZE(arrcTabs), arrcTabs);
}

PropertyPageHandler_Tools::~PropertyPageHandler_Tools()
{}

void PropertyPageHandler_Tools::OnInitDialog()
{
	m_PEReaderWriter.computeHashes(std::ref(m_SHA1), std::ref(m_MD5));
	Edit_SetText(m_hEditSHA1Hash, m_SHA1.c_str());
	Edit_SetText(m_hEditMD5Hash, m_MD5.c_str());
}

void PropertyPageHandler_Tools::btnConvertAddr_OnClick()
{
	// Get selected items in combo boxes
	AddressType idxConvertAddrFrom = static_cast<AddressType> (ComboBox_GetCurSel(m_hComboConvertAddrFrom));
	AddressType idxConvertAddrTo = static_cast<AddressType> (ComboBox_GetCurSel(m_hComboConvertAddrTo));

	// If both convert from and convert to combos have the same item
	//	selected, then don't go any further
	if (idxConvertAddrFrom == idxConvertAddrTo)
		return;

	// Get text in convert from edit box
	WCHAR szStrFromTextbox[32] = { 0 };
	Edit_GetText(m_hEditConvertAddrFrom, szStrFromTextbox, ARRAYSIZE(szStrFromTextbox));

	// Convert address from hex text to unsigned integer
	wstringstream ss;
	ULONGLONG addrBufferFrom;

	ss << std::hex << szStrFromTextbox;
	ss >> addrBufferFrom;
	if (ss.bad() || ss.fail())
		return;

	// Find from which to convert where and do so
	switch (idxConvertAddrFrom)
	{
		case AddressType::RVA:	// From RVA
			switch (idxConvertAddrTo)
			{
				case AddressType::VA:		// To VA
					Edit_SetText(m_hEditConvertAddrTo,
								 QWORD_toString(m_PEReaderWriter.RVAToVA(DWORD(addrBufferFrom)), Hexadecimal).c_str());
					break;

				case AddressType::FileOffset:	// To File offset
					Edit_SetText(m_hEditConvertAddrTo,
								 DWORD_toString(m_PEReaderWriter.RVAToFileOffset(DWORD(addrBufferFrom)), Hexadecimal).c_str());
					break;
			}
			break;

		case AddressType::VA:	// From VA
			switch (idxConvertAddrTo)
			{
				case AddressType::RVA:	// To RVA
					Edit_SetText(m_hEditConvertAddrTo,
								 DWORD_toString(m_PEReaderWriter.VAToRVA(addrBufferFrom), Hexadecimal).c_str());
					break;

				case AddressType::FileOffset:	// To File offset
					Edit_SetText(m_hEditConvertAddrTo,
								 DWORD_toString(m_PEReaderWriter.VAToFileOffset(addrBufferFrom), Hexadecimal).c_str());
					break;
			}
			break;

		case AddressType::FileOffset:	// From File offset
			switch (idxConvertAddrTo)
			{
				case AddressType::RVA:	// To RVA
					Edit_SetText(m_hEditConvertAddrTo,
								 DWORD_toString(m_PEReaderWriter.FileOffsetToRVA(DWORD(addrBufferFrom)), Hexadecimal).c_str());
					break;

				case AddressType::VA:	// To VA
					Edit_SetText(m_hEditConvertAddrTo,
								 DWORD_toString(m_PEReaderWriter.VAToFileOffset(addrBufferFrom), Hexadecimal).c_str());
					break;
			}
			break;
	}
}

void PropertyPageHandler_Tools::txtVerifyHash_Changed()
{
	// Get source hash
	WCHAR szStrFromTextbox[64] = { 0 };
	Edit_GetText(m_hEditVerifyHash, szStrFromTextbox, ARRAYSIZE(szStrFromTextbox));
	
	wstring Hash(szStrFromTextbox);
	if (Hash.empty())
	{
		SetWindowText(m_hStaticVerify, L"");
		RedrawWindow(m_hStaticVerify, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

		return;
	}

	// Make everything to lowercase
	std::transform(Hash.cbegin(), Hash.cend(), Hash.begin(), ::tolower);

	// Get SHA1 hash
	// Try to match SHA1 and MD5 hashes with source hash
	if (Hash == m_SHA1 || Hash == m_MD5)
		SetWindowText(m_hStaticVerify, L"\x2713"); // Show check sign in UNICODE
	else	// Else incorrect hash
		SetWindowText(m_hStaticVerify, L"\x274c"); // Show cross sign in UNICODE

	RedrawWindow(m_hStaticVerify, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
}

void PropertyPageHandler_Tools::txtHexViewerAddress_Changed()
{
	UpdateHexViewer();
}

void PropertyPageHandler_Tools::cmbHexViewAddrType_OnSelectionChanged(HWND hControl, int SelectedIndex)
{
	UpdateHexViewer();
}

void PropertyPageHandler_Tools::cmbHexViewDataSize_OnSelectionChanged(HWND hControl, int SelectedIndex)
{
	UpdateHexViewer();
}

void PropertyPageHandler_Tools::chkHexViewDisassemble_OnClick(HWND hControl, bool bChecked)
{
	m_View = (bChecked ? Viewer::Disassembly : Viewer::Hex);
	EnableWindow(m_hComboHexViewerDataSize, (bChecked ? FALSE : TRUE));

	UpdateHexViewer();
}

void PropertyPageHandler_Tools::UpdateHexViewer()
{
	WCHAR szStrFromTextbox[32] = { 0 };
	Edit_GetText(m_hEditHexViewerAddr, szStrFromTextbox, ARRAYSIZE(szStrFromTextbox));
	
	// Read the hex address from text box
	wstringstream ss;
	ULONGLONG addrHexViewer;

	ss << std::hex << szStrFromTextbox;
	ss >> addrHexViewer;
	if (ss.bad() || ss.fail())
		return;

	// Convert the address to file offset
	DWORD addrHexViewerInFileOffset;
	const AddressType typeViewAddress = static_cast<AddressType> (ComboBox_GetCurSel(m_hComboHexViewerAddr));
	switch (typeViewAddress)
	{
		case AddressType::RVA:
			if (addrHexViewer <= MAXDWORD)
			{
				addrHexViewerInFileOffset = m_PEReaderWriter.RVAToFileOffset(DWORD(addrHexViewer));
			}
			break;

		case AddressType::VA:
			if (addrHexViewer <= ULLONG_MAX)
			{
				addrHexViewerInFileOffset = m_PEReaderWriter.VAToFileOffset(addrHexViewer);
			}
			break;

		case AddressType::FileOffset:
			if (addrHexViewer <= MAXDWORD)
			{
				addrHexViewerInFileOffset = DWORD(addrHexViewer);
			}
			break;
	}
	
	// Depending on selected view show hex or disassembly
	wstring HexView;
	int err;
	switch (m_View)
	{
		case Viewer::Hex:
		{
			const PEReadWrite::HexViewType typeHexView = static_cast<PEReadWrite::HexViewType> (ComboBox_GetCurSel(m_hComboHexViewerDataSize));
			const DWORD MAX_BYTES_TO_SHOW = 512;
			err = m_PEReaderWriter.displayHexData(addrHexViewer, addrHexViewerInFileOffset, MAX_BYTES_TO_SHOW, typeHexView, std::ref(HexView));
		}
		break;

		case Viewer::Disassembly:
			err = m_PEReaderWriter.disassembleAt(addrHexViewer, addrHexViewerInFileOffset, std::ref(HexView));
			break;

		default:
			return;
	}

	if (err)
		HexView = L"<Invalid address>";

	Edit_SetText(m_hEditHexViewer, HexView.c_str());
}