//---------------------------------------------------------------------------

// USBtestDlg.cpp : implementation file
#include "stdafx.h"
#include "afxmt.h"  
#include "USBtest.h"
#include "USBtestDlg.h"
#include "ftd2xx.h"
#include "process.h"
#include <iostream>


using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
static char THIS_FILE[] = __FILE__;
#endif

//globals

int running = 0;
int board_present;
int bus_busy;
int temp_add = 0;


unsigned char continue_get_data_sign = 0, draw_data_sign = 0;
unsigned short ccd_int[3648];

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
		//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUSBtestDlg dialog

CUSBtestDlg::CUSBtestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUSBtestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUSBtestDlg)
	m_PortStatus = _T("");
	m_EchoRes = _T("");
	m_NumRecd = 0;
	m_128status = _T("");
	m_SerDesc = 0;
	m_fram_index = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CUSBtestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUSBtestDlg)
	DDX_Control(pDX, IDC_SPIN1, m_int_time_btn);
	DDX_Text(pDX, IDC_EDIT_PORT_STATUS, m_PortStatus);
	DDX_Text(pDX, IDC_EDIT_NAME_NUMBER, m_NameNmbr);
	DDX_Radio(pDX, IDC_RADIO_DEVNO, m_SerDesc);
	DDX_Text(pDX, IDC_EDIT1, m_fram_index);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUSBtestDlg, CDialog)
	//{{AFX_MSG_MAP(CUSBtestDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_BLINK_ON, OnButtonBlinkOn)
	ON_BN_CLICKED(IDC_BUTTON_BLINK_OFF, OnButtonBlinkOff)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_HELLO, OnButtonHello)
	ON_BN_CLICKED(IDC_RADIO_SERNUM, OnRadioSernum)
	ON_BN_CLICKED(IDC_RADIO_DEVNO, OnRadioDevno)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnButtonSearch)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_EN_CHANGE(IDC_INT_TIME_EDIT, OnChangeIntTimeEdit)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUSBtestDlg message handlers

void CUSBtestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
CDC* temp_pDC;
CGdiObject* pOldBrush;
CPen* pPenRed;
//--------------------------------------------
void CUSBtestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();

		CBrush* pBrushBlack = new CBrush;  //添加是否正确  
		pBrushBlack->CreateSolidBrush(RGB(255, 255, 255));  //添加是否正确  
		CPaintDC dc(this); // device context for painting  
	//以下代码为绘出画实时曲线的背景子窗口  
		CWnd* pWnd = GetDlgItem(IDC_curve_paint);
		//获取绘制曲线的文本框指针  
		pWnd->Invalidate();
		pWnd->UpdateWindow();
		temp_pDC = pWnd->GetDC();

		pOldBrush = temp_pDC->SelectObject(pBrushBlack);
		temp_pDC->Rectangle(0, 0, 1110, 350);//绘制画曲线的矩形区域  
		temp_pDC->SelectObject(pOldBrush);

		pPenRed = new CPen;
		pPenRed->CreatePen(PS_SOLID, 2, RGB(255, 0, 0));

	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CUSBtestDlg::OnQueryDragIcon()
{
	return (HCURSOR)m_hIcon;
}

//***********************************************************************
BOOL CUSBtestDlg::PreTranslateMessage(MSG* pMsg)
{

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
		return TRUE;
	else
		return CDialog::PreTranslateMessage(pMsg);
}
//***********************************************************************
//***********************************************************************
//***********************************************************************
//***********************************************************************
BOOL CUSBtestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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

	CheckDlgButton(IDC_RADIO_DEVNO, 0);
	m_int_time_btn.SetRange(1, 100);
	m_int_time_btn.SetPos(5);

	// TODO: Add extra initialization here
	bus_busy = 0;
	board_present = 0;
	SetTimer(15, 1, NULL);//check for new USB data in the buffer every 100mS


//	m_Slide.SetRange(0, 500, TRUE);

	//read the last settings from the registry
	m_SerDesc = AfxGetApp()->GetProfileInt("MyUSBTestAp", "SerDesc", 0);//default to Description

