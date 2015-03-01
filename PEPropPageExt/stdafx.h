#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW
#define _USRDLL
#define _WINDOWS

// Disable some unneeded portion of 'Windows.h' header
#define NOVIRTUALKEYCODES
#define NOSYSMETRICS
#define NOKEYSTATES
#define NOSYSCOMMANDS
//#define NORASTEROPS
//#define NOCOLOR
#define NODRAWTEXT
#define NOOPENFILE
//#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

#ifndef FACILITY_VISUALCPP
#define FACILITY_VISUALCPP  ((LONG)0x6d)
#endif

#define STRINGIFY(x)   #x

#include <Windows.h>
#include "resource.h"
#include "CommonDefs.h"
#include <atlbase.h>
#include <atlcom.h>
#include <cassert>
#include <string>
#include <memory>

using namespace std;
using namespace ATL;

#define szCLSID_PEPROPPAGEEXT		"{8C762F3E-2463-4012-B54F-EBD3FCF89563}"
extern const CLSID CLSID_PEPropPageExt;


class CPEPropPageExtModule : public CAtlDllModuleT<CPEPropPageExtModule>
{
	HINSTANCE m_hInstance;
public:
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_REGISTRATIONSCRIPT, szCLSID_PEPROPPAGEEXT)

	void setInstance(HINSTANCE hInstance) { m_hInstance = hInstance; }
	HINSTANCE getInstance() { return m_hInstance; }
};

typedef class DECLSPEC_UUID(szCLSID_PEPROPPAGEEXT) PEPropPageExt PEPropPageExt;