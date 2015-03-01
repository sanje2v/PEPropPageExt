// Copyright (c) 2003 Daniel Horn

#include "stdafx.h"
#include "SimpleLayoutManager.h"

#if !defined(max)
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif

struct OrientationConstraint
{
	CWAttribute attrib;
	// ID of another child control, relative to which the child's position is determined;
	// by default, not used unless set != -1; 
	// a value of -1 means settings are relative to the parent window.
	int idRelative;
	int prevDist;
	int weightSize;
	int idRelativeSize;
	bool bAlign;

	OrientationConstraint(void)
		: attrib(CWA_DEFAULT), 
		idRelative(-1), 
		weightSize(100), 
		prevDist(0), 
		idRelativeSize(-1), 
		bAlign(false)
	{
	}
};

class ChildWindowConstraint
{
	friend class SimpleLayoutManager;
	friend void SetSizeAndPos(int& xWidth, int& xPos, 
						  ChildWindowConstraint *constraint,
						  bool bHorizontal,
						  WORD nWidth,
						  int left, int right,
						  int prevWidth,
						  HWND hParent);

private:
	// Control ID of child window
	int id;			

	OrientationConstraint horizontal;
	OrientationConstraint vertical;

	// Use to force InvalidateRect call to fix painting blemish.
	bool bForceInvalidate;

	// For maintaining linked list of constraints.
	ChildWindowConstraint *next;

	// Use the default arguments to automatically determine constraints based on initial layout.
	ChildWindowConstraint(int idChild, SimpleLayoutManager *layout, CWAttribute attrHorz = CWA_DEFAULT, CWAttribute attrVert = CWA_DEFAULT);

	void SetForceInvalidate(bool b)
	{
		bForceInvalidate = b;
	}

	void SetHorzSizeWeight(int wtSize, int idRelative)
	{
		// We allow only upto 100 % if size is determined relative to the parent window,
		// but any non-negative amount for a size relative to another child window.
		if ((0 <= wtSize) && ((idRelative != -1) || (wtSize <= 100)))
		{
			horizontal.weightSize = wtSize;
			horizontal.idRelativeSize = idRelative;
		}
	}

	void SetVertSizeWeight(int wtSize, int idRelative)
	{
		if ((0 <= wtSize) && ((idRelative != -1) || (wtSize <= 100)))
		{
			vertical.weightSize = wtSize;
			vertical.idRelativeSize = idRelative;
		}
	}

	void SetRelativePos(int idRelative, bool bHorizontal, SimpleLayoutManager *layout, bool bAlign);

public:
};



ChildWindowConstraint::ChildWindowConstraint(int idChild, 
	SimpleLayoutManager *layout,
	CWAttribute attrHorz, CWAttribute attrVert)
	: id(idChild), 
	bForceInvalidate(false),
	next(NULL)
{
	RECT rect;
	GetWindowRect(GetDlgItem(layout->hParent, id), &rect);
	MapWindowPoints(NULL, layout->hParent, (LPPOINT)&rect, 2);

	horizontal.prevDist = (layout->prevWidth - rect.right) + rect.left;
	vertical.prevDist = (layout->prevHeight - rect.bottom) + rect.top;

	RECT rectParent;
	GetClientRect(layout->hParent, &rectParent);

	int xCenter = (rectParent.right - rectParent.left)/2;
	int yCenter = (rectParent.bottom - rectParent.top)/2;

	if (CWA_DEFAULT == attrHorz)
	{
		if (rect.right < xCenter)
		{
			attrHorz = CWA_LEFT;
		}
		else if (rect.left > xCenter)
		{
			attrHorz = CWA_RIGHT;
		}
		else
		{
			attrHorz = CWA_LEFTRIGHT;
		}
	}
	horizontal.attrib = attrHorz;

	if (CWA_DEFAULT == attrVert)
	{
		if (rect.bottom < yCenter)
		{
			attrVert = CWA_TOP;
		}
		else if (rect.top > yCenter)
		{
			attrVert = CWA_BOTTOM;
		}
		else
		{
			attrVert = CWA_TOPBOTTOM;
		}
	}
	vertical.attrib = attrVert;

}


SimpleLayoutManager::SimpleLayoutManager(HWND hWnd)
	: listConstraints(NULL), listTail(NULL), hParent(hWnd), minHeight(0), minWidth(0)
{
	RECT rect;
	GetClientRect(hWnd, &rect);

	prevWidth = rect.right - rect.left;
	prevHeight = rect.bottom - rect.top;
}

