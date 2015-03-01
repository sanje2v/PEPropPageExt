#include "stdafx.h"
#include "EditControl.h"


EditControl::EditControl(HINSTANCE hInstance, HWND hParent)
	: CWindow(hInstance)
{
	// NOTE: We have already load library for 'RichEdit' control 2.0, so need to worry about that here
	m_hWnd = CreateWindowEx(WS_EX_STATICEDGE,
							RICHEDIT_CLASSW,
							L"",
							WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL | ES_READONLY,
							0,
							0,
							10,
							10,
							hParent,
							NULL,
							hInstance,
							NULL);
}

void EditControl::setFont(wstring fontname, LONG fontsize)
{
	// Set font for rich edit
	CHARFORMAT CharFormat;
	ZeroMemory(&CharFormat, sizeof(CharFormat));

	CharFormat.cbSize = sizeof(CharFormat);
	CharFormat.dwMask = CFM_FACE | CFM_SIZE;
	wcscpy_s(CharFormat.szFaceName, fontname.c_str());
	CharFormat.yHeight = fontsize;
	SendMessage(m_hWnd, EM_SETCHARFORMAT, SCF_DEFAULT, LPARAM(&CharFormat));
}

void EditControl::setTabStops(int arrtabs[], int ctabs)
{
	// Set tab stop  for rich edit
	Edit_SetTabStops(m_hWnd, ctabs, arrtabs);
}

void EditControl::setText(const WCHAR szText[])
{
	if (m_hWnd)
		SetWindowText(m_hWnd, szText);
}

EditControl *EditControl::create(HINSTANCE hInstance, HWND hParent)
{
	std::unique_ptr<EditControl> ptr(new EditControl(hInstance, hParent));
	if (ptr->m_hWnd == NULL)
		return NULL;

	return ptr.release();
}