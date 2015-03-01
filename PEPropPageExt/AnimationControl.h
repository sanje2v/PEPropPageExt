#pragma once

#include <Windows.h>
#include <WindowsX.h>
#include <commctrl.h>
#include "CWindow.h"

class AnimationControl : public CWindow
{
private:
	AnimationControl(HINSTANCE hInstance, HWND hParent);

public:
	static AnimationControl *create(HINSTANCE hInstance, HWND hParent);

	bool open(HINSTANCE hMod, WORD idData);
};