SimpleLayoutManager::~SimpleLayoutManager(void)
{
	ChildWindowConstraint *constraint = listConstraints;

	while(NULL != constraint)
	{
		ChildWindowConstraint *next = constraint->next;
		delete constraint;

		constraint = next;
	}
}

void SimpleLayoutManager::AddChildConstraint(int id, CWAttribute attrHorz, CWAttribute attrVert)
{
	ChildWindowConstraint *constraint = new ChildWindowConstraint(id, this, attrHorz, attrVert);

	if (NULL == listConstraints)
	{
		listConstraints = constraint;
	}
	if (NULL != listTail)
	{
		listTail->next = constraint;
	}
	listTail = constraint;
}

ChildWindowConstraint* SimpleLayoutManager::LookupChildConstraint(int id)
{
	// As look as all operations for a given child id are done immediately after the call to
	// AddChildConstraint, then use listTail, which maintains a point to the last item in the list.
	if ((NULL != listTail) && (id == listTail->id))
	{
		return listTail;
	}

	ChildWindowConstraint *constraint = listConstraints;

	while(NULL != constraint)
	{
		if (id == constraint->id)
		{
			return constraint;
		}

		constraint = constraint->next;
	}

	return NULL;
}

void SimpleLayoutManager::SetForceInvalidate(int id, bool b)
{
	ChildWindowConstraint *constraint = LookupChildConstraint(id);
	if (NULL != constraint)
	{
		constraint->SetForceInvalidate(b);
	}
}

void SimpleLayoutManager::SetRelativeHorzPos(int id, int idRelativeHorz, bool bAlign /* = false */)
{
	ChildWindowConstraint *constraint = LookupChildConstraint(id);
	if (NULL != constraint)
	{
		constraint->SetRelativePos(idRelativeHorz, true, this, bAlign);
	}
}

void SimpleLayoutManager::SetRelativeVertPos(int id, int idRelative, bool bAlign /* = false */)
{
	ChildWindowConstraint *constraint = LookupChildConstraint(id);
	if (NULL != constraint)
	{
		constraint->SetRelativePos(idRelative, false, this, bAlign);
	}
}

void ChildWindowConstraint::SetRelativePos(int idRelative, bool bHorizontal, SimpleLayoutManager *layout, bool bAlign)
{
	// We probably should check to see if the attribute for this oriental is CWA_LEFTRIGHT or CWA_TOPBOTTOM.
	// If so, we should skip the rest of this function.
	// In particular, in this case, we don't want to change the value of prevDist.
	RECT rect;
	GetWindowRect(GetDlgItem(layout->hParent, id), &rect);
	MapWindowPoints(NULL, layout->hParent, (LPPOINT)&rect, 2);

	RECT rectRelative;
	GetWindowRect(GetDlgItem(layout->hParent, idRelative), &rectRelative);
	MapWindowPoints(NULL, layout->hParent, (LPPOINT)&rectRelative, 2);

	if (bHorizontal)
	{
		horizontal.idRelative = idRelative;
		horizontal.bAlign = bAlign;

		switch(horizontal.attrib)
		{
			case CWA_LEFT:
				horizontal.prevDist = rect.left - rectRelative.right;
				break;

			case CWA_RIGHT:
				horizontal.prevDist = rectRelative.left - rect.right;
				break;

			case CWA_LEFTRIGHT:
				break;

			default:
				horizontal.prevDist = 0;
				break;
		}

	}
	else
	{
		vertical.idRelative = idRelative;
		vertical.bAlign = bAlign;

		switch(vertical.attrib)
		{
			case CWA_TOP:
				vertical.prevDist = rect.top - rectRelative.bottom;
				break;

			case CWA_BOTTOM:
				vertical.prevDist = rectRelative.top - rect.bottom;
				break;

			case CWA_TOPBOTTOM:
				break;

			default:
				vertical.prevDist = 0;
				break;
		}

	}
}


// See ChildWindowConstraint::SetHorzSizeWeight for details.
void SimpleLayoutManager::SetHorzSizeWeight(int id, int wtSize, int idRelative /* = -1 */)
{
	ChildWindowConstraint *constraint = LookupChildConstraint(id);
	if (NULL != constraint)
	{
		constraint->SetHorzSizeWeight(wtSize, idRelative);
	}
}