/*
	if(m_SerDesc==0)
		m_NameNmbr = AfxGetApp()->GetProfileString("MyUSBTestAp", "DescString", "Enter device Description or Serial Number here.");
	if(m_SerDesc==1)
		m_NameNmbr = AfxGetApp()->GetProfileString("MyUSBTestAp", "SerialString", "Enter device Description or Serial Number here.");
	if(m_SerDesc==2)
		m_NameNmbr = AfxGetApp()->GetProfileString("MyUSBTestAp", "DevNmbr", "Enter device Description or Serial Number here.");
*/
	UpdateData(FALSE);


	Loadem();//load DLL and test for presense of DLP-USB1 and DLP-USB2

	return TRUE;  // return TRUE  unless you set the focus to a control
}



//****************************************************************************
void CUSBtestDlg::OnButtonBlinkOn()
{
	if (!board_present)
		return;

	//	AfxBeginThread(ThreadProc1, this, THREAD_PRIORITY_NORMAL);
}


//****************************************************************************


//**********************************************************************************

//**********************************************************************************

void CUSBtestDlg::OnTimer(UINT nIDEvent)
{
	if (!board_present)
		return;
	if (!running)
		return;
	//--------------------------------------------------------------
	//	get_ccd_data();
	if (draw_data_sign == 2)
	{
		OnDataLine();
		draw_data_sign = 0;
	}
	m_fram_index = temp_add;
	UpdateData(0);
	//---------------------------------------------------
	CDialog::OnTimer(nIDEvent);
}





//****************************************************************************
void CUSBtestDlg::OnButtonBlinkOff()
{


	stop_data_cmd();


	//---------------------------------------------------------------------
	//        DEV0Item->Enabled=false; OpenButton->Enabled=false;
	//        PortCloseItem->Enabled=true; ClosePortButton->Enabled=true;


}




//****************************************************************************
BOOL CUSBtestDlg::OnCommand(WPARAM wParam, LPARAM lParam) //disable the ESC key
{
	if (wParam == 2) return FALSE;
	return CDialog::OnCommand(wParam, lParam);
}


//*******************************************************************

void CUSBtestDlg::LoadDLL()
{
	m_hmodule = LoadLibrary("Ftd2xx.dll");
	if (m_hmodule == NULL)
	{
		AfxMessageBox("Error: Can't Load ft8u245.dll");
		return;
	}

	m_pWrite = (PtrToWrite)GetProcAddress(m_hmodule, "FT_Write");
	if (m_pWrite == NULL)
	{
		AfxMessageBox("Error: Can't Find FT_Write");
		return;
	}

	m_pRead = (PtrToRead)GetProcAddress(m_hmodule, "FT_Read");
	if (m_pRead == NULL)
	{
		AfxMessageBox("Error: Can't Find FT_Read");
		return;
	}

	m_pOpen = (PtrToOpen)GetProcAddress(m_hmodule, "FT_Open");
	if (m_pOpen == NULL)
	{
		AfxMessageBox("Error: Can't Find FT_Open");
		return;
	}

	m_pOpenEx = (PtrToOpenEx)GetProcAddress(m_hmodule, "FT_OpenEx");
	if (m_pOpenEx == NULL)
	{
		AfxMessageBox("Error: Can't Find FT_OpenEx");
		return;
	}

	m_pListDevices = (PtrToListDevices)GetProcAddress(m_hmodule, "FT_ListDevices");
	if (m_pListDevices == NULL)
	{
		AfxMessageBox("Error: Can't Find FT_ListDevices");
		return;
	}

	m_pClose = (PtrToClose)GetProcAddress(m_hmodule, "FT_Close");
	if (m_pClose == NULL)
	{
		AfxMessageBox("Error: Can't Find FT_Close");
		return;
	}

	m_pResetDevice = (PtrToResetDevice)GetProcAddress(m_hmodule, "FT_ResetDevice");
	if (m_pResetDevice == NULL)
	{
		AfxMessageBox("Error: Can't Find FT_ResetDevice");
		return;
	}

	m_pPurge = (PtrToPurge)GetProcAddress(m_hmodule, "FT_Purge");
	if (m_pPurge == NULL)
	{
		AfxMessageBox("Error: Can't Find FT_Purge");
		return;
	}

	m_pSetTimeouts = (PtrToSetTimeouts)GetProcAddress(m_hmodule, "FT_SetTimeouts");
	if (m_pSetTimeouts == NULL)
	{
		AfxMessageBox("Error: Can't Find FT_SetTimeouts");
		return;
	}

	m_pGetQueueStatus = (PtrToGetQueueStatus)GetProcAddress(m_hmodule, "FT_GetQueueStatus");
	if (m_pGetQueueStatus == NULL)
	{
		AfxMessageBox("Error: Can't Find FT_GetQueueStatus");
		return;
	}
}


