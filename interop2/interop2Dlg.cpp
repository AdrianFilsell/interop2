
// interop2Dlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "interop2.h"
#include "interop2Dlg.h"
#include "afxdialogex.h"
#include "authorizedlg.h"
#include "json.h"
#include "utils.h"
#include <set>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cinterop2Dlg dialog



Cinterop2Dlg::Cinterop2Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INTEROP2_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_nProcessType = 0;
}

void Cinterop2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX,IDC_FILELIST,m_ListCtrl);
	DDX_CBIndex(pDX,IDC_PROCESSCOMBO,m_nProcessType);
}

BEGIN_MESSAGE_MAP(Cinterop2Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(IDCLOSE, &Cinterop2Dlg::OnBnClickedClose)
	ON_BN_CLICKED(IDC_APPEND, &Cinterop2Dlg::OnBnClickedAppend)
	ON_BN_CLICKED(IDC_START, &Cinterop2Dlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_STOP, &Cinterop2Dlg::OnBnClickedStop)
	ON_MESSAGE(WM_UPLOADPROGRESS, &Cinterop2Dlg::OnUploadProgress)
	ON_MESSAGE(WM_UPLOADDONE, &Cinterop2Dlg::OnUploadDone)
END_MESSAGE_MAP()


// Cinterop2Dlg message handlers

BOOL Cinterop2Dlg::OnInitDialog()
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
	enabledisable();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Cinterop2Dlg::EndModalLoop(int nResult)
{
	// just emptying the thread map will stop all async activity
	m_sThreads.clear();
	#ifdef _DEBUG
	if(m_vUploads.size())
	{
		auto i = m_vUploads.cbegin(), end = m_vUploads.cend();
		ASSERT(!(*i)->getthread());
	}
	#endif

	// call base class
	CDialogEx::EndModalLoop(nResult);
}

void Cinterop2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void Cinterop2Dlg::OnPaint()
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
HCURSOR Cinterop2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Cinterop2Dlg::OnBnClickedClose()
{
	EndDialog(IDOK);
}

void Cinterop2Dlg::OnBnClickedAppend()
{
	// TODO: Add your control notification handler code here
	CFileDialog cf( TRUE, NULL, NULL, OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT );
	if( cf.DoModal() != IDOK )
		return;
	
	std::vector<std::wstring> v;
	for( POSITION pos = cf.GetStartPosition(); pos; )
	{
		CString csPath = cf.GetNextPathName( pos );
		v.push_back( std::wstring(csPath) );
	}

	append( v );

	enabledisable();
}

void Cinterop2Dlg::OnBnClickedStart()
{
	UpdateData();
	
	// dropbox authorize
	// https://www.dropbox.com/oauth2/authorize?client_id=1u3nzy29vd5hcy2&token_access_type=offline&response_type=code
	authorizedlg dlg(L"https://www.dropbox.com/oauth2/authorize?client_id=1u3nzy29vd5hcy2&token_access_type=offline&response_type=code");
	if( dlg.DoModal() != IDOK )
		return;

	// dropbox token
	gettokendropboxrequest requesttoken(nullptr,dlg.m_sVerifier);
	if( !requesttoken.call() )
		return;

	// get the id
	std::shared_ptr<json> spJSON{ new json };
	if( !spJSON->read(requesttoken.getjsonresult()) )
		return;
	const std::wstring sAccessToken = utils::string::s2ws( spJSON->getdata("access_token") );
	if( sAccessToken.size() == 0 )
		return;

	// upload
	uploadthreadrequest::type t;
	switch( m_nProcessType )
	{
		case 0:t=uploadthreadrequest::t_serial;break;
		case 1:t=uploadthreadrequest::t_serial_tbb;break;
		case 2:t=uploadthreadrequest::t_parallel;break;
		default:ASSERT(false);break;
	}
	std::vector<std::shared_ptr<dropboxupload>> vUploads;
	getuploads(vUploads);
	start( vUploads, sAccessToken, t, theApp.getscheduler() );
}

void Cinterop2Dlg::OnBnClickedStop()
{
	std::vector<std::shared_ptr<dropboxupload>> vUploads;
	getuploads(vUploads);
	stop(vUploads,true);
}

LRESULT Cinterop2Dlg::OnUploadProgress(WPARAM wParam,LPARAM lParam)
{
	int nUpload = -1;
	dropboxupload *pProgress = reinterpret_cast<dropboxupload*>(lParam);
	if( pProgress && getupload( pProgress, nUpload ) )
	{
		m_ListCtrl.invalidateprogress( nUpload );
		m_ListCtrl.UpdateWindow();
	}

	return 0;
}

LRESULT Cinterop2Dlg::OnUploadDone(WPARAM wParam,LPARAM lParam)
{
	uploadthread *pThread = reinterpret_cast<uploadthread*>(lParam);
	if( pThread )
	{
		std::shared_ptr<uploadthread> sp(pThread, [](uploadthread*){}); // we dont want this smart ptr to own the thread
		auto m = m_sThreads.find( sp );
		if( m != m_sThreads.cend() )
		{
			pThread->stop(INFINITE);

			m_sThreads.erase(m);
		}
	}

	return 0;
}

void Cinterop2Dlg::enabledisable( void )
{
	GetDlgItem(IDC_APPEND)->EnableWindow();
	GetDlgItem(IDC_START)->EnableWindow(m_ListCtrl.GetItemCount()>0);
	GetDlgItem(IDC_STOP)->EnableWindow(m_ListCtrl.GetItemCount()>0);
}

void Cinterop2Dlg::append( const std::vector<std::wstring>& v )
{
	std::set<std::wstring> s;
	{
		auto i = m_vUploads.cbegin(), end = m_vUploads.cend();
		for( ; i != end ; ++i )
			s.insert( utils::string::makelower<std::wstring>(std::wstring((*i)->getpath())) );
	}
	
	auto nPreSize = m_vUploads.size();
	auto i = v.cbegin(), end = v.cend();
	for( ; i != end ; ++i )
		if( s.find( utils::string::makelower<std::wstring>(std::wstring(*i)) ) == s.cend() )
		{
			std::shared_ptr<dropboxupload> sp{ new dropboxupload(*i) };
			m_vUploads.push_back( sp );
		}

	std::vector<std::shared_ptr<const dropboxupload>> vUploads;
	vUploads.insert( vUploads.cend(), m_vUploads.cbegin() + nPreSize, m_vUploads.cend() );
	m_ListCtrl.append(vUploads);
}

void Cinterop2Dlg::getitems( std::vector<int>& vItems ) const
{
	if(m_ListCtrl.GetSelectedCount())
	{
		vItems.resize(m_ListCtrl.GetSelectedCount());
		auto i = vItems.begin();
		for( POSITION pos = m_ListCtrl.GetFirstSelectedItemPosition() ; pos ; ++i )
			(*i) = m_ListCtrl.GetNextSelectedItem( pos );
	}

	if( vItems.size() == 0 )
	{
		vItems.resize( m_ListCtrl.GetItemCount() );
		auto i = vItems.begin();
		auto end = vItems.cend();
		for( ; i != end ; ++i )
			(*i) = static_cast<int>( std::distance(vItems.begin(),i) );
	}
}

void Cinterop2Dlg::getuploads( std::vector<std::shared_ptr<dropboxupload>>& vUploads ) const
{
	std::vector<int> vItems;
	getitems( vItems );
	vUploads.resize(vItems.size());
	auto i = vUploads.begin();
	auto end = vUploads.cend();
	auto j = vItems.cbegin();
	for( ; i != end ; ++i, ++j )
		(*i) = m_vUploads[*j];
}

bool Cinterop2Dlg::getupload( const dropboxupload *p,int& nUpload ) const
{
	auto i = m_vUploads.cbegin(), end = m_vUploads.cend();
	for( ; i != end ; ++i )
		if( (*i).get() == p )
		{
			nUpload = static_cast<int>( std::distance(m_vUploads.cbegin(),i) );
			return true;
		}
	return false;
}

void Cinterop2Dlg::start( const std::vector<std::shared_ptr<dropboxupload>>& vUploads, const std::wstring& sAccessToken, const uploadthreadrequest::type t, std::shared_ptr<const scheduler> spSched )
{
	stop(vUploads,true);

	switch( t )
	{
		case uploadthreadrequest::t_parallel:
		{
			auto i = vUploads.cbegin(), end = vUploads.cend();
			for( ; i != end ; ++i )
			{
				std::shared_ptr<uploadthread> spT{new uploadthread };
				auto s = m_sThreads.insert( spT );
				
				std::shared_ptr<uploadthreadrequest> sp{ new uploadthreadrequest({*i},sAccessToken,t) };
				if( !spT->start(sp,spSched,GetSafeHwnd()) )
					m_sThreads.erase( s.first );
			}
		}
		break;
		case uploadthreadrequest::t_serial:
		case uploadthreadrequest::t_serial_tbb:
		{
			std::shared_ptr<uploadthread> spT{new uploadthread };
			auto s = m_sThreads.insert( spT );
			
			std::shared_ptr<uploadthreadrequest> sp{ new uploadthreadrequest(vUploads,sAccessToken,t) };
			if( !spT->start(sp,spSched,GetSafeHwnd()) )
				m_sThreads.erase( s.first );
		}
		break;
		default:ASSERT(false);
	}
}

void Cinterop2Dlg::stop( const std::vector<std::shared_ptr<dropboxupload>>& vUploads, const bool bResetProgress )
{
	std::set<uploadthread*> s;
	{
		auto i = vUploads.cbegin(), end = vUploads.cend();
		for( ; i != end ; ++i )
			if((*i)->getthread())
				s.insert((*i)->getthread());
	}

	auto i = s.cbegin(), end = s.cend();
	for( ; i != end ; ++i )
	{
		auto s = m_sThreads.find(std::shared_ptr<uploadthread>(*i, [](uploadthread*){})); // we dont want this smart ptr to own the thread
		if(s == m_sThreads.cend())
		{
			ASSERT(false);
			continue;
		}

		const std::vector<std::shared_ptr<dropboxupload>> v = (*s)->getrequest()->getuploads();
		(*i)->stop(INFINITE); // there may be many uploads associated with this thread but lets stop them ALL
		
		if( bResetProgress )
			uploadthread::setprogress(v,0);
				
		m_sThreads.erase(s);
	}

	if( bResetProgress )
		m_ListCtrl.Invalidate();
}
