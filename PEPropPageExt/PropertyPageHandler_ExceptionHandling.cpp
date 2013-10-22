#include "PropertyPageHandler.h"


#define FillData2(stringobj, label, value)	(stringobj).append(label _T("\t") + tstring(value) + _T('\n'))


void PropertyPageHandler_ExceptionHandling::OnInitDialog()
{
	hEditExceptions = GetDlgItem(m_hWnd, IDC_EDITEXCEPTIONS);

	// Setup controls with layout manager
	m_pLayoutManager->AddChildConstraint(IDC_EDITEXCEPTIONS, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Set font for rich edit
	CHARFORMAT CharFormat;
	static const LPTSTR szPreferredFont = _T("MS Shell Dlg");

	ZeroMemory(&CharFormat, sizeof(CHARFORMAT));
	CharFormat.cbSize = sizeof(CHARFORMAT);
	CharFormat.dwMask = CFM_FACE;
	CopyMemory(CharFormat.szFaceName, szPreferredFont, _tcslen(szPreferredFont) + sizeof(TCHAR));
	SendMessage(hEditExceptions, EM_SETCHARFORMAT, SCF_DEFAULT, (LPARAM) &CharFormat);

	// Set tab stop  for rich edit
	DWORD cTabs = 100;
	Edit_SetTabStops(hEditExceptions, 1, &cTabs);

	// Fill controls with data
	vector<TextAndData> ExceptionItemsInfo;
	PIMAGE_NT_HEADERS pNTheaders = m_PEReaderWriter.GetSecondaryHeader<PIMAGE_NT_HEADERS>();
	tstring buffer = _T("Name\tAddress\n-----------------------------------------------------\n");

	switch (pNTheaders->FileHeader.Machine)
	{
	case IMAGE_FILE_MACHINE_ALPHA:
		for (int i = 0; i < m_PEReaderWriter.GetNoOfExceptionHandlers(); i++)
		{
			IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY &ExceptionHandler =
														*m_PEReaderWriter.GetExceptionHandler<PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY>(i);

			FillData2(buffer, _T("Begin Address (VA)"), DWORD_toString(ExceptionHandler.BeginAddress, Hexadecimal));
			FillData2(buffer, _T("End Address (VA)"), DWORD_toString(ExceptionHandler.EndAddress, Hexadecimal));
			FillData2(buffer, _T("Exception Handler (VA)"), DWORD_toString(ExceptionHandler.ExceptionHandler, Hexadecimal));
			FillData2(buffer, _T("Handler Data"), DWORD_toString(ExceptionHandler.HandlerData, Hexadecimal));
			FillData2(buffer, _T("Prolog End Address (VA)"), DWORD_toString(ExceptionHandler.PrologEndAddress, Hexadecimal));

			buffer.append(_T("\n"));
		}

		break;

	case IMAGE_FILE_MACHINE_ALPHA64:
		for (int i = 0; i < m_PEReaderWriter.GetNoOfExceptionHandlers(); i++)
		{
			IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY &ExceptionHandler =
														*m_PEReaderWriter.GetExceptionHandler<PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY>(i);
				
			FillData2(buffer, _T("Begin Address (VA)"), QWORD_toString(ExceptionHandler.BeginAddress, Hexadecimal));
			FillData2(buffer, _T("End Address (VA)"), QWORD_toString(ExceptionHandler.EndAddress, Hexadecimal));
			FillData2(buffer, _T("Exception Handler"), QWORD_toString(ExceptionHandler.ExceptionHandler, Hexadecimal));
			FillData2(buffer, _T("Handler Data"), QWORD_toString(ExceptionHandler.HandlerData, Hexadecimal));
			FillData2(buffer, _T("Prolog End Address (VA)"), QWORD_toString(ExceptionHandler.PrologEndAddress, Hexadecimal));

			buffer.append(_T("\n"));
		}

		break;

	case IMAGE_FILE_MACHINE_ARM:
	case IMAGE_FILE_MACHINE_POWERPC:
	case IMAGE_FILE_MACHINE_POWERPCFP:
	case IMAGE_FILE_MACHINE_SH3:
	case IMAGE_FILE_MACHINE_SH3DSP:
	case IMAGE_FILE_MACHINE_SH3E:
	case IMAGE_FILE_MACHINE_SH4:
		for (int i = 0; i < m_PEReaderWriter.GetNoOfExceptionHandlers(); i++)
		{
			IMAGE_CE_RUNTIME_FUNCTION_ENTRY &ExceptionHandler = *m_PEReaderWriter.GetExceptionHandler<PIMAGE_CE_RUNTIME_FUNCTION_ENTRY>(i);
				
			FillData2(buffer, _T("Begin Address (VA)"), DWORD_toString(ExceptionHandler.FuncStart, Hexadecimal));
			FillData2(buffer, _T("Prolog Length"), DWORD_toString(ExceptionHandler.PrologLen) + _T("\tNo. of instructions in function's prolog"));
			FillData2(buffer, _T("Function Length"), DWORD_toString(ExceptionHandler.FuncLen)+ _T("\tNo. of instructions in function"));
			FillData2(buffer, _T("32-bit Flag"), DWORD_toString(ExceptionHandler.ThirtyTwoBit, Hexadecimal) + _T("\t") +
																					ExceptionArch_toString(ExceptionHandler.ThirtyTwoBit));
			FillData2(buffer, _T("Exception Flag"), DWORD_toString(ExceptionHandler.ExceptionFlag, Hexadecimal) + _T("\t") +
																					ExceptionFlag_toString(ExceptionHandler.ExceptionFlag));

			buffer.append(_T("\n"));
		}

		break;

	case IMAGE_FILE_MACHINE_AMD64:
	case IMAGE_FILE_MACHINE_IA64:
		for (int i = 0; i < m_PEReaderWriter.GetNoOfExceptionHandlers(); i++)
		{
			IMAGE_RUNTIME_FUNCTION_ENTRY &ExceptionHandler = *m_PEReaderWriter.GetExceptionHandler<PIMAGE_RUNTIME_FUNCTION_ENTRY>(i);

			FillData2(buffer, _T("Begin Address (RVA)"), DWORD_toString(ExceptionHandler.BeginAddress, Hexadecimal));
			FillData2(buffer, _T("End Address (RVA)"), DWORD_toString(ExceptionHandler.EndAddress, Hexadecimal));
			FillData2(buffer, _T("Unwind Info (RVA)"), DWORD_toString(ExceptionHandler.UnwindInfoAddress, Hexadecimal));

			buffer.append(_T("\n"));
		}

	}

	// Remove the two trailing newline characters
	buffer.resize(buffer.size() - 2);

	Edit_SetText(hEditExceptions, (LPTSTR) buffer.c_str());
	// NOTE: Cannot use macros in the following because they use 'SendMessage' which didn't work.
	//	Apparently, using the 'Edit_GetLineCount' macro is just fine.
	PostMessage(hEditExceptions, EM_SETSEL, -1, 0);
	PostMessage(hEditExceptions, EM_LINESCROLL, 0, -Edit_GetLineCount(hEditExceptions));
}