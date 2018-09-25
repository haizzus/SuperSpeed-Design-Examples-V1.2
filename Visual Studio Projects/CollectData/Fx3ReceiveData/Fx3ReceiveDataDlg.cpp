
// Fx3ReceiveDataDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Fx3ReceiveData.h"
#include "Fx3ReceiveDataDlg.h"
#include <dbt.h>
#include "..\Library\inc\cyapi.h"

														// Start with modest buffering 8 x 8 x 16KB = 1MB
														// Can go up to 256 x 64 x 16KB = 256MB but this will mask some effects
#define     PACKETS_PER_TRANSFER                8		// 256
#define     NUM_TRANSFER_PER_TRANSACTION        8		// 64
#define     TIMEOUT_PER_TRANSFER_MILLI_SEC      1500
#define     MAX_QUEUE_SZ                        64
#define     BULK_END_POINT                      2
#define     STOP_DATA_COLLECTION_EVENT          33

#define     VENDOR_CMD_START_TRANSFER           0xB5

ULONG deleteIteration = 0;



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CFx3ReceiveDataDlg dialog


CCriticalSection CFx3ReceiveDataDlg::_critSect;
PCY_DATA_BUFFER CFx3ReceiveDataDlg::pHead = NULL;

CFx3ReceiveDataDlg::CFx3ReceiveDataDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFx3ReceiveDataDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFx3ReceiveDataDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CBO_DEVICES, m_cboDevices);
    DDX_Control(pDX, IDC_EDT_FILE, m_edtFile);
    DDX_Control(pDX, IDC_BTN_FILE, m_radCounter);
    DDX_Control(pDX, IDC_CBO_TIMEOUT, m_cboTimeout);
    DDX_Control(pDX, IDC_BTN_START_TRANSFER, m_btnTransfer);
}

BEGIN_MESSAGE_MAP(CFx3ReceiveDataDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_WM_TIMER()
    ON_BN_CLICKED(IDCANCEL, &CFx3ReceiveDataDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BTN_FILE, &CFx3ReceiveDataDlg::OnBnClickedBtnFile)
    ON_BN_CLICKED(IDC_RAD_COUNTER, &CFx3ReceiveDataDlg::OnBnClickedRadCounter)
    ON_BN_CLICKED(IDC_BTN_START_TRANSFER, &CFx3ReceiveDataDlg::OnBnClickedStartDataTransfer)
    ON_MESSAGE(WM_DATA_TRANSFER_COMPLETE, &CFx3ReceiveDataDlg::DataTransferComplete)
    ON_MESSAGE(WM_DATA_TRANSFER_STARTED, &CFx3ReceiveDataDlg::DataTransferStarted)
END_MESSAGE_MAP()


// CFx3ReceiveDataDlg message handlers

BOOL CFx3ReceiveDataDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    m_hDeviceNotify = NULL;
    RegisterDeviceInterface();
    SurveyExistingDevices();

    ((CButton *)GetDlgItem(IDC_RADIO2))->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_CBO_TIMEOUT)->EnableWindow(FALSE);
    GetDlgItem(IDC_STC_TIMEOUT)->EnableWindow(FALSE);
    GetDlgItem(IDC_STC_MILLI)->EnableWindow(FALSE);

    ((CButton *)GetDlgItem(IDC_RAD_COUNTER))->SetCheck(1);
    ((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(0);

    m_bBreakStreamerThreadCollection = FALSE;
    
    m_hDataQueueEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hWriteCompleted = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hFileData = INVALID_HANDLE_VALUE;
    m_pInfoMessage = NULL;


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFx3ReceiveDataDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFx3ReceiveDataDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFx3ReceiveDataDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFx3ReceiveDataDlg::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    m_bBreakStreamerThreadCollection = TRUE;
    SetEvent(m_hDataQueueEvent);
    SetEvent(m_hWriteCompleted);
    if (m_hDeviceNotify != NULL )
    {
        UnregisterDeviceNotification(*m_hDeviceNotify);    
        delete m_hDeviceNotify;
        m_hDeviceNotify = NULL;
    }
    Sleep(100);
    CloseHandle(m_hDataQueueEvent);
    CloseHandle(m_hWriteCompleted);
    OnCancel();
}

LRESULT CFx3ReceiveDataDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DEVICECHANGE && wParam >= DBT_DEVICEARRIVAL)
    {
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
        if (wParam == DBT_DEVICEARRIVAL && lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
        {               
            SurveyExistingDevices();
        }
        else if (wParam == DBT_DEVICEREMOVECOMPLETE && lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
        {
            SurveyExistingDevices();
        }
        lpdb->dbch_devicetype;
        lpdb->dbch_size;
    }
    return CDialogEx::DefWindowProc(message, wParam, lParam);
}

BOOL CFx3ReceiveDataDlg::RegisterDeviceInterface()
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = CYUSBDRV_GUID;

    if (m_hDeviceNotify == NULL ) m_hDeviceNotify = new HDEVNOTIFY;
    *m_hDeviceNotify = RegisterDeviceNotification( 
        this->m_hWnd,                       // events recipient
        &NotificationFilter,        // type of device
        DEVICE_NOTIFY_WINDOW_HANDLE // type of recipient handle
        );

    if ( NULL == *m_hDeviceNotify ) 
    {
        //ErrorHandler(TEXT("RegisterDeviceNotification"));
        delete m_hDeviceNotify;
        m_hDeviceNotify = NULL;
        return FALSE;
    }

    return TRUE;
}

