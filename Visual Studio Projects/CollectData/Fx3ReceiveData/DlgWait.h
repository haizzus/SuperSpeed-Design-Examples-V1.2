#pragma once


// CDlgWait dialog

class CDlgWait : public CDialog
{
	DECLARE_DYNAMIC(CDlgWait)

public:
	CDlgWait(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgWait();

// Dialog Data
	enum { IDD = IDD_DLG_WAIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg void OnBtnClickedCancel();
    afx_msg void OnBtnClickedOk();
    afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
    virtual void PostNcDestroy();
public:
    BOOL m_bDontClose;
    CWnd* m_pParent;
};
