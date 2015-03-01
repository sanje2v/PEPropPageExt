#include "stdafx.h"
#include "PropertyPageHandler.h"
#include <sstream>

#define SCALE(pos, addrspaceheight, layoutheight)		INT(max((((pos)/double(addrspaceheight)) * (layoutheight)), 1.0f))
#define SetColorOpacity(color, opacity)					((color) ^ Gdiplus::Color::AlphaMask) | (opacity)


PropertyPageHandler_Overview::PropertyPageHandler_Overview(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, std::ref(PEReaderWriter)),
	m_firstPageShow(true),
	m_gdiplusToken(0UL),
	m_HighestRVA(0),
	m_idxSelectionRect(-1),
	m_hTreeViewCustomItem(NULL),
	m_hBackgroundBrush(NULL, funcDeleteBrush)
{
	m_hTreeViewLegends = GetDlgItem(m_hWnd, IDC_TREELEGENDS);
	m_hStaticCustom = GetDlgItem(m_hWnd, IDC_STATICCUSTOM);
	m_hStaticCustomRVA = GetDlgItem(m_hWnd, IDC_STATICCUSTOMRVA);
	m_hStaticCustomSize = GetDlgItem(m_hWnd, IDC_STATICCUSTOMSIZE);
	m_hEditCustomRVA = GetDlgItem(m_hWnd, IDC_EDITCUSTOMRVA);
	m_hEditCustomSize = GetDlgItem(m_hWnd, IDC_EDITCUSTOMSIZE);

	// Set maximum characters that can be typed for edit boxes
	// NOTE: We allow two characters for hex '0x' and 8 characters for 32-bit hex numbers
	Edit_SetLimitText(m_hEditCustomRVA, 10);
	Edit_SetLimitText(m_hEditCustomSize, 10);

	// Set cue text for edit boxes
	Edit_SetCueBannerText(m_hEditCustomRVA, L"Address in hex");
	Edit_SetCueBannerText(m_hEditCustomSize, L"Size in dec");

	// Set default text
	SetWindowText(m_hEditCustomRVA, L"0x0");
	SetWindowText(m_hEditCustomSize, L"0");

	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

void PropertyPageHandler_Overview::OnInitDialog()
{
	// Setup controls
	m_LayoutManager.AddChildConstraint(IDC_TREELEGENDS, CWA_RIGHT, CWA_TOPBOTTOM);
	m_LayoutManager.AddChildConstraint(IDC_STATICCUSTOM, CWA_RIGHT, CWA_BOTTOM);
	m_LayoutManager.AddChildConstraint(IDC_STATICCUSTOMRVA);
	m_LayoutManager.AddChildConstraint(IDC_STATICCUSTOMSIZE);
	m_LayoutManager.AddChildConstraint(IDC_EDITCUSTOMRVA);
	m_LayoutManager.AddChildConstraint(IDC_EDITCUSTOMSIZE);
}

void PropertyPageHandler_Overview::OnShowWindow()
{	
	if (!m_firstPageShow)
		return;

	m_firstPageShow = false;

	// We prepare the drawing bitmap when the property page is shown
	//  the first time only. When redraw is required, we merely use this bitmap.

	// NOTE: This section had to be moved here because in 'OnInitDialog()'/'WM_INITDIALOG'
	//	the client area hasn't been stretched to cover all available property page area.
	RECT rectClient = { 0 };
	GetClientRect(m_hWnd, &rectClient);

	MEMORYLAYOUT_HEIGHT = rectClient.bottom - (MEMORYLAYOUT_Y / 2) * 3;
	
	// Create GDI+ Graphics and Bitmap objects
	unique_ptr<Gdiplus::Graphics> pGraphicsWindow(Gdiplus::Graphics::FromHWND(m_hWnd));
	m_pbitmapMemoryMap.reset(new Gdiplus::Bitmap(rectClient.right - rectClient.left,
		                                         rectClient.bottom - rectClient.top,
		                                         pGraphicsWindow.get()));
	unique_ptr<Gdiplus::Graphics> pGraphicsMemoryMap(Gdiplus::Graphics::FromImage(m_pbitmapMemoryMap.get()));
	
	// Create GDI+ pens
	Gdiplus::Pen penCornflowerBlue(Gdiplus::Color::CornflowerBlue);
	Gdiplus::Pen penAliceBlue(Gdiplus::Color::AliceBlue);
	Gdiplus::Pen penBlack(Gdiplus::Color::Black);
	Gdiplus::Pen penGray(Gdiplus::Color::AntiqueWhite);
	Gdiplus::Pen penBlockColor(Gdiplus::Color::Orchid);
	Gdiplus::Pen penGreen(Gdiplus::Color::Green);
	Gdiplus::FontFamily fontFamily(L"MS Shell Dlg");
	Gdiplus::Font font(&fontFamily, 11.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	Gdiplus::Font fontbold(&fontFamily, 11.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	Gdiplus::Rect rectMemoryLayout(MEMORYLAYOUT_X, MEMORYLAYOUT_Y, MEMORYLAYOUT_WIDTH, MEMORYLAYOUT_HEIGHT);

	// Find the highest RVA
	int err;
	for (WORD i = 0; i < m_PEReaderWriter.getNoOfSections(); ++i)
	{
		PIMAGE_SECTION_HEADER pSectionHeader;
		err = m_PEReaderWriter.getSectionHeader(i, std::ref(pSectionHeader));
		if (err)
		{
			LogError(L"ERROR: Couldn't read section header at index " + DWORD_toString(i) + L". File is not valid.", true);
			return;
		}

		DWORD CurrentHighestRVA = pSectionHeader->VirtualAddress + pSectionHeader->Misc.VirtualSize;
		m_HighestRVA = max(m_HighestRVA, CurrentHighestRVA);
	}

	// Flood fill everything to white
	pGraphicsMemoryMap->SetCompositingMode(Gdiplus::CompositingModeSourceOver);
	pGraphicsMemoryMap->Clear(Gdiplus::Color::White);
	m_hBackgroundBrush = CreateSolidBrush(RGB(255, 255, 255)); // NOTE: If you change the clear color above, change this too appropriately
	
	// Draw Memory layout rectangle
	pGraphicsMemoryMap->FillRectangle(penAliceBlue.GetBrush(), rectMemoryLayout);
	pGraphicsMemoryMap->DrawRectangle(&penCornflowerBlue, rectMemoryLayout);

	wstring BaseAddress, LastAddress;
	switch (m_PEReaderWriter.getPEType())
	{
		case PEReadWrite::PEType::PE32:
		{
			PIMAGE_NT_HEADERS32 pNTHeader;
			err = m_PEReaderWriter.getSecondaryHeader(std::ref(pNTHeader));
			assert(err == PEReadWrite::noErr);

			BaseAddress = DWORD_toString(pNTHeader->OptionalHeader.ImageBase, Hexadecimal);
			LastAddress = DWORD_toString(pNTHeader->OptionalHeader.ImageBase + m_HighestRVA, Hexadecimal);
		}
		break;

		case PEReadWrite::PEType::PE64:
		{
			PIMAGE_NT_HEADERS64 pNTHeader;
			err = m_PEReaderWriter.getSecondaryHeader(std::ref(pNTHeader));
			assert(err == PEReadWrite::noErr);

			BaseAddress = QWORD_toString(pNTHeader->OptionalHeader.ImageBase, Hexadecimal);
			LastAddress = QWORD_toString(pNTHeader->OptionalHeader.ImageBase + m_HighestRVA, Hexadecimal);
		}
		break;

		default:
			return;
	}

	// Draw some generic strings
	pGraphicsMemoryMap->DrawString(L"VA",
								   -1,
								   &fontbold,
								   Gdiplus::PointF(Gdiplus::REAL(MARGIN_X), Gdiplus::REAL(rectMemoryLayout.Y) - CAPTION_Y_DIFF),
								   penBlack.GetBrush());
	pGraphicsMemoryMap->DrawString(L"Memory Map",
								   -1,
								   &fontbold,
								   Gdiplus::PointF(Gdiplus::REAL(rectMemoryLayout.X), Gdiplus::REAL(rectMemoryLayout.Y) - CAPTION_Y_DIFF),
								   penBlack.GetBrush());
	pGraphicsMemoryMap->DrawString(BaseAddress.c_str(),
								   -1,
								   &font,
								   Gdiplus::PointF(Gdiplus::REAL(MARGIN_X), Gdiplus::REAL(rectMemoryLayout.Y)),
								   penBlack.GetBrush());
	pGraphicsMemoryMap->DrawString(LastAddress.c_str(),
								   -1,
								   &font,
								   Gdiplus::PointF(Gdiplus::REAL(MARGIN_X), Gdiplus::REAL(rectMemoryLayout.Y) + rectMemoryLayout.Height - CAPTION_Y_DIFF / 2),
								   penBlack.GetBrush());

	static LPWSTR const szGenericTreeviewItems[] =
	{
		L"MS-DOS Header",
		L"PE Header",
		L"Sections",
		L"Data directories",
		L"Custom Address"
	};
	TVINSERTSTRUCT Item;
	ZeroMemory(&Item, sizeof(Item));
	Item.hParent = TVI_ROOT;
	Item.hInsertAfter = TVI_LAST;
	Item.item.mask = TVIF_TEXT | LVIF_PARAM;

	// Draw MSDOS header rectangle
	static auto funcFillSelectionRectLabels = [](vector<SelectionRectData>& rect,
												 DWORD rva,
												 DWORD size,
												 INT x,
												 INT y,
												 INT width,
												 INT height) -> void
	{
		rect.push_back({ rva, size, { x, y, width, height } });
	};
	funcFillSelectionRectLabels(m_vectSelectionRects,
								0,
								sizeof(IMAGE_DOS_HEADER),
								BLOCK_X,
								BLOCK_START_Y + 0,
								BLOCK_WIDTH,
								SCALE(sizeof(IMAGE_DOS_HEADER), m_HighestRVA, MEMORYLAYOUT_HEIGHT));
	pGraphicsMemoryMap->FillRectangle(penCornflowerBlue.GetBrush(), m_vectSelectionRects.back().R);

	Item.item.pszText = szGenericTreeviewItems[0];
	Item.item.cchTextMax = int(wcslen(szGenericTreeviewItems[0]));
	Item.item.lParam = LPARAM(0);
	TreeView_InsertItem(m_hTreeViewLegends, &Item);

	// Draw PE header rectangle
	PIMAGE_DOS_HEADER pMSDOSHeader;
	err = m_PEReaderWriter.getPrimaryHeader(std::ref(pMSDOSHeader));
	assert(err == PEReadWrite::noErr);

	int PEHeader_FileOffset = pMSDOSHeader->e_lfanew;
	int ColorCount = 0;
	funcFillSelectionRectLabels(m_vectSelectionRects,
								PEHeader_FileOffset,
								sizeof(IMAGE_NT_HEADERS),
								BLOCK_X,
								BLOCK_START_Y + SCALE(PEHeader_FileOffset, m_HighestRVA, MEMORYLAYOUT_HEIGHT),
								BLOCK_WIDTH,
								SCALE(sizeof(IMAGE_NT_HEADERS), m_HighestRVA, MEMORYLAYOUT_HEIGHT));
	pGraphicsMemoryMap->FillRectangle(RotatePenColor(std::ref(penBlockColor), std::ref(ColorCount)).GetBrush(), m_vectSelectionRects.back().R);

	Item.item.pszText = szGenericTreeviewItems[1];
	Item.item.cchTextMax = int(wcslen(szGenericTreeviewItems[1]));
	Item.item.lParam = LPARAM(1);
	TreeView_InsertItem(m_hTreeViewLegends, &Item);

	// Draw Sections rectangle
	Item.item.pszText = szGenericTreeviewItems[2];
	Item.item.cchTextMax = int(wcslen(szGenericTreeviewItems[2]));
	Item.item.lParam = LPARAM(-1);
	Item.hParent = TreeView_InsertItem(m_hTreeViewLegends, &Item);

	for (int i = 0; i < m_PEReaderWriter.getNoOfSections(); ++i)
	{
		PIMAGE_SECTION_HEADER pSectionHeader;
		err = m_PEReaderWriter.getSectionHeader(i, std::ref(pSectionHeader));
		assert(err == PEReadWrite::noErr);

		funcFillSelectionRectLabels(m_vectSelectionRects,
									pSectionHeader->VirtualAddress,
									pSectionHeader->Misc.VirtualSize,
									BLOCK_X,
									BLOCK_START_Y + SCALE(pSectionHeader->VirtualAddress, m_HighestRVA, MEMORYLAYOUT_HEIGHT),
									BLOCK_WIDTH,
									SCALE(pSectionHeader->Misc.VirtualSize, m_HighestRVA, MEMORYLAYOUT_HEIGHT));
		pGraphicsMemoryMap->FillRectangle(RotatePenColor(std::ref(penBlockColor), std::ref(ColorCount)).GetBrush(), m_vectSelectionRects.back().R);

		Item.item.pszText = LPWSTR(_wcsdup(wstring(DWORD_toString(i + 1) + L": \"" + ProperSectionName(pSectionHeader->Name) + L'\"').c_str()));
		Item.item.cchTextMax = int(wcslen(Item.item.pszText));
		Item.item.lParam = LPARAM(i + 2);
		TreeView_InsertItem(m_hTreeViewLegends, &Item);
		free(Item.item.pszText);
	}

	// Draw Data directories
	Item.hParent = TVI_ROOT;
	Item.item.pszText = szGenericTreeviewItems[3];
	Item.item.cchTextMax = int(wcslen(szGenericTreeviewItems[3]));
	Item.item.lParam = LPARAM(-1);
	Item.hParent = TreeView_InsertItem(m_hTreeViewLegends, &Item);

	static const LPWSTR szDataDirectories[] =
	{
		L"Export Table", L"Import Table", L"Resource Table", L"Exception Table",
		L"Certificate Data", L"Base Reloc Table", L"Debug Data", L"Architecture Data",
		L"Global Ptr", L"TLS Table", L"Load Config Table", L"Bound Table",
		L"Import Addr Table", L"Delay Import Desc", L"CLR Runtime Header", L"Reserved Data"
	};

	for (DWORD i = 0; i < m_PEReaderWriter.getNoOfDataDirectories_Corrected(); ++i)
	{
		if (i == 4)
			continue;	// Ignore 'Certificate Data' because it's a file offset

		PIMAGE_DATA_DIRECTORY pDataDir;
		err = m_PEReaderWriter.getDataDirectory(i, std::ref(pDataDir));
		if (err)
		{
			LogError(L"ERROR: Couldn't read data directory with index " + DWORD_toString(i) + L". File is not valid.");
			break;
		}

		if (pDataDir->Size == 0)
			continue;	// Ignore empty data directory references

		funcFillSelectionRectLabels(m_vectSelectionRects,
									pDataDir->VirtualAddress,
									pDataDir->Size,
									BLOCK_X,
									BLOCK_START_Y + SCALE(pDataDir->VirtualAddress, m_HighestRVA, MEMORYLAYOUT_HEIGHT),
									BLOCK_WIDTH,
									SCALE(pDataDir->Size, m_HighestRVA, MEMORYLAYOUT_HEIGHT));
		pGraphicsMemoryMap->FillRectangle(RotatePenColor(std::ref(penBlockColor), std::ref(ColorCount)).GetBrush(), m_vectSelectionRects.back().R);

		Item.item.pszText = szDataDirectories[i];
		Item.item.cchTextMax = int(wcslen(szDataDirectories[i]));
		Item.item.lParam = LPARAM(m_vectSelectionRects.size() - 1);
		TreeView_InsertItem(m_hTreeViewLegends, &Item);
	}

	// Draw Custom
	funcFillSelectionRectLabels(m_vectSelectionRects, 0, 0, 0, 0, 0, 0);

	Item.hParent = TVI_ROOT;
	Item.item.pszText = szGenericTreeviewItems[4];
	Item.item.cchTextMax = int(wcslen(szGenericTreeviewItems[4]));
	Item.item.lParam = LPARAM(m_vectSelectionRects.size() - 1);
	m_hTreeViewCustomItem = TreeView_InsertItem(m_hTreeViewLegends, &Item);
}

void PropertyPageHandler_Overview::OnPaint(HDC hdc, const RECT& rectUpdate)
{
	Gdiplus::Graphics g(hdc);
	g.SetClip(Gdiplus::Rect(rectUpdate.left, rectUpdate.top,
							rectUpdate.right - rectUpdate.left,
							rectUpdate.bottom - rectUpdate.top));

	Gdiplus::Pen pen(Gdiplus::Color::Black, 2.0f);

	g.DrawImage(m_pbitmapMemoryMap.get(), 0, 0);			// Draw memory map

	if (m_idxSelectionRect >= 0)
	{
		Gdiplus::FontFamily fontFamily(L"MS Shell Dlg");
		Gdiplus::Font font(&fontFamily, 11.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Gdiplus::Pen penBlack(Gdiplus::Color::Black);
		Gdiplus::RectF rectClient;
		rectClient.X = rectClient.Y = 0.0f;
		rectClient.Width = rectClient.Height = 100.0f;
		Gdiplus::RectF rectBoundingBox;
		wstring StartAddress, EndAddress;
		int EndAddress_Y = m_vectSelectionRects[m_idxSelectionRect].R.Y;

		// Draw selection rectangle
		g.DrawRectangle(&pen, m_vectSelectionRects[m_idxSelectionRect].R);
		
		// Draw start/end virtual address
		int err;
		switch (m_PEReaderWriter.getPEType())
		{
			case PEReadWrite::PEType::PE32:
			{
				PIMAGE_NT_HEADERS32 pPEHeader;
				err = m_PEReaderWriter.getSecondaryHeader(std::ref(pPEHeader));
				assert(err == PEReadWrite::noErr);

				StartAddress = DWORD_toString(pPEHeader->OptionalHeader.ImageBase + m_vectSelectionRects[m_idxSelectionRect].RVA, Hexadecimal);
				g.MeasureString(StartAddress.c_str(), -1, &font, rectClient, &rectBoundingBox);
				EndAddress = DWORD_toString(pPEHeader->OptionalHeader.ImageBase + m_vectSelectionRects[m_idxSelectionRect].RVA + m_vectSelectionRects[m_idxSelectionRect].Size, Hexadecimal);
			}
			break;

			case PEReadWrite::PEType::PE64:
			{
				PIMAGE_NT_HEADERS64 pPEHeader;
				err = m_PEReaderWriter.getSecondaryHeader(std::ref(pPEHeader));
				assert(err == PEReadWrite::noErr);

				StartAddress = QWORD_toString(pPEHeader->OptionalHeader.ImageBase + m_vectSelectionRects[m_idxSelectionRect].RVA, Hexadecimal);
				g.MeasureString(StartAddress.c_str(), -1, &font, rectClient, &rectBoundingBox);
				EndAddress = QWORD_toString(pPEHeader->OptionalHeader.ImageBase + m_vectSelectionRects[m_idxSelectionRect].RVA + m_vectSelectionRects[m_idxSelectionRect].Size, Hexadecimal);
			}
			break;

			default:
				return;
		}

		EndAddress_Y += int(rectBoundingBox.Height > m_vectSelectionRects[m_idxSelectionRect].R.Height ? rectBoundingBox.Height : m_vectSelectionRects[m_idxSelectionRect].R.Height);

		g.DrawString(StartAddress.c_str(),
					 -1,
					 &font,
					 Gdiplus::PointF(Gdiplus::REAL(SELECTION_RECT_ADDR_X), Gdiplus::REAL(m_vectSelectionRects[m_idxSelectionRect].R.Y) - rectBoundingBox.Height / 2),
					 penBlack.GetBrush());
		g.DrawString(EndAddress.c_str(),
					 -1,
					 &font,
					 Gdiplus::PointF(Gdiplus::REAL(SELECTION_RECT_ADDR_X), Gdiplus::REAL(EndAddress_Y) - rectBoundingBox.Height / 2),
					 penBlack.GetBrush());
	}
}

void PropertyPageHandler_Overview::tvwLegends_OnSelection(HWND hControl, NMTVITEMCHANGE *pItemChange)
{
	m_idxSelectionRect = int(pItemChange->lParam);

	if (pItemChange->hItem == m_hTreeViewCustomItem)
		UpdateCustomAddressDrawingBox();

	RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
}

void PropertyPageHandler_Overview::txtCustomRVA_OnLostFocus(HWND hControl)
{
	if (GetFocus() == m_hTreeViewLegends)
	{
		TVITEM Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = TVIF_HANDLE | TVIF_STATE;
		Item.hItem = m_hTreeViewCustomItem;
		Item.stateMask = TVIS_SELECTED;

		TreeView_GetItem(m_hTreeViewLegends, &Item);

		if (TestFlag(Item.state, TVIS_SELECTED))
			UpdateCustomAddressDrawingBox(true);
	}
}

void PropertyPageHandler_Overview::txtCustomSize_OnLostFocus(HWND hControl)
{
	if (GetFocus() == m_hTreeViewLegends)
	{
		TVITEM Item;
		ZeroMemory(&Item, sizeof(Item));
		Item.mask = TVIF_HANDLE | TVIF_STATE;
		Item.hItem = m_hTreeViewCustomItem;
		Item.stateMask = TVIS_SELECTED;

		TreeView_GetItem(m_hTreeViewLegends, &Item);

		if (TestFlag(Item.state, TVIS_SELECTED))
			UpdateCustomAddressDrawingBox(true);
	}
}

void PropertyPageHandler_Overview::OnDestroy()
{
	if (m_gdiplusToken)
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

void PropertyPageHandler_Overview::UpdateCustomAddressDrawingBox(bool bInvalidate)
{
	// Set RVA, size data and compute rectangle
	WCHAR szEditText[32] = { 0 };

	Edit_GetText(m_hEditCustomRVA, szEditText, ARRAYSIZE(szEditText));
		
	wstringstream ss;
	DWORD CustomRVA = 0, CustomSize = 0;
		
	// Convert hexadecimal string to integer
	ss << hex << szEditText;
	if (ss.fail() || ss.bad())
	{
		LogError(L"Please enter a valid hexadecimal custom address!", true);

		return;
	}

	// Make sure input is valid
	wstring EditText(szEditText);
	if (EditText.empty())
		return;

	if (EditText.find_first_not_of(L"0123456789abcdefABCDEF",
		EditText.substr(0, 2) == L"0x" ||
		EditText.substr(0, 2) == L"0X" ? 2 : 0) != wstring::npos)
	{
		LogError(L"Please enter a valid hexadecimal address!", true);

		return;
	}

	if (!(EditText.substr(0, 2) == L"0x" ||
		  EditText.substr(0, 2) == L"0X"))
	{
		EditText = L"0x" + EditText;

		Edit_SetText(m_hEditCustomRVA, EditText.c_str());
	}

	// Extract integer RVA
	ss >> CustomRVA;

	// Get size text
	Edit_GetText(m_hEditCustomSize, szEditText, ARRAYSIZE(szEditText));

	ss.clear();
	ss << dec << szEditText;
	if (ss.fail() || ss.bad())
	{
		LogError(L"Please enter a valid decimal custom size!", true);

		return;
	}

	// Convert decimal string to integer
	ss >> CustomSize;

	m_vectSelectionRects[m_idxSelectionRect].RVA = min(CustomRVA, m_HighestRVA);
	m_vectSelectionRects[m_idxSelectionRect].Size = (CustomRVA >= m_HighestRVA ? 0 : CustomSize);
	m_vectSelectionRects[m_idxSelectionRect].R =
	{
		BLOCK_X,
		BLOCK_START_Y + SCALE(m_vectSelectionRects[m_idxSelectionRect].RVA, m_HighestRVA, MEMORYLAYOUT_HEIGHT),
		BLOCK_WIDTH,
		SCALE(m_vectSelectionRects[m_idxSelectionRect].Size, m_HighestRVA, MEMORYLAYOUT_HEIGHT)
	};

	if (bInvalidate)
		RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);	// Invoke repaint event
}

Gdiplus::Pen& PropertyPageHandler_Overview::RotatePenColor(Gdiplus::Pen& pen, int& Count)
{
	static const unsigned long ALPHA_LEVEL = 0xC0000000L;	// 75% Opacity
	static const Gdiplus::Color ColorCodes[] =
	{
		Gdiplus::Color::BlueViolet, Gdiplus::Color::DarkSalmon, Gdiplus::Color::DarkSeaGreen,
		Gdiplus::Color::Coral, Gdiplus::Color::DarkCyan, Gdiplus::Color::Crimson,
		Gdiplus::Color::CadetBlue, Gdiplus::Color::Chocolate, Gdiplus::Color::DarkRed,
		Gdiplus::Color::DarkGray, Gdiplus::Color::DarkOliveGreen, Gdiplus::Color::DarkOrange,
		Gdiplus::Color::DarkSlateBlue, Gdiplus::Color::DeepPink, Gdiplus::Color::SkyBlue,
		Gdiplus::Color::DimGray, Gdiplus::Color::DodgerBlue, Gdiplus::Color::Firebrick
	};

	if (Count < ARRAYSIZE(ColorCodes))
	{
		pen.SetColor(SetColorOpacity(ColorCodes[Count++].GetValue(), ALPHA_LEVEL));

		return pen;
	}

	Gdiplus::Color color;

	pen.GetColor(&color);
	pen.SetColor(Gdiplus::Color(color.GetAlpha(), color.GetR() - 20, color.GetG() + 18, color.GetB() - 25));

	return pen;
}