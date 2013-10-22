// dllmain.h : Declaration of module class.

class CPEPropPageExtModule : public ATL::CAtlDllModuleT< CPEPropPageExtModule >
{
public:
	HINSTANCE hInstance;
	DECLARE_LIBID(LIBID_PEPropPageExtLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_PEPROPPAGEEXT, "{59FD3190-AB7F-4226-9530-5BDD9CC9F95E}")
};

extern class CPEPropPageExtModule _AtlModule;