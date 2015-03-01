#include "stdafx.h"
#include "AnimationControl.h"


AnimationControl::AnimationControl(HINSTANCE hInstance, HWND hParent)
	: CWindow(hInstance)
{
	m_hWnd = Animate_Create(hParent, 0, ACS_AUTOPLAY | WS_CHILD, m_hInstance);
}

bool AnimationControl::open(HINSTANCE hMod, WORD idData)
{
	// NOTE: We need the following to tell the control that the module is loaded as data file not image
	hMod = HINSTANCE(UINT_PTR(hMod) | UINT_PTR(0x1));
	return (Animate_OpenEx(m_hWnd, hMod, MAKEINTRESOURCE(idData)) == TRUE);
}

AnimationControl *AnimationControl::create(HINSTANCE hInstance, HWND hParent)
{
	std::unique_ptr<AnimationControl> ptr(new AnimationControl(hInstance, hParent));
	if (ptr->getHandle() == NULL)
		return NULL;

	return ptr.release();
}