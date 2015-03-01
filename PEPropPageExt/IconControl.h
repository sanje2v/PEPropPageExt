#pragma once

#include <Windows.h>
#include <WindowsX.h>
#include <commctrl.h>
#include "CWindow.h"

class IconControl : public CWindow
{
private:
	IconControl(HINSTANCE hInstance, HWND hParent);

public:
	static IconControl *create(HINSTANCE hInstance, HWND hParent);

	void setIconHandle(HICON hIcon);
};