BOOL CFx3ReceiveDataDlg::SurveyExistingDevices()
{
    CCyUSBDevice	*USBDevice;
    USBDevice = new CCyUSBDevice(this->m_hWnd, CYUSBDRV_GUID, true);
    m_cboDevices.ResetContent();

    if (USBDevice != NULL) 
    {
        int nDeviceCount = USBDevice->DeviceCount();
        for (int nCount = 0; nCount < nDeviceCount; nCount++ )
        {
            CString strDeviceData;
            USBDevice->Open(nCount);
			// Only put Streamer devices in the menu - mmmm, this didn't work
//			if ((USBDevice->VendorID == 0x04B4) && (USBDevice->ProductID == 0x00F1))
//			{
				strDeviceData.Format(L"(0x%04X - 0x%04X) %s", USBDevice->VendorID, USBDevice->ProductID, CString(USBDevice->FriendlyName));
				m_cboDevices.InsertString(nCount, strDeviceData);
//			}
        }
        delete USBDevice;
        if (m_cboDevices.GetCount() >= 1 ) m_cboDevices.SetCurSel(0);
        SetFocus();
    }
    else return FALSE;

    return TRUE;
}
void CFx3ReceiveDataDlg::OnBnClickedBtnFile()
{
    // TODO: Add your control notification handler code here
    TCHAR szFilters[]= _T("FX3 log Files (*.bin)|*.bin|All Files (*.*)|*.*||");
    CFileDialog fileDlg(FALSE,  _T("bin"), _T("CollectData.bin"), OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY, szFilters);
    if (fileDlg.DoModal() != IDOK ) return;
    CString file = fileDlg.GetFileName();
    CString path = fileDlg.GetFolderPath();
    m_edtFile.SetWindowText(path + L"\\" + file);

    ((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(1);
    ((CButton *)GetDlgItem(IDC_RAD_COUNTER))->SetCheck(0);
    
    GetDlgItem(IDC_CBO_TIMEOUT)->EnableWindow(TRUE);
    GetDlgItem(IDC_STC_TIMEOUT)->EnableWindow(TRUE);
    GetDlgItem(IDC_STC_MILLI)->EnableWindow(TRUE);

    m_cboTimeout.SetCurSel(4);
        
}

void CFx3ReceiveDataDlg::OnBnClickedRadCounter()
{
    // TODO: Add your control notification handler code here
    m_edtFile.SetWindowText(L"");
    GetDlgItem(IDC_CBO_TIMEOUT)->EnableWindow(FALSE);
    GetDlgItem(IDC_STC_TIMEOUT)->EnableWindow(FALSE);
    GetDlgItem(IDC_STC_MILLI)->EnableWindow(FALSE);

    m_cboTimeout.SetCurSel(-1);
}

void CFx3ReceiveDataDlg::OnBnClickedStartDataTransfer()
{
    // TODO: Add your control notification handler code here
    CString strText;
    m_btnTransfer.GetWindowText(strText);

    if (strText.CompareNoCase(L"Start Data Transfer") == 0 )
    {
        // Start the data transfer.
        m_bBreakStreamerThreadCollection = FALSE;
        m_btnTransfer.SetWindowText(L"Stop Data Transfer");
        AfxBeginThread((AFX_THREADPROC)CFx3ReceiveDataDlg::PerformStreamerDataCollection, (LPVOID)this);
    }
    else
    {
        // Stop the data transfer.
        m_btnTransfer.SetWindowText(L"Start Data Transfer");
        m_bBreakStreamerThreadCollection = TRUE;
        SetEvent(m_hDataQueueEvent);
        if (this->m_cboTimeout.IsWindowEnabled() )
            KillTimer(STOP_DATA_COLLECTION_EVENT);
    }
    
}

LRESULT CFx3ReceiveDataDlg::DataTransferComplete(WPARAM wParam, LPARAM lParam)
{
    m_btnTransfer.SetWindowText(L"Start Data Transfer");
    m_bBreakStreamerThreadCollection = TRUE;
    SetEvent(m_hDataQueueEvent);
    return 1;
}

LRESULT CFx3ReceiveDataDlg::DataWriteRoutine(LPVOID lpParam)
{
    CFx3ReceiveDataDlg *pThis = (CFx3ReceiveDataDlg *)lpParam;
    PCY_DATA_BUFFER pWorkingSet = pHead;

    DWORD dwWritten = 0;
    if (pWorkingSet == NULL ) 
    {
        // Make sure link list head is initialized...........
        do {
            WaitForSingleObject(pThis->m_hDataQueueEvent, INFINITE);        
        } while(pHead == NULL);

        // Wait till the two datas are available to write.
        if (pHead->pNextData == NULL )
            WaitForSingleObject(pThis->m_hDataQueueEvent, INFINITE);

        if (pWorkingSet == NULL ) pWorkingSet = pHead;        
    }

    ////////////////////////////////////////////////////////////////////////////////
    /////////////// Let's start the data write loop /////////////////////////////////
    while (pWorkingSet != NULL || pThis->m_bBreakStreamerThreadCollection == FALSE )
    {
        ////////// Write the device data to the file //////////////////////////////////////////////////
        ////////// At USB super speed, there is no hard drive can handle this back pressure.///////////
        ////////// So, This operation will have overlap errors, so data integrity can't maintained.////
        WriteFile(pThis->m_hFileData, pWorkingSet->buffer, pWorkingSet->length, &dwWritten, NULL);

        /// Traverse through the link list data structure.////////////////////////
        if (pWorkingSet->pNextData == NULL )
        {
            do {
            if (pWorkingSet->pNextData == NULL) WaitForSingleObject(pThis->m_hDataQueueEvent, INFINITE);

            _critSect.Lock();
            if (pWorkingSet->pNextData == NULL && pThis->m_bBreakStreamerThreadCollection == TRUE ) 
            {
                _critSect.Unlock();
                break;
            }
            _critSect.Unlock();
            }while (pWorkingSet->pNextData == NULL);
        }
        ///////// We are good to loop for the next operation..............///////////
        _critSect.Lock();
        pHead = pHead->pNextData;
        delete pWorkingSet;        
        pWorkingSet = pHead;
        deleteIteration++;
        _critSect.Unlock();
    }

    /////////////////////////////////////////////////////////////////////////////////
    /////////// All write operation is at completion////////////////////////////////
    /////////// Now, signal the data collection thread to clean up the resource.////
    /////////////////////////////////////////////////////////////////////////////////
    CloseHandle(pThis->m_hFileData);
    SetEvent(pThis->m_hWriteCompleted);
    pThis->m_hFileData = INVALID_HANDLE_VALUE;
    pHead = NULL;
    return 1;
}

LRESULT CFx3ReceiveDataDlg::PerformStreamerDataCollection(LPVOID lpParam)
{
    // Allocate the arrays needed for queueing
    UCHAR QueueSize = NUM_TRANSFER_PER_TRANSACTION;
    CFx3ReceiveDataDlg *pThis = (CFx3ReceiveDataDlg *)lpParam;

    PUCHAR			*buffers		= new PUCHAR[QueueSize];
    PUCHAR			*contexts		= new PUCHAR[QueueSize];
    OVERLAPPED		inOvLap[MAX_QUEUE_SZ];
    int             nCount          = 0;
    long            BytesXferred    = 0;
    CString         strFile         = L"";
    BOOL            isItSuperSpeed  = FALSE;
    PCY_DATA_BUFFER pPrevBuffer     = NULL;
    ULONG iteration = 0;
    
    // We are asked to run the data collection job in the file.

    // Sanity check on the file to dump data .
    // Is the file open already?, if so please close the file.
    //
    if (pThis->m_hFileData != INVALID_HANDLE_VALUE ) CloseHandle(pThis->m_hFileData);
    pThis->m_hFileData = INVALID_HANDLE_VALUE;
    
    // Is this operation meant for discard data or file write?
    if (pThis->GetDlgItem(IDC_CBO_TIMEOUT)->IsWindowEnabled() == TRUE )
    {
        // This operation for file write.     
        // Get the file name and the path.
        pThis->m_edtFile.GetWindowText(strFile);
        // It is safe to delete file rather than overwrite such a huge file.
        DeleteFile(strFile);
        pThis->m_hFileData = CreateFile(strFile, (GENERIC_READ | GENERIC_WRITE), FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        
        // Is the file creation succeed?
        if (pThis->m_hFileData == INVALID_HANDLE_VALUE )
        {
            //
            // No, its not. Some issues in creating the file.
            // Lets display the windows error code from the failed file creation operation.
            //
            CString strMsg;
            strMsg.Format(L"Unable to create the log file and Windows OS returned a error code \"0x%X\". Bailing out data collection procedure..", GetLastError());
            AfxMessageBox(strMsg);

            // do the needed Cleanup.
            delete []buffers;
            delete []contexts;
            // We are done. Time to bail out.
            pThis->PostMessage(WM_DATA_TRANSFER_COMPLETE, 0, 0);
            return 6;
        }
        // Yeah we are into doing file and we need setup a timeout.
        // Read the time and set it up.
        CString strData;
        //pThis->m_cboTimeout.GetLBText(pThis->m_cboTimeout.GetCurSel(), strData);
        pThis->m_cboTimeout.GetWindowText(strData);
        int nElapseTime = _wtoi(strData.GetBuffer(0));
        // Ontimer callback function will be called at the time out.
        pThis->SetTimer(STOP_DATA_COLLECTION_EVENT, (nElapseTime*1000), NULL);
    }

    // Open the right device and get the device end point now.
    int nDeviceIndex = pThis->m_cboDevices.GetCurSel();
    if (nDeviceIndex == -1 ) 
    {
        AfxMessageBox(L"No device available to run the data collection routine");

        // do the needed Cleanup.
        delete []buffers;
        delete []contexts;
        if (pThis->m_hFileData != INVALID_HANDLE_VALUE ) CloseHandle(pThis->m_hFileData);
        // No device to performan any operation, bail out....
        pThis->PostMessage(WM_DATA_TRANSFER_COMPLETE, 0, 0);
        return 1;
    }

    // Get the USB Device Instance Going.
    CCyUSBDevice	*USBDevice = new CCyUSBDevice(pThis->m_hWnd, CYUSBDRV_GUID, true);

    ////////////////////////////////////////////////////////////
    // Try open the device in ASynchronous mode (Overlapped IO)
    // Cypress Library opens the device for Overlapped IO.
    ////////////////////////////////////////////////////////////
    if (USBDevice->Open(nDeviceIndex) == false ) 
    {
        // Failed to open the device. Bail out......

        AfxMessageBox(L"Unable to Open the Device, exiting the data collection routine");

        // do the needed Cleanup.
        delete []buffers;
        delete []contexts;
        if (pThis->m_hFileData != INVALID_HANDLE_VALUE ) CloseHandle(pThis->m_hFileData);
        delete USBDevice;
        pThis->PostMessage(WM_DATA_TRANSFER_COMPLETE, 0, 0);

        return 2;
    }

    // Is the connected device works at super speed rate?
    isItSuperSpeed = USBDevice->bSuperSpeed;    

    //////////////////////////////////////////////////////////////
    // Find the first bulk IN endpoint needed for the transactions.
    //////////////////////////////////////////////////////////////
    UCHAR nEndPointCount = USBDevice->EndPointCount();
    CCyUSBEndPoint *ept = NULL;
    for (nCount = 0; nCount < nEndPointCount; nCount++ )
    {
        ept = USBDevice->EndPoints[nCount];
        if (ept->Attributes == BULK_END_POINT && ept->bIn == true)
            break;
    }

    if (nCount >= nEndPointCount ) 
    {
        // Something wrong. We missed to find any BULK IN endpont. Bail Out..........
        AfxMessageBox(L"Unable to find the Device BULK - IN endpoint, exiting the data collection routine");

        // do the needed cleanup.
        delete []buffers;
        delete []contexts;
        if (pThis->m_hFileData != INVALID_HANDLE_VALUE && isItSuperSpeed == FALSE) CloseHandle(pThis->m_hFileData);
        delete USBDevice;
        pThis->PostMessage(WM_DATA_TRANSFER_COMPLETE, 0, 0);

        return 3;
    }

    //
    // Calculate the max packet size (USB Frame Size).
    // From bulk burst transfer, this size represent bulk burst size and this 
    // size now belongs to multiple USB frames
    //
    long totalTransferSize = ept->MaxPktSize * PACKETS_PER_TRANSFER;
    ept->SetXferSize(totalTransferSize);

    // Allocate all the buffers for the queues
    for (nCount = 0; nCount < QueueSize; nCount++) 
    { 
        buffers[nCount]        = new UCHAR[totalTransferSize];
        inOvLap[nCount].hEvent = CreateEvent(NULL, false, false, NULL);

        memset(buffers[nCount],0xEF,totalTransferSize);
    }

    SYSTEMTIME objStartTime;
    GetSystemTime(&objStartTime);

    // Queue-up the first batch of transfer requests
    for (nCount = 0; nCount < QueueSize; nCount++)	
    {
        ////////////////////BeginDataXFer will kick start the IN transactions.................
        contexts[nCount] = ept->BeginDataXfer(buffers[nCount], totalTransferSize, &inOvLap[nCount]);
        if (ept->NtStatus || ept->UsbdStatus) 
        {
            // BeginDataXfer failed
            // Handle the error now.
            ept->Abort();
            for (int j=0; j< QueueSize; j++)
            {
                CloseHandle(inOvLap[j].hEvent);
                delete [] buffers[j];        
            }   

            // Bail out......
            delete []contexts;
            if (pThis->m_hFileData != INVALID_HANDLE_VALUE && isItSuperSpeed == FALSE) CloseHandle(pThis->m_hFileData);
            delete USBDevice;
            
            AfxMessageBox(L"Unable to queue the application capture buffer, device did not respond correctly...");
            pThis->PostMessage(WM_DATA_TRANSFER_COMPLETE, 0, 0);
            return 4;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    /////// Send start notification to the device //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    if (USBDevice->bSuperSpeed )
    {
        LONG dataLength = 0;
        CCyControlEndPoint *ControlEndPt = (CCyControlEndPoint *)USBDevice->EndPointOf(0);

        ControlEndPt->Target    = TGT_DEVICE; 
        ControlEndPt->ReqType   = REQ_VENDOR; 
        // Vendor Command that is transmitted for starting the read.
        ControlEndPt->ReqCode   = VENDOR_CMD_START_TRANSFER;
        ControlEndPt->Direction = DIR_TO_DEVICE;
        //// Send Value = 1 and Index = 0, to kick start the transaction.
        ControlEndPt->Value     = 0x0001;
        ControlEndPt->Index     = 0;

        // Send vendor command now......
        ControlEndPt->XferData(NULL, dataLength, NULL);        
    }



    ////////////////////////////////////////////////////////////////////////////
    /*Launch a data write thread to the disk for super speed operation....*/
    //////////////////////////////////////////////////////////////////////////////
    if (isItSuperSpeed == TRUE && pThis->m_hFileData != INVALID_HANDLE_VALUE)
        AfxBeginThread((AFX_THREADPROC)DataWriteRoutine, (LPVOID)pThis);

    ////////////////////////////////////////////////////////////////////////
    // Start the data collection phase..................
    ///////////////////////////////////////////////////////////////////////
    nCount = 0;
    BytesXferred = 0;
    while ( pThis->m_bBreakStreamerThreadCollection == FALSE )
    {
        long readLength = totalTransferSize;        

        //////////Wait till the transfer completion..///////////////////////////
        if (!ept->WaitForXfer(&inOvLap[nCount], TIMEOUT_PER_TRANSFER_MILLI_SEC))
        {
            ept->Abort();
            if (ept->LastError == ERROR_IO_PENDING)
                WaitForSingleObject(inOvLap[nCount].hEvent, TIMEOUT_PER_TRANSFER_MILLI_SEC);
        }

        ////////////Read the trasnferred data from the device///////////////////////////////////////
        if (ept->FinishDataXfer(buffers[nCount], readLength, &inOvLap[nCount], contexts[nCount])) 
            BytesXferred += totalTransferSize;

        //////////BytesXFerred is need for current data rate calculation.
        ///////// Refer to CalculateTransferSpeed function for the exact 
        ///////// calculation.............................
        if (BytesXferred < 0) // Rollover - reset counters
        {
            BytesXferred = 0;
            GetSystemTime(&objStartTime);
        }
        
        ///////////////////////////////////////////////////////////
        /// Time to write the data to the file, if needed /////////
        ///////////////////////////////////////////////////////////

        DWORD dwWrite = 0;
        if (pThis->m_hFileData != INVALID_HANDLE_VALUE ) 
        {
            ///////////File writting is valid, so push the data to the file.////////////
            //// Is this device super speed device?
            if (isItSuperSpeed == TRUE )
            {
                ///////////////////////////////////////////////////////
                /// Don't block the data transfer by write here.
                /// Let's push the data to a link list and 
                /// the disk write thread will push the data to the file.
                /////////////////////////////////////////////////////

                _critSect.Lock();
                CY_DATA_BUFFER *pBuffer = new CY_DATA_BUFFER;
                pBuffer->pNextData = NULL;
                
                pBuffer->buffer = buffers[nCount];
                pBuffer->length = totalTransferSize;

                if (pPrevBuffer == NULL ) pPrevBuffer = pBuffer;
                else 
                {
                    pPrevBuffer->pNextData = pBuffer;
                    pPrevBuffer = pBuffer;
                }

                if (pHead == NULL ) pHead = pBuffer;
                else SetEvent(pThis->m_hDataQueueEvent);
                _critSect.Unlock();

                ///////////////////Link List Population completes///////////
            }
            else
            {
                ///// For non super speed devices, write the data. We have lots of time.
                WriteFile(pThis->m_hFileData, buffers[nCount], totalTransferSize, &dwWrite, NULL);
            }

        }
        
        // Re-submit this queue element to keep the queue full
        contexts[nCount] = ept->BeginDataXfer(buffers[nCount], totalTransferSize, &inOvLap[nCount]);
        if (ept->NtStatus || ept->UsbdStatus) 
        {
            // BeginDataXfer failed............
            // Time to bail out now............
            ept->Abort();
            for (int j=0; j< QueueSize; j++)
            {
                CloseHandle(inOvLap[j].hEvent);
                delete [] buffers[j];        
             }   
            delete []contexts;
            delete USBDevice;
            if (pThis->m_hFileData != INVALID_HANDLE_VALUE && isItSuperSpeed == FALSE) CloseHandle(pThis->m_hFileData);

            AfxMessageBox(L"Error queuing the application buffer now, bailing out from data collection routine...");
            pThis->PostMessage(WM_DATA_TRANSFER_COMPLETE, 0, 0);
            return 5;
        }

        /////////////////////////////////////////////////////////////////
        //////////// Calculate the data rate now............//////////////
        nCount++;
        if (nCount == QueueSize )
        {
            nCount = 0;
            if (readLength ) pThis->CalculateTransferSpeed(objStartTime, BytesXferred);
            else
            {
                float fValue = (float)readLength;
                CString strData;
                strData.Format(L"%.2f", fValue);
                pThis->GetDlgItem(IDC_STC_DATA_RATE)->SetWindowText(strData);
                BytesXferred = 0;
                GetSystemTime(&objStartTime);
            }
        }
        
    }

    ///////////////////////////////////////////////////////////////
    ///////////// Alright, we out of data collection loop./////////
    ///////////// Stop can happen from User pressing the button or
    ///////////// Time out can trigger the exit. //////////////////

    ////////////////////////////////////////////////////////////////////////////
    /////// Send stop notification to the device //////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    if (USBDevice->bSuperSpeed )
    {
        LONG dataLength = 0;
        CCyControlEndPoint *ControlEndPt = (CCyControlEndPoint *)USBDevice->EndPointOf(0);

        ControlEndPt->Target    = TGT_DEVICE; 
        ControlEndPt->ReqType   = REQ_VENDOR; 
        // Vendor Command that is transmitted for starting the read.
        ControlEndPt->ReqCode   = VENDOR_CMD_START_TRANSFER;
        ControlEndPt->Direction = DIR_TO_DEVICE;
        //// Send Value = 0 and Index = 0, to stop the transaction.
        ControlEndPt->Value     = 0x0000;
        ControlEndPt->Index     = 0;

        // Send vendor command now......
        ControlEndPt->XferData(NULL, dataLength, NULL);        
    }
    ept->Abort();

    ///////////////////////////////////////////////////////////////////////
    ////////////// Wait for the write thread complete writting the data.
    ////////////// And then do the memory cleanup.
    ////////////////////////////////////////////////////////////////////////

    if (pThis->m_hFileData != INVALID_HANDLE_VALUE && isItSuperSpeed == TRUE) 
    {
        ////////////////////////////////////////////////////////////////////////////
        //// Now pop up the notification to the user saying that write operation is
        //// still going on.
        /////////////////////////////////////////////////////////////////////////////
        
        pThis->PostMessage(WM_DATA_TRANSFER_STARTED, 0, 0);
        WaitForSingleObject(pThis->m_hWriteCompleted, INFINITE);

        if (pThis->m_pInfoMessage )
            pThis->m_pInfoMessage->PostMessage(WM_CLOSE, 0, 0);           
    }

    ///////////////////////////////////////////////////////////
    ////////////////////Perform Memory cleanup now/////////////
    ///////////////////////////////////////////////////////////
    
    for (int j=0; j< QueueSize; j++)
    {
        CloseHandle(inOvLap[j].hEvent);
        delete [] buffers[j];        
    }   
    delete []contexts;
    delete USBDevice;
    if (pThis->m_hFileData != INVALID_HANDLE_VALUE && isItSuperSpeed == FALSE) {
        CloseHandle(pThis->m_hFileData);
        pThis->m_hFileData = INVALID_HANDLE_VALUE;
    }
    pThis->PostMessage(WM_DATA_TRANSFER_COMPLETE, 0, 0);    
    return 0;
}

BOOL CFx3ReceiveDataDlg::CalculateTransferSpeed(SYSTEMTIME objStartTime, long bytesXferred)
{
    SYSTEMTIME objCurTime;
    GetSystemTime(&objCurTime);

    DWORD curTimeInMS = (objCurTime.wMinute * 60 * 1000) + (objCurTime.wSecond * 1000) + (objCurTime.wMilliseconds);
    DWORD startTimeInMS = (objStartTime.wMinute * 60 * 1000) + (objStartTime.wSecond * 1000) + (objStartTime.wMilliseconds);
    DWORD elapsedTimeInMS = (curTimeInMS - startTimeInMS);

    // Bytes transfer in Millisecond = Killo Bytes transfer / Second
    double transferRate = 0;
    if (elapsedTimeInMS != 0 ) transferRate = (bytesXferred / elapsedTimeInMS); 
    if (transferRate == 0 && this->m_bBreakStreamerThreadCollection == FALSE ) return FALSE;
    
    CString strData;
    strData.Format(L"%.2f", transferRate);
    GetDlgItem(IDC_STC_DATA_RATE)->SetWindowText(strData);

    return TRUE;
}

void CFx3ReceiveDataDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == STOP_DATA_COLLECTION_EVENT )
    {
        m_bBreakStreamerThreadCollection = TRUE;
        m_btnTransfer.SetWindowText(L"Start Data Transfer");
        SetEvent(m_hDataQueueEvent);
        KillTimer(nIDEvent);
        return;
    }
    CDialogEx::OnTimer(nIDEvent);
}

LRESULT CFx3ReceiveDataDlg::DataTransferStarted(WPARAM wParam, LPARAM lParam)
{
    m_pInfoMessage = new CDlgWait(this);
    m_pInfoMessage->Create(CDlgWait::IDD, GetDesktopWindow());
	m_pInfoMessage->ShowWindow(SW_SHOW);

    return 1;
}