#include "stdafx.h"
#include "DialogControl.h"


HINSTANCE DialogControl::s_hInstance = NULL;

DialogControl::DialogControl(HINSTANCE hInstance, LPCDLGTEMPLATE pTemplate, DWORD sizeTemplate)
	: CWindow(hInstance)
{
	if (sizeTemplate < sizeof(DLGTEMPLATE))
		return;

	m_pTemplate = pTemplate;

	SetLastError(ERROR_SUCCESS); // We need to see the last error for the following function so reset it
	m_hWnd = CreateDialogIndirect(m_hInstance,
								  m_pTemplate,
								  NULL,
								  NULL);
	if (m_hWnd == NULL && GetLastError() == ERROR_TLW_WITH_WSCHILD)
	{
		// Window creation failed because it has 'WS_CHILD' style. That's alright, we can make a preview
		//  parent window for this child window.
		unique_handle<HWND, decltype(funcDestroyWindow)> hParent(CreateWindow(CHILDPREVIEWDIALOG_CLASSNAME,
			L"PEPropPageExt - Child dialog preview",
			WS_DLGFRAME,
			0,
			0,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			NULL,
			NULL,
			hInstance,
			NULL), funcDestroyWindow);
		if (!hParent.get())
			return;

		m_hWnd = CreateDialogIndirect(m_hInstance,
									  m_pTemplate,
									  hParent.get(),
									  NULL);
		if (m_hWnd)
		{
			// Resize to make all of the child window contents visible
			SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER); // Move child dialog to top left of client area

			// Ask child dialog about it's dimension
			RECT rectChild = { 0 };
			GetWindowRect(m_hWnd, &rectChild);
			
			// Readjust child window rectangle to include window border
			AdjustWindowRectEx(&rectChild, GetWindowStyle(m_hWnd), FALSE, DWORD(GetWindowLongPtr(m_hWnd, GWL_EXSTYLE)));
			SetWindowPos(hParent.get(), NULL, 0, 0, (rectChild.right - rectChild.left), (rectChild.bottom - rectChild.top), SWP_NOMOVE | SWP_NOZORDER);
			ShowWindow(m_hWnd, SW_SHOW);

			m_hWnd = hParent.release_ownership();  // All child window functions are now handled by parent
		}
	}
	else if (m_hWnd != NULL)  // If dialog was created without parent, bring it to top
		BringWindowToTop(m_hWnd);
}

void DialogControl::registerParentWindowClass(HINSTANCE hInstance)
{
	s_hInstance = (hInstance == NULL ? GetModuleHandle(NULL) : hInstance);

	// Register a new window, if not already
	WNDCLASS WndClass;

	if (GetClassInfo(hInstance, CHILDPREVIEWDIALOG_CLASSNAME, &WndClass) == FALSE)
	{
		// Class hasn't been registered, so do so
		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof(wc);
		wc.lpszClassName = CHILDPREVIEWDIALOG_CLASSNAME;
		wc.hInstance = hInstance;
		wc.lpfnWndProc = WNDPROC(controlWndProc);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon = NULL;
		wc.lpszMenuName = NULL;
		wc.hbrBackground = HBRUSH(COLOR_WINDOW + 1);
		wc.style = 0;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hIconSm = NULL;

		ATOM atomClass = RegisterClassEx(&wc);
		assert(atomClass != NULL);
	}
}

void DialogControl::unregisterParentWindowClass()
{
	WNDCLASS WndClass;

	if (GetClassInfo(s_hInstance, CHILDPREVIEWDIALOG_CLASSNAME, &WndClass) == TRUE)
	{
		BOOL classNotInUse = UnregisterClass(CHILDPREVIEWDIALOG_CLASSNAME, s_hInstance);

		assert(classNotInUse != 0);	// WARNING: Failed assertion here means there is still a window open which uses this window class
	}
}

DialogControl *DialogControl::create(HINSTANCE hInstance, LPCDLGTEMPLATE pTemplate, DWORD sizeTemplate)
{
	unique_ptr<DialogControl> ptr(new DialogControl(hInstance, pTemplate, sizeTemplate));
	if (ptr->m_hWnd == NULL)
		return NULL;

	return ptr.release();
}

LRESULT DialogControl::controlWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}