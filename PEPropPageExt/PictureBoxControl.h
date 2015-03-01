#pragma once
/////////////////////////////////////////////////////////////////////
// Picture Box Control
// Written by: Sanjeev Sharma
// Information at: http://sanje2v.wordpress.com/
// License: Public Domain, any liability is disclaimed
////////////////////////////////////////////////////////////////////

#include <cassert>
#include <Windows.h>
#include "CWindow.h"

#define PICTUREBOXCONTROL_CLASSNAME			L"PICTUREBOXCONTROL"
#define PBC_GETBITMAP						WM_USER
#define PBC_SETBITMAP						WM_USER + 1


typedef class PictureBoxControl : public CWindow
{
private:
	static const int SMALL_SCROLL = 10;
	static const int BIG_SCROLL = 20;

	static HINSTANCE s_hInstance;

	HBITMAP m_hBitmap;

	PictureBoxControl(HINSTANCE hInstance, HWND hParent);

	void showScrollBarsIfNeeded();

public:
	static void registerControlWindowClass(HINSTANCE hInstance = NULL);
	static void unregisterControlWindowClass();

	static PictureBoxControl *create(HINSTANCE hInstance, HWND hParent);
	static LRESULT CALLBACK controlWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	~PictureBoxControl();

	HBITMAP getBitmapHandle();
	void setBitmapHandle(HBITMAP hBitmap);	
} *pPictureBoxControl;