#include "stdafx.h"
#include "PictureBoxControl.h"


HINSTANCE PictureBoxControl::s_hInstance = NULL;

PictureBoxControl::PictureBoxControl(HINSTANCE hInstance, HWND hParent)
	: CWindow(hInstance),
	m_hBitmap(NULL)
{
	// Create a new window
	m_hWnd = CreateWindowEx(WS_EX_STATICEDGE,
							PICTUREBOXCONTROL_CLASSNAME,
							L"",
							WS_CHILD,
							0,
							0,
							16,
							16,
							hParent,
							NULL,
							m_hInstance,
							LPVOID(this));	// NOTE: The last parameter value is used to pass 'this' pointer
}
PictureBoxControl::~PictureBoxControl()
{
	if (m_hBitmap)
		DeleteObject(m_hBitmap);
}

void PictureBoxControl::registerControlWindowClass(HINSTANCE hInstance)
{
	s_hInstance = (hInstance == NULL ? GetModuleHandle(NULL) : hInstance);

	// Register a new window, if not already
	WNDCLASS WndClass;

	if (GetClassInfo(hInstance, PICTUREBOXCONTROL_CLASSNAME, &WndClass) == FALSE)
	{
		// Class hasn't been registered, so do so
		WNDCLASSEX wc = { 0 };
		wc.cbSize         = sizeof(wc);
		wc.lpszClassName  = PICTUREBOXCONTROL_CLASSNAME;
		wc.hInstance      = hInstance;
		wc.lpfnWndProc    = WNDPROC(controlWndProc);
		wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon          = NULL;
		wc.lpszMenuName   = NULL;
		wc.hbrBackground  = HBRUSH(COLOR_WINDOW + 1);
		wc.style          = 0;
		wc.cbClsExtra     = 0;
		wc.cbWndExtra     = 0;
		wc.hIconSm        = NULL;
		
		ATOM atomClass = RegisterClassEx(&wc);
		assert(atomClass != NULL);
	}
}

void PictureBoxControl::unregisterControlWindowClass()
{
	WNDCLASS WndClass;

	if (GetClassInfo(s_hInstance, PICTUREBOXCONTROL_CLASSNAME, &WndClass) == TRUE)
	{
		BOOL classNotInUse = UnregisterClass(PICTUREBOXCONTROL_CLASSNAME, s_hInstance);

		assert(classNotInUse != 0);	// WARNING: Failed assertion here means there is still a window open which uses this window class
	}
}

PictureBoxControl *PictureBoxControl::create(HINSTANCE hInstance, HWND hParent)
{
	std::unique_ptr<PictureBoxControl> ptr(new PictureBoxControl(hInstance, hParent));
	if (ptr->getHandle() == NULL)
		return NULL;

	return ptr.release();
}

