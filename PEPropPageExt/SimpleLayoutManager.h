// Copyright (c) 2003 Daniel Horn

#ifndef __SIMPLELAYOUTMANAGER_H__
#define __SIMPLELAYOUTMANAGER_H__


// Child window position attributes
enum CWAttribute
{
	CWA_DEFAULT = 0,
	CWA_LEFT = 0x0001,		// => horizontal position does not change.
	CWA_RIGHT = 0x0002,		// => distance from right edge of parent window does not change.
	CWA_LEFTRIGHT = 0x0003,	// => stretches size of child window
	CWA_HCENTER = 0x0004,
	CWA_TOP = 0x0001,		// => vertical position does not change.
	CWA_BOTTOM = 0x0002,
	CWA_TOPBOTTOM = 0x0003,
	CWA_VCENTER = 0x0004
};



class SimpleLayoutManager
{
	friend class ChildWindowConstraint;

private:
	ChildWindowConstraint *listConstraints;
	ChildWindowConstraint *listTail;

	// The instance of a layout manager is associated with a particular window.
	HWND hParent;
	int prevWidth;
	int prevHeight;

	int minWidth;
	int minHeight;

	ChildWindowConstraint* LookupChildConstraint(int id);

public:
	SimpleLayoutManager(HWND hWnd);
	~SimpleLayoutManager(void);

	// Use the default arguments to automatically determine constraints based on initial layout.
	void AddChildConstraint(int id, CWAttribute attrHorz = CWA_DEFAULT, CWAttribute attrVert = CWA_DEFAULT);

	void SetForceInvalidate(int id, bool b);
	
	// Set minimum size of window; uses screen coordinates.
	void SetMinSize(int minWidth, int minHeight);
	// Set minimum size of window to current size.
	void SetMinSize(void);

	void SetHorzSizeWeight(int id, int wtSize, int idRelative = -1);
	void SetVertSizeWeight(int id, int wtSize, int idRelative = -1);
	
	void SetRelativeHorzPos(int id, int idRelativeHorz, bool bAlign = false);
	void SetRelativeVertPos(int id, int idRelative, bool bAlign = false);

	void DoLayout(WPARAM wSizeType, LPARAM lParam);
	void DoLayout();
	BOOL DoSizing(RECT* pRect);

	void RemoveChildConstraint(int id);
};


#endif	// __SIMPLELAYOUTMANAGER_H__
