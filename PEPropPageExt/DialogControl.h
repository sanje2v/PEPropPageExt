#pragma once
#include <cassert>
#include <Windows.h>
#include <WindowsX.h>
#include "unique_handle.h"
#include "CWindow.h"

#define CHILDPREVIEWDIALOG_CLASSNAME			L"CHILDPREVIEWDIALOG"


class DialogControl : public CWindow
{
private:
	static HINSTANCE s_hInstance;

	LPCDLGTEMPLATE m_pTemplate;

	DialogControl(HINSTANCE hInstance, LPCDLGTEMPLATE pTemplate, DWORD sizeTemplate);

public:
	static void registerParentWindowClass(HINSTANCE hInstance = NULL);
	static void unregisterParentWindowClass();

	static DialogControl *create(HINSTANCE hInstance, LPCDLGTEMPLATE pTemplate, DWORD sizeTemplate);
	static LRESULT CALLBACK controlWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};