// DlgWait.cpp : implementation file
//

#include "stdafx.h"
#include "Fx3ReceiveData.h"
#include "DlgWait.h"
#include "Fx3ReceiveDataDlg.h"


// CDlgWait dialog

IMPLEMENT_DYNAMIC(CDlgWait, CDialog)

CDlgWait::CDlgWait(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgWait::IDD, pParent)
{
    m_bDontClose = TRUE;
    m_pParent = pParent;
}

CDlgWait::~CDlgWait()
{
}

void CDlgWait::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgWait, CDialog)
    ON_BN_CLICKED(IDOK, OnBtnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBtnClickedCancel)
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDlgWait message handlers
void CDlgWait::OnBtnClickedCancel()
{
    if (m_bDontClose ) return;

    DestroyWindow();
}

void CDlgWait::OnBtnClickedOk()
{
}
void CDlgWait::PostNcDestroy()
{
	CDialog::PostNcDestroy();
    if(m_pParent) ((CFx3ReceiveDataDlg *)m_pParent)->m_pInfoMessage = NULL;
	delete this;
}

void CDlgWait::OnClose()
{
    DestroyWindow();
}
