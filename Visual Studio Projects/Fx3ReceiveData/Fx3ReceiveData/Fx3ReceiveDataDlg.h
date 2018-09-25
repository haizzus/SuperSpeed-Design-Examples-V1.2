
// Fx3ReceiveDataDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "DlgWait.h"
#define WM_DATA_TRANSFER_COMPLETE   (WM_USER + 100)
#define WM_DATA_TRANSFER_STARTED    (WM_USER + 103)

typedef struct _CY_DATA_BUFFER {

    UCHAR *buffer;                      /*Pointer to the buffer from where the data is read/written */
    UINT32 length;                      /*Length of the buffer */
    _CY_DATA_BUFFER *pNextData;
} CY_DATA_BUFFER,*PCY_DATA_BUFFER;

// CFx3ReceiveDataDlg dialog
class CFx3ReceiveDataDlg : public CDialogEx
{
// Construction
public:
	CFx3ReceiveDataDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FX3RECEIVEDATA_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
    HDEVNOTIFY *m_hDeviceNotify;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT DataTransferComplete(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT DataTransferStarted(WPARAM wParam, LPARAM lParam);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
    static LRESULT PerformStreamerDataCollection(LPVOID lpParam);
    static LRESULT DataWriteRoutine(LPVOID lpParam);
    BOOL m_bBreakStreamerThreadCollection;
    static CCriticalSection _critSect;
    static PCY_DATA_BUFFER pHead;
public:
    afx_msg void OnBnClickedRadCounter();
    afx_msg void OnBnClickedStartDataTransfer();
    afx_msg void OnBnClickedBtnFile();
    afx_msg void OnBnClickedCancel();

    BOOL RegisterDeviceInterface();
    BOOL SurveyExistingDevices();
    BOOL CalculateTransferSpeed(SYSTEMTIME objStartTime, long bytesXferred);
    
    CComboBox m_cboDevices;
    CEdit m_edtFile;
    CButton m_radCounter;
    CComboBox m_cboTimeout;
    CButton m_btnTransfer;
    CDlgWait* m_pInfoMessage;

    HANDLE m_hDataQueueEvent;
    HANDLE m_hFileData;
    HANDLE m_hWriteCompleted;
    afx_msg void OnBnClickedOk();
};
