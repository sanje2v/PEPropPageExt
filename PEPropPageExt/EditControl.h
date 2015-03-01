#pragma once
#include <Windows.h>
#include <WindowsX.h>
#include <commctrl.h>
#include <Richedit.h>
#include "CWindow.h"


class EditControl : public CWindow
{
private:
	EditControl(HINSTANCE hInstance, HWND hParent);

public:
	static EditControl *create(HINSTANCE hInstance, HWND hParent);

	void setFont(wstring fontname, LONG fontsize);
	void setTabStops(int arrtabs[], int ctabs);
	void setText(const WCHAR szText[]);
};