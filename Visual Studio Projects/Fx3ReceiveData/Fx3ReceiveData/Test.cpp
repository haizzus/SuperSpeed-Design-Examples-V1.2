LRESULT CFx3ReceiveDataDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DEVICECHANGE && wParam >= DBT_DEVICEARRIVAL)
    {
        int devNumber = -1;
        ULONG size = 0;
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
        if (lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE )
        {
            PDEV_BROADCAST_DEVICEINTERFACE pDev = (PDEV_BROADCAST_DEVICEINTERFACE)(lParam);
            pDev->dbcc_classguid;
            size = lstrlen(pDev->dbcc_name);

            CString szDevId = pDev->dbcc_name+4;
            int idx = szDevId.ReverseFind(_T('#'));
            ASSERT( -1 != idx );
            szDevId.Truncate(idx);
            szDevId.Replace(_T('#'), _T('\\'));
            szDevId.MakeUpper();

            CString szClass;
            idx = szDevId.Find(_T('\\'));
            ASSERT(-1 != idx );
            szClass = szDevId.Left(idx);

            DWORD dwFlag = DBT_DEVICEARRIVAL != wParam
                                ? DIGCF_ALLCLASSES : (DIGCF_ALLCLASSES | DIGCF_PRESENT);
            HDEVINFO hDevInfo = SetupDiGetClassDevs(NULL, szClass, NULL, dwFlag);
            if( INVALID_HANDLE_VALUE != hDevInfo )
            {
                 SP_DEVINFO_DATA* pspDevInfoData =
                            (SP_DEVINFO_DATA*)HeapAlloc(GetProcessHeap(), 0, sizeof(SP_DEVINFO_DATA));
                            pspDevInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
                for(devNumber = 0; SetupDiEnumDeviceInfo(hDevInfo,devNumber,pspDevInfoData); devNumber++)
                {
                    DWORD nSize=0 ;
                    TCHAR buf[MAX_PATH];

                    if ( !SetupDiGetDeviceInstanceId(hDevInfo, pspDevInfoData, buf, sizeof(buf), &nSize) )
                    {
                        AfxMessageBox(CString("SetupDiGetDeviceInstanceId(): ")
                            + _com_error(GetLastError()).ErrorMessage(), MB_ICONEXCLAMATION);
                        break;
                    }

                    if ( szDevId == buf )
                    {
                        break;
                    }
                }
                if ( pspDevInfoData ) HeapFree(GetProcessHeap(), 0, pspDevInfoData);
                SetupDiDestroyDeviceInfoList(hDevInfo);                
            }

        }
    }
    return CDialogEx::DefWindowProc(message, wParam, lParam);

}