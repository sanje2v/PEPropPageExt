#include "PropertyPageHandler.h"
#include <sstream>


#define FillData2(vectorobj, rva, size, ...)			(vectorobj).push_back(SelectionRectData(rva, size, Rect(__VA_ARGS__)))
#define SCALE(pos, addrspaceheight, layoutheight)		(INT) max((((pos)/(double) (addrspaceheight)) * (layoutheight)), 1)
#define SetColorOpacity(color, opacity)					((color) ^ Color::AlphaMask) | (opacity)


static const unsigned long ALPHA_LEVEL = 0xC0000000L;	// 75% Opacity


Gdiplus::Pen& RotatePenColor(Gdiplus::Pen& pen);


PropertyPageHandler_Overview::PropertyPageHandler_Overview(HWND hWnd, PEReadWrite& PEReaderWriter)
		: PropertyPageHandler(hWnd, PEReaderWriter),
			m_gdiplusToken(0), m_pbitmapMemoryMap(NULL),
			m_HighestRVA(0), m_SelectionRectIndex(-1),
			m_hTreeViewCustomItem(NULL)
{
	m_hTreeViewLegends = GetDlgItem(m_hWnd, IDC_TREELEGENDS);
	m_hStaticCustom = GetDlgItem(m_hWnd, IDC_STATICCUSTOM);
	m_hStaticCustomRVA = GetDlgItem(m_hWnd, IDC_STATICCUSTOMRVA);
	m_hStaticCustomSize = GetDlgItem(m_hWnd, IDC_STATICCUSTOMSIZE);
	m_hEditCustomRVA = GetDlgItem(m_hWnd, IDC_EDITCUSTOMRVA);
	m_hEditCustomSize = GetDlgItem(m_hWnd, IDC_EDITCUSTOMSIZE);

	// Initialize GDI+
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}
void PropertyPageHandler_Overview::OnInitDialog()
{}

