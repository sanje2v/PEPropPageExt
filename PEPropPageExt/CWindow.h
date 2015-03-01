#pragma once

#include <Windows.h>


class CWindow abstract
{
protected:
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	CWindow(HINSTANCE hInstance)
		: m_hWnd(NULL),
		m_hInstance(hInstance == NULL ? GetModuleHandle(NULL) : hInstance)
	{}

public:
	virtual ~CWindow()
	{
		if (m_hWnd != NULL)
			DestroyWindow(m_hWnd);
	}

	HWND getHandle()
	{
		return m_hWnd;
	}

	void setPosition(int x, int y)
	{
		if (m_hWnd)
			SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	void setSize(int width, int height)
	{
		if (m_hWnd)
			SetWindowPos(m_hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	void setVisible(bool show)
	{
		if (m_hWnd)
			ShowWindow(m_hWnd, (show ? SW_SHOW : SW_HIDE));
	}

	void repaint()
	{
		if (m_hWnd)
			RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
	}
};