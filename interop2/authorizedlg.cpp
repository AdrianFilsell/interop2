
#include "pch.h"
#include "authorizedlg.h"
#include "resource.h"
#include "utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

authorizedlg::authorizedlg(const std::wstring& sURL, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_AUTHORIZEDLG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_csCopy = sURL.c_str();
}

void authorizedlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX,IDC_COPYEDIT,m_csCopy);
	DDX_Text(pDX,IDC_PASTEEDIT,m_csPaste);
}

BEGIN_MESSAGE_MAP(authorizedlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_COPYBUTTON, &authorizedlg::OnBnClickedCopybutton)
	ON_BN_CLICKED(IDC_PASTEBUTTON, &authorizedlg::OnBnClickedPastebutton)
	ON_EN_KILLFOCUS(IDC_PASTEEDIT, &authorizedlg::OnKillFocusPasteEdit)
END_MESSAGE_MAP()

INT_PTR authorizedlg::DoModal( void )
{
	const INT_PTR n = CDialogEx::DoModal();
	if( n == IDOK )
		parsepaste();
	return n;
}

BOOL authorizedlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void authorizedlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
//		CAboutDlg dlgAbout;
//		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void authorizedlg::OnPaint()
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
HCURSOR authorizedlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void authorizedlg::OnBnClickedCopybutton()
{
	// TODO: Add your control notification handler code here
	UpdateData();

	if (!OpenClipboard()) 
		return; 
	EmptyClipboard(); 

	unsigned int strSize = m_csCopy.GetLength();
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (strSize+1) * sizeof(char));
	if (hglbCopy) 
	{
		char *lptstrCopy = (char*)GlobalLock(hglbCopy); 
		memcpy(lptstrCopy, CStringA(m_csCopy).GetBuffer(), strSize * sizeof(char));
		lptstrCopy[strSize] = '\0';
		GlobalUnlock(hglbCopy); 

		SetClipboardData(CF_TEXT,hglbCopy);
	}

	CloseClipboard();
}

void authorizedlg::OnBnClickedPastebutton()
{
	// TODO: Add your control notification handler code here
	m_csPaste.Empty();
	
	if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard())
	{
		HGLOBAL hglb = GetClipboardData(CF_TEXT); 
		if (hglb) 
		{ 
			char *lptstr = (char*)GlobalLock(hglb); 
			if (lptstr) 
				m_csPaste = lptstr;
			GlobalUnlock(hglb);
		}
		CloseClipboard(); 
	}

	UpdateData(false);
}

void authorizedlg::OnKillFocusPasteEdit()
{
	UpdateData();
}

void authorizedlg::parsepaste()
{
	m_sVerifier = m_csPaste;
}