void PropertyPageHandler_Overview::OnShowWindow()
{
	THREAD_ISOLATED_STORAGE static bool bExecutedOnce = false;

	if (bExecutedOnce)
		return;

	// NOTE: This section had to be moved here because in 'OnInitDialog'
	//	the client area hasn't been set yet
	RECT rectClient;
	GetClientRect(m_hWnd, &rectClient);

	MEMORYLAYOUT_HEIGHT = rectClient.bottom - MEMORYLAYOUT_Y * 3 / 2;
	LEGENDS_WIDTH = rectClient.right - LEGENDS_X - MARGIN_X;
	
	unique_ptr<Graphics> pGraphicsWindow(Graphics::FromHWND(m_hWnd));
	m_pbitmapMemoryMap = new Bitmap(rectClient.right - rectClient.left,
									rectClient.bottom - rectClient.top,
									pGraphicsWindow.get());
	unique_ptr<Graphics> pGraphicsMemoryMap(Graphics::FromImage(m_pbitmapMemoryMap));
	
	Pen penCornflowerBlue(Color::CornflowerBlue);
	Pen penAliceBlue(Color::AliceBlue);
	Pen penBlack(Color::Black);
	Pen penGray(Color::AntiqueWhite);
	Pen penBlockColor(Color::Orchid);
	Pen penGreen(Color::Green);
	FontFamily fontFamily(L"MS Shell Dlg");
	Font font(&fontFamily, 11.0f, FontStyleRegular, UnitPixel);
	Font fontbold(&fontFamily, 11.0f, FontStyleBold, UnitPixel);
	Rect rectMemoryLayout(MEMORYLAYOUT_X, MEMORYLAYOUT_Y, MEMORYLAYOUT_WIDTH, MEMORYLAYOUT_HEIGHT);

	// Find the highest RVA
	for (int i = 0; i < m_PEReaderWriter.GetNoOfSections(); i++)
	{
		DWORD CurrentHighestRVA = m_PEReaderWriter.GetSectionHeader(i)->VirtualAddress +
									m_PEReaderWriter.GetSectionHeader(i)->Misc.VirtualSize;
		if (m_HighestRVA < CurrentHighestRVA)
			m_HighestRVA = CurrentHighestRVA;
	}

	// Flood fill everything to white
	pGraphicsMemoryMap->SetCompositingMode(CompositingModeSourceOver);
	pGraphicsMemoryMap->Clear(Color::White);	
	
	// Draw Memory layout rectangle
	pGraphicsMemoryMap->FillRectangle(penAliceBlue.GetBrush(), rectMemoryLayout);
	pGraphicsMemoryMap->DrawRectangle(&penCornflowerBlue, rectMemoryLayout);

	tstring BaseAddress, LastAddress;

	switch (m_PEReaderWriter.GetPEType())
	{
	case PEReadWrite::PE32:
		{
			PIMAGE_NT_HEADERS32 pNTHeader = m_PEReaderWriter.GetSecondaryHeader<PIMAGE_NT_HEADERS32>();
			BaseAddress = DWORD_toString(pNTHeader->OptionalHeader.ImageBase, Hexadecimal);
			LastAddress = DWORD_toString(pNTHeader->OptionalHeader.ImageBase + m_HighestRVA, Hexadecimal);
		}

		break;

	case PEReadWrite::PE64:
		{
			PIMAGE_NT_HEADERS64 pNTHeader = m_PEReaderWriter.GetSecondaryHeader<PIMAGE_NT_HEADERS64>();
			BaseAddress = QWORD_toString(pNTHeader->OptionalHeader.ImageBase, Hexadecimal);
			LastAddress = QWORD_toString(pNTHeader->OptionalHeader.ImageBase + m_HighestRVA, Hexadecimal);
		}
	}

	// Draw some generic strings
	pGraphicsMemoryMap->DrawString(L"VA", -1, &fontbold, PointF((REAL) MARGIN_X, (REAL) rectMemoryLayout.Y - CAPTION_Y_DIFF), penBlack.GetBrush());
	pGraphicsMemoryMap->DrawString(L"Memory Map", -1, &fontbold, PointF((REAL) rectMemoryLayout.X, (REAL) rectMemoryLayout.Y - CAPTION_Y_DIFF), penBlack.GetBrush());
	pGraphicsMemoryMap->DrawString(L"Legends", -1, &fontbold, PointF((REAL) LEGENDS_X, (REAL) LEGENDS_Y - CAPTION_Y_DIFF), penBlack.GetBrush());
	pGraphicsMemoryMap->DrawString(BaseAddress.c_str(), -1, &font, PointF((REAL) MARGIN_X, (REAL) rectMemoryLayout.Y), penBlack.GetBrush());
	pGraphicsMemoryMap->DrawString(LastAddress.c_str(), -1, &font, PointF((REAL) MARGIN_X, (REAL) rectMemoryLayout.Y + rectMemoryLayout.Height - CAPTION_Y_DIFF / 2), penBlack.GetBrush());

	static LPTSTR const GenericTreeviewItems[] = { _T("MS-DOS Header"),
													_T("PE Header"),
													_T("Sections"),
													_T("Data directories"),
													_T("Custom Address") };
	TVINSERTSTRUCT Item;
	ZeroMemory(&Item, sizeof(TVINSERTSTRUCT));
	Item.hParent = TVI_ROOT;
	Item.hInsertAfter = TVI_LAST;
	Item.item.mask = TVIF_TEXT | LVIF_PARAM;

	// Draw MSDOS header rectangle
	FillData2(m_vectorSelectionRects,
				0,
				sizeof(IMAGE_DOS_HEADER),
				BLOCK_X,
				BLOCK_START_Y + 0,
				BLOCK_WIDTH,
				SCALE(sizeof(IMAGE_DOS_HEADER), m_HighestRVA, MEMORYLAYOUT_HEIGHT));
	pGraphicsMemoryMap->FillRectangle(penCornflowerBlue.GetBrush(), m_vectorSelectionRects.back().R);

	Item.item.pszText = GenericTreeviewItems[0];
	Item.item.cchTextMax = (int) _tcslen(GenericTreeviewItems[0]);
	Item.item.lParam = 0;
	TreeView_InsertItem(m_hTreeViewLegends, &Item);

	// Draw PE header rectangle
	int PEHeader_FileOffset = m_PEReaderWriter.GetPrimaryHeader<PIMAGE_DOS_HEADER>()->e_lfanew;
	FillData2(m_vectorSelectionRects,
				PEHeader_FileOffset,
				sizeof(IMAGE_NT_HEADERS),
				BLOCK_X,
				BLOCK_START_Y + SCALE(PEHeader_FileOffset, m_HighestRVA, MEMORYLAYOUT_HEIGHT),
				BLOCK_WIDTH,
				SCALE(sizeof(IMAGE_NT_HEADERS), m_HighestRVA, MEMORYLAYOUT_HEIGHT));
	pGraphicsMemoryMap->FillRectangle(RotatePenColor(penBlockColor).GetBrush(), m_vectorSelectionRects.back().R);

	Item.item.pszText = GenericTreeviewItems[1];
	Item.item.cchTextMax = (int) _tcslen(GenericTreeviewItems[1]);
	Item.item.lParam = 1;
	TreeView_InsertItem(m_hTreeViewLegends, &Item);

	// Draw Sections rectangle
	Item.item.pszText = GenericTreeviewItems[2];
	Item.item.cchTextMax = (int) _tcslen(GenericTreeviewItems[2]);
	Item.item.lParam = -1;
	Item.hParent = TreeView_InsertItem(m_hTreeViewLegends, &Item);

	for (int i = 0; i < m_PEReaderWriter.GetNoOfSections(); i++)
	{
		PIMAGE_SECTION_HEADER pSectionHeader = m_PEReaderWriter.GetSectionHeader(i);
		FillData2(m_vectorSelectionRects,
					pSectionHeader->VirtualAddress,
					pSectionHeader->Misc.VirtualSize,
					BLOCK_X,
					BLOCK_START_Y + SCALE(pSectionHeader->VirtualAddress, m_HighestRVA, MEMORYLAYOUT_HEIGHT),
					BLOCK_WIDTH,
					SCALE(pSectionHeader->Misc.VirtualSize, m_HighestRVA, MEMORYLAYOUT_HEIGHT));

		pGraphicsMemoryMap->FillRectangle(RotatePenColor(penBlockColor).GetBrush(), m_vectorSelectionRects.back().R);

		tstring buffer = DWORD_toString(i + 1) + _T(": \"") + ProperSectionName(pSectionHeader->Name) + _T("\"");
		Item.item.pszText = (LPTSTR) buffer.c_str();
		Item.item.cchTextMax = (int) buffer.size();
		Item.item.lParam = i + 2;
		TreeView_InsertItem(m_hTreeViewLegends, &Item);
	}

	// Draw Data directories
	Item.hParent = TVI_ROOT;
	Item.item.pszText = GenericTreeviewItems[3];
	Item.item.cchTextMax = (int) _tcslen(GenericTreeviewItems[3]);
	Item.item.lParam = -1;
	Item.hParent = TreeView_InsertItem(m_hTreeViewLegends, &Item);

	LPTSTR DataDirectories[] = { _T("Export Table"), _T("Import Table"), _T("Resource Table"),
									_T("Exception Table"), _T("Certificate Data"), _T("Base Reloc Table"), _T("Debug Data"),
									_T("Architecture Data"), _T("Global Ptr"), _T("TLS Table"),
									_T("Load Config Table"), _T("Bound Table"), _T("Import Addr Table"),
									_T("Delay Import Desc"), _T("CLR Runtime Header"), _T("Reserved Data") };

	for (DWORD i = 0; i < m_PEReaderWriter.GetNoOfDataDirectories(); i++)
	{
		if (i == 4) continue;	// Ignore 'Certificate Data' because it's a file offset

		PIMAGE_DATA_DIRECTORY pDataDir = m_PEReaderWriter.GetDataDirectory(i);

		if (pDataDir->Size == 0) continue;	// Ignore empty data directory references

		FillData2(m_vectorSelectionRects,
					pDataDir->VirtualAddress,
					pDataDir->Size,
					BLOCK_X,
					BLOCK_START_Y + SCALE(pDataDir->VirtualAddress, m_HighestRVA, MEMORYLAYOUT_HEIGHT),
					BLOCK_WIDTH,
					SCALE(pDataDir->Size, m_HighestRVA, MEMORYLAYOUT_HEIGHT));

		pGraphicsMemoryMap->FillRectangle(RotatePenColor(penBlockColor).GetBrush(), m_vectorSelectionRects.back().R);

		Item.item.pszText = DataDirectories[i];
		Item.item.cchTextMax = (int) _tcslen(DataDirectories[i]);
		Item.item.lParam = (int) m_vectorSelectionRects.size() - 1;
		TreeView_InsertItem(m_hTreeViewLegends, &Item);
	}

	// Draw Custom
	FillData2(m_vectorSelectionRects, 0, 0, 0, 0, 0, 0);

	Item.hParent = TVI_ROOT;
	Item.item.pszText = GenericTreeviewItems[4];
	Item.item.cchTextMax = (int) _tcslen(GenericTreeviewItems[4]);
	Item.item.lParam = (int) m_vectorSelectionRects.size() - 1;
	m_hTreeViewCustomItem = TreeView_InsertItem(m_hTreeViewLegends, &Item);

	// Handle Legends tree view
	// Move control into position
	MoveWindow(m_hTreeViewLegends, LEGENDS_X, LEGENDS_Y, LEGENDS_WIDTH, LEGENDS_HEIGHT, TRUE);

	// Handle Custom static and text boxes
	RECT rectStaticCustom, rectEditCustom;

	GetWindowRect(m_hStaticCustom, &rectStaticCustom);
	GetWindowRect(m_hEditCustomRVA, &rectEditCustom);

	SetWindowPos(m_hStaticCustom, HWND_TOP, CUSTOM_X, CUSTOM_Y, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	SetWindowPos(m_hStaticCustomRVA, HWND_TOP, CUSTOM_X, CUSTOM_Y + rectStaticCustom.bottom - rectStaticCustom.top + (rectEditCustom.bottom - rectEditCustom.top) / 2, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	SetWindowPos(m_hStaticCustomSize, HWND_TOP, CUSTOM_X, CUSTOM_Y + (rectStaticCustom.bottom - rectStaticCustom.top) * 2 + (rectEditCustom.bottom - rectEditCustom.top) + 2, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	MoveWindow(m_hEditCustomRVA, CUSTOM_X + 30, CUSTOM_Y + (rectStaticCustom.bottom - rectStaticCustom.top) * 3 / 2, rectClient.right - CUSTOM_X - 30 - MARGIN_X, rectEditCustom.bottom - rectEditCustom.top, FALSE);
	MoveWindow(m_hEditCustomSize, CUSTOM_X + 30, CUSTOM_Y + (rectStaticCustom.bottom - rectStaticCustom.top) * 3 + 6, rectClient.right - CUSTOM_X - 30 - MARGIN_X, rectEditCustom.bottom - rectEditCustom.top, FALSE);

	// Set default text
	SetWindowText(m_hEditCustomRVA, _T("0x0"));
	SetWindowText(m_hEditCustomSize, _T("0"));

	bExecutedOnce = true;
}

void PropertyPageHandler_Overview::OnPaint(HDC hdc, const RECT& rectUpdate)
{
	Graphics g(hdc);
	g.SetClip(Rect(rectUpdate.left, rectUpdate.top,
					rectUpdate.right - rectUpdate.left,
					rectUpdate.bottom - rectUpdate.top));

	Pen pen(Color::Black, 2.0f);

	g.DrawImage(m_pbitmapMemoryMap, 0, 0);			// Draw memory map

	if (m_SelectionRectIndex >= 0)
	{
		FontFamily fontFamily(L"MS Shell Dlg");
		Font font(&fontFamily, 11.0f, FontStyleRegular, UnitPixel);
		Pen penBlack(Color::Black);
		tstring StartAddress, EndAddress;
		RectF rectClient;
		rectClient.X = rectClient.Y = 0.0f;
		rectClient.Width = rectClient.Height = 100.0f;
		RectF rectBoundingBox;
		int EndAddress_Y = m_vectorSelectionRects[m_SelectionRectIndex].R.Y;

		// Draw selection rectangle
		g.DrawRectangle(&pen, m_vectorSelectionRects[m_SelectionRectIndex].R);
		
		// Draw start/end virtual address
		switch (m_PEReaderWriter.GetPEType())
		{
		case PEReadWrite::PE32:
			{
				PIMAGE_NT_HEADERS32 pPEHeader = m_PEReaderWriter.GetSecondaryHeader<PIMAGE_NT_HEADERS32>();
				StartAddress = DWORD_toString(pPEHeader->OptionalHeader.ImageBase + m_vectorSelectionRects[m_SelectionRectIndex].RVA, Hexadecimal);
				
				g.MeasureString(StartAddress.c_str(), -1, &font, rectClient, &rectBoundingBox);
				g.DrawString(StartAddress.c_str(),
								-1, &font, PointF((REAL) SELECTION_RECT_ADDR_X, (REAL) m_vectorSelectionRects[m_SelectionRectIndex].R.Y - rectBoundingBox.Height / 2), penBlack.GetBrush());

				EndAddress_Y += (int) (rectBoundingBox.Height > m_vectorSelectionRects[m_SelectionRectIndex].R.Height ? rectBoundingBox.Height : m_vectorSelectionRects[m_SelectionRectIndex].R.Height);

				EndAddress = DWORD_toString(pPEHeader->OptionalHeader.ImageBase + m_vectorSelectionRects[m_SelectionRectIndex].RVA + m_vectorSelectionRects[m_SelectionRectIndex].Size, Hexadecimal);
				g.DrawString(EndAddress.c_str(), -1, &font, PointF((REAL) SELECTION_RECT_ADDR_X, (REAL) EndAddress_Y - rectBoundingBox.Height / 2), penBlack.GetBrush());
			}

			break;

		case PEReadWrite::PE64:
			{
				PIMAGE_NT_HEADERS64 pPEHeader = m_PEReaderWriter.GetSecondaryHeader<PIMAGE_NT_HEADERS64>();
				StartAddress = QWORD_toString(pPEHeader->OptionalHeader.ImageBase + m_vectorSelectionRects[m_SelectionRectIndex].RVA, Hexadecimal);
				
				g.MeasureString(StartAddress.c_str(), -1, &font, rectClient, &rectBoundingBox);
				g.DrawString(QWORD_toString(pPEHeader->OptionalHeader.ImageBase + m_vectorSelectionRects[m_SelectionRectIndex].RVA, Hexadecimal).c_str(),
								-1, &font, PointF((REAL) SELECTION_RECT_ADDR_X, (REAL) m_vectorSelectionRects[m_SelectionRectIndex].R.Y - rectBoundingBox.Height / 2), penBlack.GetBrush());
				
				EndAddress_Y += (int) (rectBoundingBox.Height > m_vectorSelectionRects[m_SelectionRectIndex].R.Height ? rectBoundingBox.Height : m_vectorSelectionRects[m_SelectionRectIndex].R.Height);

				EndAddress = QWORD_toString(pPEHeader->OptionalHeader.ImageBase + m_vectorSelectionRects[m_SelectionRectIndex].RVA + m_vectorSelectionRects[m_SelectionRectIndex].Size, Hexadecimal);
				g.DrawString(EndAddress.c_str(), -1, &font, PointF((REAL) SELECTION_RECT_ADDR_X, (REAL) EndAddress_Y - rectBoundingBox.Height / 2), penBlack.GetBrush());
			}
		}
	}
}

void PropertyPageHandler_Overview::tvwLegends_OnSelection(HWND hControl, NMTVITEMCHANGE *pItemChange)
{
	m_SelectionRectIndex = (int) pItemChange->lParam;

	if (pItemChange->hItem == m_hTreeViewCustomItem)
	{
		// Set RVA, size data and compute rectangle
		TCHAR szBuffer[64];

		Edit_GetText(m_hEditCustomRVA, szBuffer, GetArraySize(szBuffer));
		
		tstringstream ss;
		DWORD CustomRVA = 0, CustomSize = 0;
		
		ss << hex << szBuffer;
		if (ss.fail())
			return;
		ss >> CustomRVA;

		ss.clear();
		Edit_GetText(m_hEditCustomSize, szBuffer, GetArraySize(szBuffer));
		ss << dec << szBuffer;
		if (ss.fail())
			return;
		ss >> CustomSize;

		m_vectorSelectionRects[m_SelectionRectIndex].RVA = CustomRVA;
		m_vectorSelectionRects[m_SelectionRectIndex].Size = CustomSize;
		m_vectorSelectionRects[m_SelectionRectIndex].R = Rect(BLOCK_X,
																BLOCK_START_Y + SCALE(CustomRVA, m_HighestRVA, MEMORYLAYOUT_HEIGHT),
																BLOCK_WIDTH,
																SCALE(CustomSize, m_HighestRVA, MEMORYLAYOUT_HEIGHT));
	}

	InvalidateRect(m_hWnd, NULL, TRUE);
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
		{
			// Set RVA, size data and compute rectangle
			TCHAR szBuffer[64];

			Edit_GetText(m_hEditCustomRVA, szBuffer, GetArraySize(szBuffer));
		
			tstringstream ss;
			DWORD CustomRVA = 0, CustomSize = 0;
		
			ss << hex << szBuffer;
			if (ss.fail())
				return;
			ss >> CustomRVA;

			ss.clear();
			Edit_GetText(m_hEditCustomSize, szBuffer, GetArraySize(szBuffer));
			ss << dec << szBuffer;
			if (ss.fail())
				return;
			ss >> CustomSize;

			m_vectorSelectionRects[m_SelectionRectIndex].RVA = CustomRVA;
			m_vectorSelectionRects[m_SelectionRectIndex].Size = CustomSize;
			m_vectorSelectionRects[m_SelectionRectIndex].R = Rect(BLOCK_X,
																	BLOCK_START_Y + SCALE(CustomRVA, m_HighestRVA, MEMORYLAYOUT_HEIGHT),
																	BLOCK_WIDTH,
																	SCALE(CustomSize, m_HighestRVA, MEMORYLAYOUT_HEIGHT));

			InvalidateRect(m_hWnd, NULL, TRUE);
		}
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
		{
			// Set RVA, size data and compute rectangle
			TCHAR szBuffer[64];

			Edit_GetText(m_hEditCustomRVA, szBuffer, GetArraySize(szBuffer));
		
			tstringstream ss;
			DWORD CustomRVA = 0, CustomSize = 0;
		
			ss << hex << szBuffer;
			if (ss.fail())
				return;
			ss >> CustomRVA;

			ss.clear();
			Edit_GetText(m_hEditCustomSize, szBuffer, GetArraySize(szBuffer));
			ss << dec << szBuffer;
			if (ss.fail())
				return;
			ss >> CustomSize;

			m_vectorSelectionRects[m_SelectionRectIndex].RVA = CustomRVA;
			m_vectorSelectionRects[m_SelectionRectIndex].Size = CustomSize;
			m_vectorSelectionRects[m_SelectionRectIndex].R = Rect(BLOCK_X,
																	BLOCK_START_Y + SCALE(CustomRVA, m_HighestRVA, MEMORYLAYOUT_HEIGHT),
																	BLOCK_WIDTH,
																	SCALE(CustomSize, m_HighestRVA, MEMORYLAYOUT_HEIGHT));

			InvalidateRect(m_hWnd, NULL, TRUE);
		}
	}
}

void PropertyPageHandler_Overview::OnDestroy()
{
	SAFE_RELEASE(m_pbitmapMemoryMap);

	GdiplusShutdown(m_gdiplusToken);
}

Gdiplus::Pen& RotatePenColor(Gdiplus::Pen& pen)
{
	static const Color ColorCodes[] = { Color::BlueViolet, Color::DarkSalmon, Color::DarkSeaGreen,
										Color::Coral, Color::DarkCyan, Color::Crimson,
										Color::CadetBlue, Color::Chocolate, Color::DarkRed,
										Color::DarkGray, Color::DarkOliveGreen, Color::DarkOrange,
										Color::DarkSlateBlue, Color::DeepPink, Color::SkyBlue,
										Color::DimGray, Color::DodgerBlue, Color::Firebrick };
	THREAD_ISOLATED_STORAGE static int Count = 0;

	if (Count < GetArraySize(ColorCodes))
	{
		pen.SetColor(SetColorOpacity(ColorCodes[Count].GetValue(), ALPHA_LEVEL));
		Count++;

		return pen;
	}

	Color color;

	pen.GetColor(&color);
	pen.SetColor(Color(color.GetAlpha(), color.GetR() - 20, color.GetG() + 18, color.GetB() - 25));

	return pen;
}