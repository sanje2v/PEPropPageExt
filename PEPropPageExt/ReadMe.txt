Project: PEPropPageExt
Version: 1.0
Description: This project aims to create a Windows Explorer property page extension for EXE and DLL files.
			 The property page extension can show binary data in human readable form. These information is
			 helpful to programmers.
License: The author disclaims any liability from the use of this product. Both product and source code are
		 free for research and personal use. Commercial use is restricted.

Installing: Copy 'PEPropPageExt.dll' and 'ManagedFuncs.dll' to installation directory. With command prompt at
			installation directory and opened with administrative previleges, enter 'regsvr32 PEPropPageExt.dll'.
Uninstalling: With command prompt opened with administrative previlaiges, enter 'regsvr32 \u PEPropPageExt.dll'.
			  You can now delete the files 'PEPropPageExt.dll' and 'ManagedFuncs.dll'.

Regarding GNU C++ unmangling: Make sure the files 'libstdc++-6.dll' and 'libgcc_s_seh-1.dll' are in 'System32'
							  directory. These files are distributed with MinGW installations.

Known Issues:
1. RTTI information used by context menu in Imports page, Debug Info page and Load Config page is not correct for file offset