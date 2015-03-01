#pragma once

#define BOM_UTF8										(DWORD) 0x00BFBBEF
#define BOM_UTF16										(WORD) 0xFEFF

#define THREAD_ISOLATED_STORAGE							_declspec(thread)
#define TestFlag(val, flag)								(((val) & (flag)) == (flag))
#define GetArraySize(arr)								(unsigned int) (sizeof(arr) / sizeof(*arr))
#define GOTO_RELEASE_HANDLER(step, _retval)				{ Retval = _retval; goto ReleaseHandler##step; }
#define DEFINE_RELEASE_HANDLER(no, instruc)				ReleaseHandler##no: instruc
#define SAFE_RELEASE(ptr)								if (ptr) { delete (ptr); (ptr) = NULL; }
#define MAKEDWORDLONG(a,b)								((DWORDLONG)(((DWORD)(a))|(((DWORDLONG)((DWORD)(b)))<<32)))
#define SET_DWORD_FIRST_BIT(val, isset)					((DWORD) (val) | (((bool) isset) ? 0x80000000 : 0x0))
#define GET_DWORD_FIRST_BIT(val)						((bool) ((DWORD)(val) & 0x80000000))
#define REMOVE_DWORD_FIRST_BIT(val)						((DWORD)(val) & ~0x80000000)
#define RemoveTrailingCommaSpace(val)					(((val).length() > 2 ? (val).resize((val).length() - 2) : NULL), (val))
#define RemoveTrailingSpace(val)						(((val).length() > 1 ? (val).resize((val).length() - 1) : NULL), (val))
#define Window_SetStyle(hwnd, style)					SetWindowLongPtrW((hwnd), GWL_STYLE, GetWindowLongPtrW((hwnd), GWL_STYLE) | (style))
#define ListView_Reset(hwnd)							while (ListView_DeleteColumn((hwnd), 0)); ListView_DeleteAllItems((hwnd))
#define Edit_SetLimitText(hwnd, count)                  SendMessage((hwnd), EM_SETLIMITTEXT, WPARAM(count), NULL);
#define Tooltip_Create(hwndParent, hInst)				CreateWindowExW(NULL, TOOLTIPS_CLASSW, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, \
																		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, (hwndParent), NULL, \
																		(hInst), NULL)
#define Tooltip_AddTool(hwnd, lptoolinfo)				SendMessageW((hwnd), TTM_ADDTOOL, 0, (LPARAM)(lptoolinfo))