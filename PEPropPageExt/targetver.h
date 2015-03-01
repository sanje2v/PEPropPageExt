#pragma once

// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <winsdkver.h>
//
#define NTDDI_VERSION		NTDDI_WS03SP1
#define _WIN32_WINNT		_WIN32_WINNT_WS03
#define WINVER				_WIN32_WINNT_WS03

#include <SDKDDKVer.h>

#ifndef _USING_V110_SDK71_
#define _USING_V110_SDK71_
#endif
