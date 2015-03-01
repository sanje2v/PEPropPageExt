#include "stdafx.h"
#include "PropertyPageHandler.h"


PropertyPageHandler_ExceptionHandling::PropertyPageHandler_ExceptionHandling(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, std::ref(PEReaderWriter))
{
	m_hEditExceptions = GetDlgItem(m_hWnd, IDC_EDITEXCEPTIONS);

	// Setup controls with layout manager
	m_LayoutManager.AddChildConstraint(IDC_EDITEXCEPTIONS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Set font for rich edit
	CHARFORMAT CharFormat;
	ZeroMemory(&CharFormat, sizeof(CharFormat));

	CharFormat.cbSize = sizeof(CharFormat);
	CharFormat.dwMask = CFM_FACE;
	CopyMemory(CharFormat.szFaceName, szPreferredFont, wcslen(szPreferredFont) + sizeof(WCHAR));
	SendMessage(m_hEditExceptions, EM_SETCHARFORMAT, SCF_DEFAULT, LPARAM(&CharFormat));

	// Set tab stop  for rich edit
	DWORD _cTabs = cTabs;
	Edit_SetTabStops(m_hEditExceptions, 1, &_cTabs);
}

void PropertyPageHandler_ExceptionHandling::OnInitDialog()
{
	// Fill controls with data
	vector<TextAndData> ExceptionItemsInfo;
	wstring ExceptInfo = L"Name\tAddress\n"
		                 L"---------------------------------------------------------\n";

	static auto funcMakeExceptionHandlingDesc = [](wstring& info, wstring label, wstring value)
	{
		info.append(label + L":\t" + value + L'\n');
	};

	const int NoOfExceptionHandlers = m_PEReaderWriter.getNoOfExceptionHandlers();

	int err;
	switch (m_PEReaderWriter.getMachineType())
	{
		case PEReadWrite::MachineType::Alpha_AXP:
			for (int i = 0; i < NoOfExceptionHandlers; ++i)
			{
				PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY pExceptionHandler;
				err = m_PEReaderWriter.getExceptionHandler(std::ref(pExceptionHandler), i);
				if (err)
				{
					LogError(L"ERROR: Exception handling data of type 'IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY' at index " + DWORD_toString(i) + L" is incomplete. File is not valid.", true);
					break;
				}

				funcMakeExceptionHandlingDesc(ExceptInfo, L"Begin Address (VA)", DWORD_toString(pExceptionHandler->BeginAddress, Hexadecimal, true));
				funcMakeExceptionHandlingDesc(ExceptInfo, L"End Address (VA)", DWORD_toString(pExceptionHandler->EndAddress, Hexadecimal, true));
				funcMakeExceptionHandlingDesc(ExceptInfo, L"Exception Handler (VA)", DWORD_toString(pExceptionHandler->ExceptionHandler, Hexadecimal, true));
				funcMakeExceptionHandlingDesc(ExceptInfo, L"Handler Data", DWORD_toString(pExceptionHandler->HandlerData, Hexadecimal, true));
				funcMakeExceptionHandlingDesc(ExceptInfo, L"Prolog End Address (VA)", DWORD_toString(pExceptionHandler->PrologEndAddress, Hexadecimal, true));

				ExceptInfo.append(L"\n");
			}
			break;

		case PEReadWrite::MachineType::Alpha64:
			for (int i = 0; i < NoOfExceptionHandlers; ++i)
			{
				PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY pExceptionHandler;
				err = m_PEReaderWriter.getExceptionHandler(std::ref(pExceptionHandler), i);
				if (err)
				{
					LogError(L"ERROR: Exception handling data of type 'IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY' at index " + DWORD_toString(i) + L" is incomplete. File is not valid.", true);
					break;
				}
				
				funcMakeExceptionHandlingDesc(ExceptInfo, L"Begin Address (VA)", QWORD_toString(pExceptionHandler->BeginAddress, Hexadecimal, true));
				funcMakeExceptionHandlingDesc(ExceptInfo, L"End Address (VA)", QWORD_toString(pExceptionHandler->EndAddress, Hexadecimal, true));
				funcMakeExceptionHandlingDesc(ExceptInfo, L"Exception Handler", QWORD_toString(pExceptionHandler->ExceptionHandler, Hexadecimal, true));
				funcMakeExceptionHandlingDesc(ExceptInfo, L"Handler Data", QWORD_toString(pExceptionHandler->HandlerData, Hexadecimal, true));
				funcMakeExceptionHandlingDesc(ExceptInfo, L"Prolog End Address (VA)", QWORD_toString(pExceptionHandler->PrologEndAddress, Hexadecimal, true));

				ExceptInfo.append(L"\n");
			}
			break;

		case PEReadWrite::MachineType::ARM_LE:
		case PEReadWrite::MachineType::PowerPC_LE:
		case PEReadWrite::MachineType::PowerPCFPU:
		case PEReadWrite::MachineType::HitachiSH3:
		case PEReadWrite::MachineType::HitachiSH3DSP:
		case PEReadWrite::MachineType::HitachiSH3_LE:
		case PEReadWrite::MachineType::HitachiSH4:
			for (int i = 0; i < NoOfExceptionHandlers; ++i)
			{
				PIMAGE_CE_RUNTIME_FUNCTION_ENTRY pExceptionHandler;
				err = m_PEReaderWriter.getExceptionHandler(std::ref(pExceptionHandler), i);
				if (err)
				{
					LogError(L"ERROR: Exception handling data of type 'IMAGE_CE_RUNTIME_FUNCTION_ENTRY' at index " + DWORD_toString(i) + L" is incomplete. File is not valid.", true);
					break;
				}
				
				funcMakeExceptionHandlingDesc(ExceptInfo, L"Begin Address (VA)", DWORD_toString(pExceptionHandler->FuncStart, Hexadecimal, true));
				funcMakeExceptionHandlingDesc(ExceptInfo, L"Prolog Length", DWORD_toString(pExceptionHandler->PrologLen) + L"\tNo. of instructions in function's prolog");
				funcMakeExceptionHandlingDesc(ExceptInfo, L"Function Length", DWORD_toString(pExceptionHandler->FuncLen) + L"\tNo. of instructions in function");
				funcMakeExceptionHandlingDesc(ExceptInfo, L"32-bit Flag", DWORD_toString(pExceptionHandler->ThirtyTwoBit, Hexadecimal) + L"\t" +
						  ExceptionArch_toString(pExceptionHandler->ThirtyTwoBit));
				funcMakeExceptionHandlingDesc(ExceptInfo, L"Exception Flag", DWORD_toString(pExceptionHandler->ExceptionFlag, Hexadecimal) + L"\t" +
						  ExceptionFlag_toString(pExceptionHandler->ExceptionFlag));

				ExceptInfo.append(L"\n");
			}
			break;

		case PEReadWrite::MachineType::x64:
		case PEReadWrite::MachineType::Itanium:
			for (int i = 0; i < NoOfExceptionHandlers; ++i)
			{
				PIMAGE_RUNTIME_FUNCTION_ENTRY pExceptionHandler;
				err = m_PEReaderWriter.getExceptionHandler(std::ref(pExceptionHandler), i);
				if (err)
				{
					LogError(L"ERROR: Exception handling data of type 'IMAGE_RUNTIME_FUNCTION_ENTRY' at index " + DWORD_toString(i) + L" is incomplete. File is not valid." , true);
					break;
				}

				funcMakeExceptionHandlingDesc(ExceptInfo, L"Begin Address (RVA)", DWORD_toString(pExceptionHandler->BeginAddress, Hexadecimal, true));
				funcMakeExceptionHandlingDesc(ExceptInfo, L"End Address (RVA)", DWORD_toString(pExceptionHandler->EndAddress, Hexadecimal, true));
				funcMakeExceptionHandlingDesc(ExceptInfo, L"Unwind Info (RVA)", DWORD_toString(pExceptionHandler->UnwindInfoAddress, Hexadecimal, true));

				ExceptInfo.append(L"\n");
			}
			break;
	}

	// Remove the two trailing newline characters
	if (ExceptInfo.size() > 2)
		ExceptInfo.resize(ExceptInfo.size() - 2);

	Edit_SetText(m_hEditExceptions, LPWSTR(ExceptInfo.c_str()));
	// NOTE: Cannot use macros in the following because they use 'SendMessage' which didn't work.
	//	Apparently, using the 'Edit_GetLineCount' macro is just fine.
	PostMessage(m_hEditExceptions, EM_SETSEL, -1, 0);
	PostMessage(m_hEditExceptions, EM_LINESCROLL, 0, -Edit_GetLineCount(m_hEditExceptions));
}