void SimpleLayoutManager::SetVertSizeWeight(int id, int wtSize, int idRelative /* = -1 */)
{
	ChildWindowConstraint *constraint = LookupChildConstraint(id);
	if (NULL != constraint)
	{
		constraint->SetVertSizeWeight(wtSize, idRelative);
	}
}

void SimpleLayoutManager::SetMinSize(int minWidth, int minHeight)
{
	this->minWidth = minWidth;
	this->minHeight = minHeight;
}

// Set minimum size of window to current size.
void SimpleLayoutManager::SetMinSize(void)
{
	RECT rect;
	GetWindowRect(hParent, &rect);

	SetMinSize(rect.right - rect.left, rect.bottom - rect.top);
}

// Enforce minimum size of form.
BOOL SimpleLayoutManager::DoSizing(RECT* pRect)
{
	LONG nWidth = pRect->right - pRect->left;
	LONG nHeight = pRect->bottom - pRect->top;

	if ((nWidth < minWidth) || (nHeight < minHeight))
	{
		pRect->right = max(pRect->right, pRect->left + minWidth);
		pRect->bottom = max(pRect->bottom, pRect->top + minHeight);
	}

	return TRUE;
}

void SimpleLayoutManager::RemoveChildConstraint(int id)
{
	ChildWindowConstraint *prevconstraint = listConstraints;
	ChildWindowConstraint *constraint = (prevconstraint != NULL ? listConstraints->next : NULL);
	
	if (prevconstraint == NULL)
		return;

	if (constraint == NULL)
	{
		if (prevconstraint->id == id)
			delete prevconstraint;

		return;
	}

	if (prevconstraint->id == id)
	{
		listConstraints = prevconstraint->next;
		delete prevconstraint;

		return;
	}

	while (NULL != constraint)
	{
		if (constraint->id == id)
		{
			prevconstraint->next = constraint->next;

			delete constraint;

			break;
		}

		prevconstraint = constraint;
		constraint = constraint->next;
	};
}


// Note that, despite the names of the parameters and local variables, this routine works for both
// horizontal AND vertical settings, because:
//		the logic for vertical vs. horizontal is exactly symmetric when you substitute top for left and bottom for right, etc.;
//		the definitions of the CWAttribute values correspond (eg, the integer value of CWA_LEFT is the same as CWA_TOP, CWA_LEFTRIGHT is
//		the same as CWA_TOPBOTTOM).
// The routine refers to horizontal values only because it is easier to think and write this thinking of one of the concrete cases, instead
// of both at once.
static void SetSizeAndPos(int& xWidth, int& xPos, 
						  ChildWindowConstraint *constraint,
						  bool bHorizontal,
						  WORD nWidth,
						  int left, int right,
						  int prevWidth,
						  HWND hParent)
{
	OrientationConstraint *orientation = &(bHorizontal ? constraint->horizontal: constraint->vertical);

	// Note: If the attributes = CWA_LEFT && CWA_TOP, then an optimization in the 
	// original version would
	// be to not add the child window to the layout manager's list at all.
	// (ie, we change neither the child's position nor its size.)
	// However, in this second version, child attributes may be changed after
	// adding them, so this optimization would probably no longer make sense 
	// (unless changing an attribute would add the item to the list if it wasn't present).

	// if "weights" applied:

	if (100 != orientation->weightSize)
	{
		if (-1 == orientation->idRelativeSize)
		{
			xWidth = (nWidth * orientation->weightSize)/100;
		}
		else
		{
			RECT rectRelative;
			HWND hRelative = GetDlgItem(hParent, orientation->idRelativeSize);
			GetWindowRect(hRelative, &rectRelative);
			MapWindowPoints(NULL, hParent, (LPPOINT)&rectRelative, 2);

			xWidth = ((bHorizontal ? (rectRelative.right - rectRelative.left) : (rectRelative.bottom - rectRelative.top)) 
				* orientation->weightSize)/100;
		}
	}
	
	// width of control does not change except in case of CWA_LEFTRIGHT
	//
	// Note that the CWA_LEFTRIGHT attribute overrides the size weight (if any).
	// This is done solely for simplicity's sake; 
	// I can think of a couple of meanings for this, but neither is intuitive,
	// nor gives functionality that is needed at this time, 
	if ((orientation->attrib & CWA_LEFTRIGHT) == CWA_LEFTRIGHT)
	{
		// xPos doesn't change...
		if (right > left)
		{
			xWidth =  nWidth - (prevWidth - right) - left;
		}
		else
		{
			// This is the bug fix to the problem with version 1.
			xWidth =  nWidth - orientation->prevDist;
		}
	}
	else if (orientation->attrib & CWA_LEFT)
	{
		// xPos and xWidth don't change.
	}
	else if (orientation->attrib & CWA_HCENTER)
	{
		// xWidth doesn't change.
		xPos = (nWidth - xWidth)/2;
	}
	else	// CWA_RIGHT
	{
		// xWidth doesn't change
		xPos = nWidth - (prevWidth - right + xWidth);
	}

	// Above settings changed if constraint should be relative to another child window.
	if (-1 != orientation->idRelative)
	{
		RECT rectRelative;
		HWND hRelative = GetDlgItem(hParent, orientation->idRelative);
		GetWindowRect(hRelative, &rectRelative);
		MapWindowPoints(NULL, hParent, (LPPOINT)&rectRelative, 2);

		if (bHorizontal)
		{
			switch(orientation->attrib)
			{
				case CWA_HCENTER:
					xPos = (rectRelative.right + rectRelative.left)/2 - xWidth/2;
					break;
				
				// In this case, we also make the current child window the same size as the relative child.
				case CWA_LEFTRIGHT:
					xWidth = rectRelative.right - rectRelative.left;
					xPos = rectRelative.left;
					break;
				
				case CWA_RIGHT:
					if (orientation->bAlign)
					{
						xPos = rectRelative.right - xWidth;
					}
					else
					{
						xPos = rectRelative.left - xWidth - orientation->prevDist;
					}
					break;

				case CWA_LEFT:
				default:
					if (orientation->bAlign)
					{
						xPos = rectRelative.left;
					}
					else
					{
						xPos = rectRelative.right + orientation->prevDist;
					}
					break;
			}
		}
		else
		{
			switch(orientation->attrib)
			{
				case CWA_VCENTER:
					xPos = (rectRelative.bottom + rectRelative.top)/2 - xWidth/2;
					break;
				
				// In this case, we also make the current child window the same size as the relative child.
				case CWA_TOPBOTTOM:
					xWidth = rectRelative.bottom - rectRelative.top;
					xPos = rectRelative.top;
					break;
				
				case CWA_BOTTOM:
					if (orientation->bAlign)
					{
						xPos = rectRelative.bottom - xWidth;
					}
					else
					{
						xPos = rectRelative.top - xWidth - orientation->prevDist;
					}
					break;

				case CWA_TOP:
				default:
					if (orientation->bAlign)
					{
						xPos = rectRelative.top;
					}
					else
					{
						xPos = rectRelative.bottom + orientation->prevDist;
					}
					break;
			}
		}
	}
}

