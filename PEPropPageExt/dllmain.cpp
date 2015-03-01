#include "stdafx.h"


// Module object from 'CPEPropPageExtModule' class
CPEPropPageExtModule g_PEPropPageExtModule;

// DLL Entry Point
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			g_PEPropPageExtModule.setInstance(hInstance);
			DisableThreadLibraryCalls(hInstance);

			break;
	}

	return g_PEPropPageExtModule.DllMain(dwReason, pReserved);
}

STDAPI DllCanUnloadNow()
{
	return g_PEPropPageExtModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return g_PEPropPageExtModule.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer()
{
	return g_PEPropPageExtModule.DllRegisterServer(FALSE);
}

STDAPI DllUnregisterServer()
{
	return g_PEPropPageExtModule.DllUnregisterServer(FALSE);
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
	HRESULT hr = E_FAIL;
	static const wchar_t szUserSwitch[] = L"user";

	if (pszCmdLine != NULL)
		if (_wcsnicmp(pszCmdLine, szUserSwitch, ARRAYSIZE(szUserSwitch)) == 0)
			AtlSetPerUserRegistration(true);

	if (bInstall)
	{
		hr = DllRegisterServer();

		if (FAILED(hr))
			DllUnregisterServer();
	}
	else
		hr = DllUnregisterServer();

	return hr;
}