LRESULT PictureBoxControl::controlWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PictureBoxControl *_this = pPictureBoxControl(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (uMsg)
	{
	case WM_CREATE:
		{
			// Retrieve 'this' pointer and save it as window instance's user data
			LPCREATESTRUCT pCreate = LPCREATESTRUCT(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, LONG_PTR(pCreate->lpCreateParams));
		}
		break;

	case WM_SIZE:
		{
			assert(_this);

			_this->showScrollBarsIfNeeded();
		}
		break;

	case WM_PAINT:
		{
			assert(_this);

			// Drawing bitmap based on the scroll bar position is done here
			PAINTSTRUCT Paint;
			HDC hDC = BeginPaint(hWnd, &Paint);
			if (hDC)
			{
				HDC hSrcDC = CreateCompatibleDC(hDC);
				SelectObject(hSrcDC, _this->getBitmapHandle());

				// Draw from 'hSrcDC' to 'hDC'
				RECT rectClient = { 0 };
				GetClientRect(hWnd, &rectClient);	// NOTE: Client rectangle subtract area occupied by scroll bars

				BITMAP bitmap = { 0 };
				GetObject(_this->getBitmapHandle(), sizeof(bitmap), &bitmap);

				const int posVScroll = GetScrollPos(hWnd, SB_VERT);
				const int posHScroll = GetScrollPos(hWnd, SB_HORZ);
				BitBlt(hDC,
					   -posHScroll,
					   -posVScroll,
					   (rectClient.right - rectClient.left) + posHScroll,
					   (rectClient.bottom - rectClient.top) + posVScroll,
					   hSrcDC,
					   0,
					   0,
					   SRCCOPY);

				DeleteDC(hSrcDC);

				EndPaint(hWnd, &Paint);
			}

			return TRUE;
		}
		break;

	case WM_VSCROLL:
		{
			assert(_this);

			// NOTE: The fourth parameter of the function 'SetScrollInfo()' is whether
			//	the scroll bar should redraw itself and NOT that the control redraw itself.
			//	To redraw the control we call 'RedrawWindow()' at the end of 'switch' statement.
			SCROLLINFO infoScroll = { 0 };
			infoScroll.cbSize = sizeof(infoScroll);

			switch (LOWORD(wParam))
			{
			case SB_TOP:
				infoScroll.fMask = SIF_RANGE;
				GetScrollInfo(hWnd, SB_VERT, &infoScroll);
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = infoScroll.nMin;

				break;

			case SB_BOTTOM:
				infoScroll.fMask = SIF_RANGE;
				GetScrollInfo(hWnd, SB_VERT, &infoScroll);
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = infoScroll.nMax;

				break;

			case SB_PAGEUP:
				infoScroll.fMask = SIF_RANGE | SIF_POS;
				GetScrollInfo(hWnd, SB_VERT, &infoScroll);
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = max(infoScroll.nMin, infoScroll.nPos - BIG_SCROLL);

				break;

			case SB_PAGEDOWN:
				infoScroll.fMask = SIF_RANGE | SIF_POS;
				GetScrollInfo(hWnd, SB_VERT, &infoScroll);
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = min(infoScroll.nMax, infoScroll.nPos + BIG_SCROLL);

				break;

			case SB_LINEUP:
				infoScroll.fMask = SIF_RANGE | SIF_POS;
				GetScrollInfo(hWnd, SB_VERT, &infoScroll);
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = max(infoScroll.nMin, infoScroll.nPos - SMALL_SCROLL);

				break;

			case SB_LINEDOWN:
				infoScroll.fMask = SIF_RANGE | SIF_POS;
				GetScrollInfo(hWnd, SB_VERT, &infoScroll);
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = min(infoScroll.nMax, infoScroll.nPos + SMALL_SCROLL);

				break;

			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = HIWORD(wParam);

				break;

			default:
				return 0;
			}

			SetScrollInfo(hWnd, SB_VERT, &infoScroll, TRUE);
			_this->repaint();

			return 0;
		}
		break;

		case WM_HSCROLL:
		{
			assert(_this);

			SCROLLINFO infoScroll = { 0 };
			infoScroll.cbSize = sizeof(infoScroll);

			switch (LOWORD(wParam))
			{
			case SB_LEFT:
				infoScroll.fMask = SIF_RANGE;
				GetScrollInfo(hWnd, SB_HORZ, &infoScroll);
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = infoScroll.nMin;

				break;

			case SB_RIGHT:
				infoScroll.fMask = SIF_RANGE;
				GetScrollInfo(hWnd, SB_HORZ, &infoScroll);
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = infoScroll.nMax;

				break;

			case SB_PAGEUP:
				infoScroll.fMask = SIF_RANGE | SIF_POS;
				GetScrollInfo(hWnd, SB_HORZ, &infoScroll);
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = max(infoScroll.nMin, infoScroll.nPos - BIG_SCROLL);

				break;

			case SB_PAGEDOWN:
				infoScroll.fMask = SIF_RANGE | SIF_POS;
				GetScrollInfo(hWnd, SB_HORZ, &infoScroll);
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = min(infoScroll.nMax, infoScroll.nPos + BIG_SCROLL);

				break;

			case SB_LINEUP:
				infoScroll.fMask = SIF_RANGE | SIF_POS;
				GetScrollInfo(hWnd, SB_HORZ, &infoScroll);
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = max(infoScroll.nMin, infoScroll.nPos - SMALL_SCROLL);

				break;

			case SB_LINEDOWN:
				infoScroll.fMask = SIF_RANGE | SIF_POS;
				GetScrollInfo(hWnd, SB_HORZ, &infoScroll);
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = min(infoScroll.nMax, infoScroll.nPos + SMALL_SCROLL);

				break;

			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				infoScroll.fMask = SIF_POS;
				infoScroll.nPos = HIWORD(wParam);

				break;

			default:
				return 0;
			}

			SetScrollInfo(hWnd, SB_HORZ, &infoScroll, TRUE);
			_this->repaint();

			return 0;
		}
		break;

	case PBC_GETBITMAP:
		assert(_this);
		return LRESULT(_this->getBitmapHandle());

	case PBC_SETBITMAP:
		{
			assert(_this);
			_this->setBitmapHandle(HBITMAP(wParam));

			return TRUE;
		}
		break;

	default: break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void PictureBoxControl::showScrollBarsIfNeeded()
{
	if (m_hWnd && m_hBitmap)
	{
		RECT rectClient = { 0 };
		GetClientRect(m_hWnd, &rectClient);

		BITMAP bitmap = { 0 };
		GetObject(m_hBitmap, sizeof(bitmap), &bitmap);

		// Show/Hide scroll bars
		BOOL bShowVScroll = ((bitmap.bmHeight > (rectClient.bottom - rectClient.top)) ? TRUE : FALSE);
		BOOL bShowHScroll = ((bitmap.bmWidth > (rectClient.right - rectClient.left)) ? TRUE : FALSE);

		ShowScrollBar(m_hWnd, SB_VERT, bShowVScroll);
		ShowScrollBar(m_hWnd, SB_HORZ, bShowHScroll);

		if (bShowVScroll)
		{
			// Set minimum-maximum value range for vertical scroll bar
			SCROLLINFO infoScroll = { 0 };
			infoScroll.cbSize = sizeof(infoScroll);
			infoScroll.fMask = SIF_POS | SIF_RANGE;
			infoScroll.nMin = 0;
			infoScroll.nMax = max(0, (bitmap.bmHeight - (rectClient.bottom - rectClient.top)));
			infoScroll.nPos = 0;

			SetScrollInfo(m_hWnd, SB_VERT, &infoScroll, TRUE);
		}

		if (bShowHScroll)
		{
			// Set minimum-maximum value range for horizontal scroll bar
			SCROLLINFO infoScroll = { 0 };
			infoScroll.cbSize = sizeof(infoScroll);
			infoScroll.fMask = SIF_POS | SIF_RANGE;
			infoScroll.nMin = 0;
			infoScroll.nMax = max(0, (bitmap.bmWidth - (rectClient.right - rectClient.left)));
			infoScroll.nPos = 0;

			SetScrollInfo(m_hWnd, SB_HORZ, &infoScroll, TRUE);
		}
	}
}

HBITMAP PictureBoxControl::getBitmapHandle()
{
	return m_hBitmap;
}

void PictureBoxControl::setBitmapHandle(HBITMAP hBitmap)
{
	if (m_hBitmap)
		DeleteObject(m_hBitmap);  // First, destroy the bitmap handle currently owned, if any

	m_hBitmap = hBitmap;

	showScrollBarsIfNeeded();
	repaint();
}