void SimpleLayoutManager::DoLayout(WPARAM wSizeType, LPARAM lParam)
{
	if (!( (SIZE_RESTORED == wSizeType)
		|| (SIZE_MAXIMIZED == wSizeType)
		) )
	{
		return;
	}

	ChildWindowConstraint *constraint = listConstraints;

	WORD nWidth = LOWORD(lParam);  // width of client area 
	WORD nHeight = HIWORD(lParam); // height of client area

	while(NULL != constraint)
	{
		HWND hControl = GetDlgItem(hParent, constraint->id);

		/*if (bIgnoreHidden)
			if (!IsWindowVisible(hControl))
			{
				constraint = constraint->next;

				continue;
			}*/

		RECT rect;
		GetWindowRect(hControl, &rect);
		MapWindowPoints(NULL, hParent, (LPPOINT)&rect, 2);

		int xPos = rect.left, 
			yPos = rect.top, 
			xWidth = rect.right - rect.left,
			yHeight = rect.bottom - rect.top;

		SetSizeAndPos(xWidth, xPos, constraint, true, nWidth, rect.left, rect.right, prevWidth, hParent);
		SetSizeAndPos(yHeight, yPos, constraint, false, nHeight, rect.top, rect.bottom, prevHeight, hParent);

		MoveWindow(hControl, 
			xPos,
			yPos,
			xWidth,
			yHeight,
			TRUE);		// Force repaint because called in WM_SIZE message.

		if (constraint->bForceInvalidate)
		{
			InvalidateRect(hControl, NULL, TRUE);
		}

		constraint = constraint->next;
	}

	prevWidth = nWidth;
	prevHeight = nHeight;
}


void SimpleLayoutManager::DoLayout()
{
	RECT Rect;
	GetClientRect(hParent, &Rect);
	DoLayout(SIZE_RESTORED, MAKELPARAM(Rect.right, Rect.bottom));
}