//****************************************************************************************
FT_STATUS CUSBtestDlg::Read(LPVOID lpvBuffer, DWORD dwBuffSize, LPDWORD lpdwBytesRead)
{
	if (!m_pRead)
	{
		AfxMessageBox("FT_Read is not valid!");
		return FT_INVALID_HANDLE;
	}

	return (*m_pRead)(m_ftHandle, lpvBuffer, dwBuffSize, lpdwBytesRead);
}


//****************************************************************************************
FT_STATUS CUSBtestDlg::Write(LPVOID lpvBuffer, DWORD dwBuffSize, LPDWORD lpdwBytes)
{
	if (!m_pWrite)
	{
		AfxMessageBox("FT_Write is not valid!");
		return FT_INVALID_HANDLE;
	}

	return (*m_pWrite)(m_ftHandle, lpvBuffer, dwBuffSize, lpdwBytes);
}





//****************************************************************************************
FT_STATUS CUSBtestDlg::Open(PVOID pvDevice)
{
	if (!m_pOpen)
	{
		AfxMessageBox("FT_Open is not valid!");
		return FT_INVALID_HANDLE;
	}

	return (*m_pOpen)(pvDevice, &m_ftHandle);
}

//****************************************************************************************
FT_STATUS CUSBtestDlg::OpenEx(PVOID pArg1, DWORD dwFlags)
{
	if (!m_pOpenEx)
	{
		AfxMessageBox("FT_OpenEx is not valid!");
		return FT_INVALID_HANDLE;
	}

	return (*m_pOpenEx)(pArg1, dwFlags, &m_ftHandle);
}


//****************************************************************************************
FT_STATUS CUSBtestDlg::ListDevices(PVOID pArg1, PVOID pArg2, DWORD dwFlags)
{
	if (!m_pListDevices)
	{
		AfxMessageBox("FT_ListDevices is not valid!");
		return FT_INVALID_HANDLE;
	}

	return (*m_pListDevices)(pArg1, pArg2, dwFlags);
}


//****************************************************************************************
FT_STATUS CUSBtestDlg::Close()
{
	if (!m_pClose)
	{
		AfxMessageBox("FT_Close is not valid!");
		return FT_INVALID_HANDLE;
	}

	return (*m_pClose)(m_ftHandle);
}

//****************************************************************************************
FT_STATUS CUSBtestDlg::ResetDevice()
{
	if (!m_pResetDevice)
	{
		AfxMessageBox("FT_ResetDevice is not valid!");
		return FT_INVALID_HANDLE;
	}

	return (*m_pResetDevice)(m_ftHandle);
}
//****************************************************************************************
FT_STATUS CUSBtestDlg::Purge(ULONG dwMask)
{
	if (!m_pPurge)
	{
		AfxMessageBox("FT_Purge is not valid!");
		return FT_INVALID_HANDLE;
	}

	return (*m_pPurge)(m_ftHandle, dwMask);
}
//****************************************************************************************
FT_STATUS CUSBtestDlg::SetTimeouts(ULONG dwReadTimeout, ULONG dwWriteTimeout)
{
	if (!m_pSetTimeouts)
	{
		AfxMessageBox("FT_SetTimeouts is not valid!");
		return FT_INVALID_HANDLE;
	}

	return (*m_pSetTimeouts)(m_ftHandle, dwReadTimeout, dwWriteTimeout);
}


//****************************************************************************************
FT_STATUS CUSBtestDlg::GetQueueStatus(LPDWORD lpdwAmountInRxQueue)
{
	if (!m_pGetQueueStatus)
	{
		AfxMessageBox("FT_GetQueueStatus is not valid!");
		return FT_INVALID_HANDLE;
	}

	return (*m_pGetQueueStatus)(m_ftHandle, lpdwAmountInRxQueue);
}

//****************************************************************************************
void CUSBtestDlg::OnRadioSernum()   //internal trigger setting
{
	if (!board_present)
		return;

	DWORD ret_bytes;

	Write("#Text:1%", 8, &ret_bytes);

	CheckDlgButton(IDC_RADIO_DEVNO, 0);


	//	UpdateData(TRUE);
	//	m_NameNmbr = AfxGetApp()->GetProfileString("MyUSBTestAp", "SerialString", "Enter device Description or Serial Number here.");
	//	MessageBox("External");
	//	UpdateData(FALSE);
	//	UpdateWindow();
}

