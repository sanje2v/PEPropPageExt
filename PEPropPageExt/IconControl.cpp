#include "stdafx.h"
#include "IconControl.h"


IconControl::IconControl(HINSTANCE hInstance, HWND hParent)
	: CWindow(hInstance)
{
	m_hWnd = CreateWindow(L"STATIC",
						  L"",
						  WS_CHILD | SS_ICON | SS_REALSIZEIMAGE,
						  0,
						  0,
						  10,
						  10,
						  hParent,
						  NULL,
						  m_hInstance,
						  NULL);
}

IconControl *IconControl::create(HINSTANCE hInstance, HWND hParent)
{
	std::unique_ptr<IconControl> ptr(new IconControl(hInstance, hParent));
	if (ptr->m_hWnd == NULL)
		return NULL;

	return ptr.release();
}

void IconControl::setIconHandle(HICON hIcon)
{
	Static_SetIcon(m_hWnd, hIcon);
}