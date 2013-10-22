#include "PropertyPageHandler.h"


void PropertyPageHandler_Resources::OnInitDialog()
{
	HWND hComboResourceName = GetDlgItem(m_hWnd, IDC_CMBRESOURCENAME);
	HWND hComboResourceLang = GetDlgItem(m_hWnd, IDC_CMBRESOURCELANG);
	HWND hListResourceType = GetDlgItem(m_hWnd, IDC_LISTRESOURCETYPE);
	HWND hStaticResourcePreview = GetDlgItem(m_hWnd, IDC_STATICRESOURCEPREVIEW);

	// Setup controls
	m_pLayoutManager->AddChildConstraint(IDC_LISTRESOURCETYPE, CWA_LEFTRIGHT);
	m_pLayoutManager->AddChildConstraint(IDC_STATICRESOURCEPREVIEW, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Fill them with data
}