//****************************************************************************************
void CUSBtestDlg::OnRadioDevno()   //external trigger setting
{
	if (!board_present)
		return;

	//	CWinApp* pApp = AfxGetApp();  
	//write the new setup to the registry
//	pApp->WriteProfileInt("MyUSBTestAp", "SerDesc", 0);
	DWORD ret_bytes;

	Write("#Text:0%", 8, &ret_bytes);

	CheckDlgButton(IDC_RADIO_SERNUM, 0);

}

//****************************************************************************************
void CUSBtestDlg::Loadem()
{
	unsigned char txbuf[25], rxbuf[25];

	LoadDLL();

	UpdateData(TRUE);

	//open the device
	FT_STATUS status = OpenBy();
	if (status)
	{
		m_PortStatus = _T("USB1 not responding");
		board_present = 0;
		GetDlgItem(IDC_BUTTON_HELLO)->EnableWindow(TRUE);
	}
	else
	{
		ResetDevice();
		SetTimeouts(3000, 3000);//extend timeout while board DLP-USB2 finishes reset
		Purge(FT_PURGE_RX || FT_PURGE_TX);

		m_PortStatus = _T("USB CCD Connected");

		board_present = 1;
	}

	SetTimeouts(300, 300);
	UpdateData(FALSE);
	UpdateWindow();
}



//****************************************************************************************
FT_STATUS CUSBtestDlg::OpenBy()
{
	UpdateData(TRUE);

	FT_STATUS status;
	ULONG x = 0;

	status = Open((PVOID)x);//load default device 0

	return status;
}


//****************************************************************************************
void CUSBtestDlg::OnButtonHello() //code for the "Open" button
{

	//	unsigned char txbuf[25], rxbuf[25];
	//	DWORD ret_bytes;


	UpdateData(TRUE);
	m_PortStatus = _T("-reset-");
	UpdateData(FALSE);
	UpdateWindow();

	Close();

	//open the device
	FT_STATUS status = OpenBy();
	if (status > 0)
	{
		m_PortStatus = _T("Could not open DLP-USB1");
		board_present = 0;
	}
	else
	{

		ResetDevice();
		Purge(FT_PURGE_RX || FT_PURGE_TX);
		ResetDevice();
		SetTimeouts(3000, 3000);//extend timeout while board finishes reset
		Sleep(150);

		m_PortStatus = _T("USB_CCD Connected");

		board_present = 1;
	}

	SetTimeouts(300, 300);
	UpdateData(FALSE);
	UpdateWindow();
}


