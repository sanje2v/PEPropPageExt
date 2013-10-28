#include "PropertyPageHandler.h"


PropertyPageHandler_Resources::PropertyPageHandler_Resources(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, PEReaderWriter)
{
	m_hComboResourceName = GetDlgItem(m_hWnd, IDC_CMBRESOURCENAME);
	m_hComboResourceLang = GetDlgItem(m_hWnd, IDC_CMBRESOURCELANG);
	m_hListResourceType = GetDlgItem(m_hWnd, IDC_LISTRESOURCETYPE);
	m_hStaticResourcePreview = GetDlgItem(m_hWnd, IDC_STATICRESOURCEPREVIEW);

	// Setup controls
	m_pLayoutManager->AddChildConstraint(IDC_LISTRESOURCETYPE, CWA_LEFTRIGHT);
	m_pLayoutManager->AddChildConstraint(IDC_STATICRESOURCEPREVIEW, CWA_LEFTRIGHT, CWA_TOPBOTTOM);
}

void PropertyPageHandler_Resources::OnInitDialog()
{
	// Fill them with data
}