#pragma once

using namespace System::Runtime::InteropServices;

///////////// P/Invoke Windows APIs /////////////
typedef void*					HICON;
typedef void*					HBITMAP;
typedef void*					HANDLE;
typedef void*                   HWND;

#ifdef _WIN64
typedef unsigned long long		PTR;
#else
typedef unsigned long			PTR;
#endif

typedef unsigned int			UINT;
typedef int						INT;
typedef long					LONG;
typedef unsigned int            DWORD;
typedef unsigned short			WORD;
typedef unsigned char			BYTE;
typedef void *					LPVOID;
typedef void *                  HGDIOBJ;

enum
{
	IMAGE_BITMAP = 0,
	IMAGE_ICON = 1,
	IMAGE_CURSOR = 2
};

#define WINAPI	__stdcall

typedef struct tagBITMAP {
	LONG   bmType;
	LONG   bmWidth;
	LONG   bmHeight;
	LONG   bmWidthBytes;
	WORD   bmPlanes;
	WORD   bmBitsPixel;
	LPVOID bmBits;
} BITMAP, *PBITMAP;

[DllImport("user32", CharSet=CharSet::Unicode)]
extern "C" HWND CreateWindowEx(DWORD dwExStyle, wchar_t *lpClassName, wchar_t *lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, void *hMenu, void* hInstance, void *lpParam);

[DllImport("user32", CharSet=CharSet::Unicode)]
extern "C" long long SendMessage(HWND hWnd, int Msg, void *wParam, void *lParam);

[DllImport("gdi32")]
extern "C" int GetObject(HGDIOBJ hgdiobj, int cbBuffer, LPVOID lpvObject);

[DllImport("gdi32")]
LONG GetBitmapBits(HBITMAP hbmp, LONG cbBuffer, LPVOID lpvBits);

[DllImport("kernel32")]
DWORD GetLastError();

[DllImport("user32")]
extern "C" HICON CopyImage(HANDLE hImage, UINT uType, INT cxDesired, INT cyDesired, UINT fuFlags);
/////////////// End of P/Invoke ///////////////