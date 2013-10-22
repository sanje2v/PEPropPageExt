#pragma once

#ifdef UNICODE
#define tstring					wstring
#define tstringstream			wstringstream
#else
#define tstring					string
#define tstringstream			stringstream
#endif

#define BOM_UTF8										(DWORD) 0x00BFBBEF
#define BOM_UTF16										(WORD) 0xFEFF

#define THREAD_ISOLATED_STORAGE							_declspec(thread)
#define TestFlag(val, flag)								(((val) & (flag)) == (flag))
#define GetArraySize(arr)								(unsigned int) (sizeof(arr) / sizeof(*arr))
#define GOTO_RELEASE_HANDLER(step, _retval)				{ Retval = _retval; goto ReleaseHandler##step; }
#define DEFINE_RELEASE_HANDLER(no, instruc)				ReleaseHandler##no: instruc
#define SAFE_RELEASE(ptr)								if (ptr) { delete (ptr); (ptr) = NULL; }
#define LODWORD(l)										((DWORD)((DWORDLONG)(l)))
#define HIDWORD(l)										((DWORD)(((DWORDLONG)(l)>>32)&0xFFFFFFFF))
#define MAKEDWORDLONG(a,b)								((DWORDLONG)(((DWORD)(a))|(((DWORDLONG)((DWORD)(b)))<<32)))
#define SET_DWORD_FIRST_BIT(val, isset)					((DWORD) (val) | (((bool) isset) ? 0x80000000 : 0x0))
#define GET_DWORD_FIRST_BIT(val)						((bool) ((DWORD)(val) & 0x80000000))
#define REMOVE_DWORD_FIRST_BIT(val)						((DWORD)(val) & ~0x80000000)
#define RemoveTrailingSpace(val)						(((val).length() > 2 ? (val).resize((val).length() - 2) : NULL), val)
#define Window_SetStyle(hwnd, style)					SetWindowLongPtr((hwnd), GWL_STYLE, GetWindowLongPtr((hwnd), GWL_STYLE) | (style))