//****************************************************************************************
void CUSBtestDlg::OnButtonSearch()
{
	//search for Descriptions or Serial Numbers depending on the current mode
	FT_STATUS ftStatus;
	DWORD numDevs;

	Close();//must be closed to perform the ListDevices() function
	UpdateData(TRUE);
	m_PortStatus = _T("DLP-USB1 Closed.");
	m_NumRecd = 0;
	m_NameNmbr = _T("");
	m_Received.ResetContent();
	UpdateData(FALSE);
	UpdateWindow();

	ftStatus = ListDevices(&numDevs, NULL, FT_LIST_NUMBER_ONLY);
	if (ftStatus == FT_OK)
	{
		// FT_ListDevices OK, show number of devices connected in list box
		CString str;
		str.Format("%d device(s) attached:", (int)numDevs);
		m_Received.AddString(str);


		//if current mode is open "by device #" then list device numbers
		if ((m_SerDesc == 2) && (numDevs > 0))
		{
			for (DWORD d = 0; d < numDevs; d++)
			{
				str.Format("%d", d);
				m_Received.AddString(str);
			}

		}



		//if current mode is open "by description" then list descriptions of all connected devices
		if ((m_SerDesc == 0) && (numDevs > 0))
		{
			ftStatus = ListDevices(&numDevs, NULL, FT_LIST_NUMBER_ONLY);
			if (ftStatus == FT_OK)
			{
				char* BufPtrs[64]; // pointer to array of 64 pointers
				for (DWORD d = 0; d < numDevs; d++)
					BufPtrs[d] = new char[64];
				//BufPtrs[d] = NULL;

				ftStatus = ListDevices(BufPtrs, &numDevs, FT_LIST_ALL | FT_OPEN_BY_DESCRIPTION);
				if (FT_SUCCESS(ftStatus))
				{
					for (DWORD u = 0; u < numDevs; u++)
					{
						str.Format("%s", BufPtrs[u]);
						m_Received.AddString(str);
					}
				}
				else
				{
					str.Format("ListDevices failed");
					m_Received.AddString(str);
				}

				//free ram to avoid memory leaks
				for (DWORD d = 0; d < numDevs; d++)
				{
					delete BufPtrs[d];
				}
			}
		}

		//if current mode is open "by serial number" the list descriptions 
		//of all connected devices
		if ((m_SerDesc == 1) && (numDevs > 0))
		{
			//AfxMessageBox("by serial"); 
			ftStatus = ListDevices(&numDevs, NULL, FT_LIST_NUMBER_ONLY);
			if (ftStatus == FT_OK)
			{
				char* BufPtrs[64]; // pointer to array of 64 pointers
				for (DWORD d = 0; d < numDevs; d++)
					BufPtrs[d] = new char[64];
				//BufPtrs[d] = NULL;

				ftStatus = ListDevices(BufPtrs, &numDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
				if (FT_SUCCESS(ftStatus))
				{
					for (DWORD u = 0; u < numDevs; u++)
					{
						str.Format("%s", BufPtrs[u]);
						m_Received.AddString(str);
					}
				}
				else
				{
					str.Format("ListDevices failed");
					m_Received.AddString(str);
				}

				//free ram to avoid memory leaks
				for (DWORD d = 0; d < numDevs; d++)
					delete BufPtrs[d];
			}
		}

	}
	else
	{
		// FT_ListDevices failed
		AfxMessageBox("FT_ListDevices failed");
	}
}



void CUSBtestDlg::OnButton1()
{
	running = 0;
	Close();
	FreeLibrary(m_hmodule);
	exit(1);

}

void CUSBtestDlg::OnChangeIntTimeEdit()
{

	if (!board_present)
		return;

	int baiwei, shiwei, gewei, temp_int_time;

	temp_int_time = m_int_time_btn.GetPos();
	baiwei = temp_int_time / 100;
	shiwei = (temp_int_time % 100) / 10;
	gewei = temp_int_time % 10;

	char temp_str[14];
	sprintf(temp_str, "#CCDInt:%d%d%d%%", baiwei, shiwei, gewei);
	cout << "temp_str " << temp_str<<" "<< baiwei<<" "<< shiwei <<" "<< gewei << endl;
	DWORD ret_bytes;

	Write(temp_str, 12, &ret_bytes);

}


void CUSBtestDlg::OnDataLine()
{

	double temp_y_val, temp_x_val;
	int x_pos, y_pos, i;



	//pPenT=temp_pDC->SelectObject(pPenBlack);  	
//	temp_pDC->SelectObject(pPenT);  	

	temp_pDC->Rectangle(0, 0, 1110, 350);//绘制画曲线的矩形区域  

	CGdiObject* pPenT = temp_pDC->SelectObject(pPenRed);


	//--------------------------------------------------------------
	temp_y_val = 350.0 / 51000;
	temp_x_val = 1110.0 / 3648;
	y_pos = 360 - ccd_int[0] * temp_y_val;
	temp_pDC->MoveTo(0, y_pos);

	for (i = 1; i < 3648; i++)
	{
		x_pos = i * temp_x_val;
		y_pos = 360 - ccd_int[i] * temp_y_val;
		
		temp_pDC->LineTo(x_pos, y_pos);
	}

	temp_pDC->SelectObject(pPenT);

}

BYTE first_run_sign = 1;

void CUSBtestDlg::OnButton2()
{
	

	if (!board_present)
		return;

	if (first_run_sign)
	{
		first_run_sign = 0;
		AfxBeginThread(ThreadProc1, this, THREAD_PRIORITY_NORMAL);
	}
	get_data_cmd();

	//     Timer1->Enabled=true;

	draw_data_sign = 0;

	//		ccd_data_thread->Resume();


}


//****************************************************************************************

void CUSBtestDlg::get_data_cmd()
{
	char* written_buf;
	ULONG bytesWritten;
	written_buf = "#CSDTP:1%";
	BYTE i;
	for (i = 0; i < 2; i++)
		Write(written_buf, 9, &bytesWritten);
	running = 1;

}

void CUSBtestDlg::stop_data_cmd()
{
	const char* written_buf;
	ULONG bytesWritten;
	written_buf = "#CSDTP:0%";
	BYTE i;

	running = 0;

	for (i = 0; i < 200; i++)
	{
		Write(const_cast<char*>(written_buf), 9, &bytesWritten);
	}

}

UINT ThreadProc1(LPVOID param)//count up thread
{
	//typecast the handle to the parent window
	CUSBtestDlg* pMyHndl = (CUSBtestDlg*)param;

	ULONG BytesRead;
	ULONG total_bytes = 0;
	FT_STATUS ftStatus;

	unsigned short ccd_vol_array[3648];
	
	//---------------------------------------------------------------------------------
	unsigned char temp_read_buf[150], total_read_buf[7300], read_buf_A[7300], read_buf_B[7300];
	unsigned char buf_sign = 0;
	int i, j, k, temp_total, loop_sign;
	//--------------------------------------------------------------------------------
	loop_sign = 1;


	while (loop_sign)  //find start position
	{
		ftStatus = pMyHndl->Read((BYTE*)(temp_read_buf), 128, &BytesRead);
		
		
		if (FT_SUCCESS(ftStatus))
		{
			for (k = 0; k < 128; k++)
			{
				read_buf_A[total_bytes++] = temp_read_buf[k];
				if (total_bytes == 7296)
				{
					total_bytes = 0;
					loop_sign = 0;
					break;
				}
			}
		}
	} //while(loop_sign)
///---------------------------------------------------------------------------------------
	temp_add = 0;

	while (1)
	{
		if (!running)
			continue;
		//-----------------------------------------------------------------------------       
		ftStatus = pMyHndl->Read((BYTE*)(temp_read_buf), 128, &BytesRead);
		if (FT_SUCCESS(ftStatus))
		{
			for (k = 0; k < 128; k++)
			{
				if (buf_sign == 0)
					read_buf_B[total_bytes++] = temp_read_buf[k];
				else
					read_buf_A[total_bytes++] = temp_read_buf[k];

				if (total_bytes == 7296)
				{
					total_bytes = 0;

					if (buf_sign == 0)
					{
						buf_sign = 1;
						loop_sign = 0;

						for (i = 0; i < 7295; i++)
						{
							if (read_buf_A[i] == 255 && read_buf_A[i + 1] == 254)
							{
								//cout << "total_read_buf " << temp_read_buf[k] << endl;
								for (j = i + 2; j < 7296; j++)
									total_read_buf[total_bytes++] = read_buf_A[j];
								for (j = 0; j < 7296; j++)
								{
									total_read_buf[total_bytes++] = read_buf_B[j];
									if (total_bytes == 7296)
									{
										total_bytes = 0;
										loop_sign = 1;
										break;
									}
								}
							}
							if (loop_sign)   break;
						}
					} //if(buf_sign==0)
					else if (buf_sign == 1)
					{
						buf_sign = 0;
						loop_sign = 0;
						for (i = 0; i < 7295; i++)
						{
							if (read_buf_B[i] == 255 && read_buf_B[i + 1] == 254)
							{
								for (j = i + 2; j < 7296; j++)
									total_read_buf[total_bytes++] = read_buf_B[j];
								for (j = 0; j < 7296; j++)
								{
									total_read_buf[total_bytes++] = read_buf_A[j];
									if (total_bytes == 7296)
									{
										total_bytes = 0;
										loop_sign = 1;
										break;
									}
								}
							}
							if (loop_sign)   break;
						}
					}   //else if(buf_sign==1)

					if (draw_data_sign == 0)
					{
						draw_data_sign = 1;
						
						for (i = 0; i < 7296; i += 2)
							ccd_vol_array[i / 2] = total_read_buf[i] * 256 + total_read_buf[i + 1];
						//cout << "total_read_buf " << total_read_buf[i + 1] << endl;
						ccd_vol_array[0] = ccd_vol_array[1] = ccd_vol_array[3647] = ccd_vol_array[2];
						//cout << "total_read_buf " << ccd_vol_array[1] << endl;
						CopyMemory((BYTE*)ccd_int, (BYTE*)ccd_vol_array, 3648 * sizeof(unsigned short));
						draw_data_sign = 2;
					}

					temp_add++;
					if (temp_add > 100)
						temp_add = 0;

				} //if(total_bytes==7296)
			} //for(k=0;k<128;k++)
		} //if(FT_SUCCESS(ftStatus))
	}  //while(1)
//-----------------------------------------


	return 0;
}




