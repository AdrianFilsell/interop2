
#include "pch.h"
#include "uploadlistctrl.h"
#include "interop2.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(uploadlistctrl, CListCtrl)
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

BOOL uploadlistctrl::PreCreateWindow(CREATESTRUCT& cs)
{
	// mutate style, report view, owner draw, ...
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= LVS_REPORT|LVS_OWNERDRAWFIXED;
	cs.dwExStyle |= LVS_EX_FULLROWSELECT|LVS_EX_INFOTIP;

	// call base class
	return CListCtrl::PreCreateWindow(cs);
}

int uploadlistctrl::OnCreate(LPCREATESTRUCT lpCIS)
{
	// call base class
	const int n = CListCtrl::OnCreate(lpCIS);

	// setup hdr
	setuphdr();

	return n;
}

void uploadlistctrl::PreSubclassWindow()
{
	// call base class
	CListCtrl::PreSubclassWindow();

	// setup hdr
	setuphdr();
}

void uploadlistctrl::OnSetFocus(CWnd *p)
{
	InvalidateRect(nullptr);
}

void uploadlistctrl::setuphdr( void )
{
	if( GetHeaderCtrl()->GetItemCount() > 0 )
		return;

	m_ProgressTheme.open( GetSafeHwnd() );

	CString csPath;csPath.LoadString(IDS_PATH);
	CString csProgress;csProgress.LoadString(IDS_PROGRESS);
	CRect rc;
	GetWindowRect(rc);
	::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rc,2);
	rc.DeflateRect(2,2);
	InsertColumn(0,csPath,LVCFMT_LEFT,rc.Width()-200);
	InsertColumn(1,csProgress,LVCFMT_CENTER,200);
}

void uploadlistctrl::getitemcolumns( const CRect& rcItemRect, std::vector<CRect>& vColumns ) const
{
	// Determine if there is anything to do
	CHeaderCtrl *pHdrCtrl = GetHeaderCtrl();
	vColumns.resize( pHdrCtrl->GetItemCount() );
	auto i = vColumns.begin();
	auto end = vColumns.cend();
	for( ; i != end ; ++i )
	{
		const int nColumn = static_cast<int>(std::distance(vColumns.begin(),i));

		CRect rcRect;
		pHdrCtrl->GetItemRect( nColumn, rcRect );

		rcRect.top = rcItemRect.top;
		rcRect.bottom = rcItemRect.bottom;
		rcRect.OffsetRect( rcItemRect.left, 0 );

		(*i) = rcRect;
	}
}

void uploadlistctrl::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	// ownerdrawfixed so will not get called, if ownerdrawvvariable will be called for every item
	ASSERT( false );

	// assume the currently selected font is the same used when rendering
	TEXTMETRIC tm; 
	HDC hDC = ::GetDC( m_hWnd );
	CFont* pFont = GetFont();
	HFONT hFontOld = static_cast<HFONT>( SelectObject( hDC, pFont->GetSafeHandle() ) );
	GetTextMetrics( hDC, &tm );
	lpMIS->itemHeight = tm.tmHeight + tm.tmExternalLeading + 1;
	SelectObject( hDC, hFontOld );
	::ReleaseDC( m_hWnd, hDC );
}

void uploadlistctrl::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	// get the columns
	std::vector<CRect> vColumns;
	const CRect rcItemRect( lpDIS->rcItem );
	getitemcolumns( rcItemRect, vColumns );
	
	// render background
	CDC *pDC = CDC::FromHandle( lpDIS->hDC );
	CWnd *pFocusWnd = GetFocus();
	const bool bCtrlFocus = ( pFocusWnd && ( pFocusWnd->GetSafeHwnd() == m_hWnd ) );
	const bool bSelected = ( lpDIS->itemState & ODS_SELECTED );
	const COLORREF crBkGnd = ::GetSysColor( COLOR_WINDOW );
	pDC->FillSolidRect( &rcItemRect, ::GetSysColor( COLOR_WINDOW ) );

	// selection
	const CRect rcText = rcItemRect;
	if( bSelected )
	{
		if( bCtrlFocus )
			pDC->FillSolidRect( &rcItemRect, ::GetSysColor( COLOR_HIGHLIGHT ) );
		else
			pDC->FillSolidRect( &rcItemRect, ::GetSysColor( COLOR_BTNFACE ) );
	}

	// upload
	const dropboxupload *pUpload = reinterpret_cast<const dropboxupload*>(GetItemData(lpDIS->itemID));
		
	// text
	CFont *pFont = GetFont();
	CFont *pOldFont = pDC->SelectObject( pFont );
	int nOldMode = pDC->SetBkMode( TRANSPARENT );
	COLORREF crOldTextColour = pDC->SetTextColor( ::GetSysColor( ( bSelected && bCtrlFocus ) ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT ) );
	auto i = vColumns.cbegin(), end = vColumns.cend();
	for( ; i != end ; ++i )
	{
		const int nColumn = static_cast<int>(std::distance(vColumns.cbegin(),i));
		
		LVCOLUMN lvc;
		lvc.mask = LVCF_FMT;
		GetColumn( nColumn, &lvc );

		switch( gettypefromcolumn( nColumn ) )
		{
			case ct_path:
			{
				const CString csText = GetItemText( lpDIS->itemID, nColumn );
				DWORD dwFlags = DT_VCENTER|DT_SINGLELINE|DT_PATH_ELLIPSIS|DT_END_ELLIPSIS;

				if(lvc.fmt & LVCFMT_CENTER)
					dwFlags |= DT_CENTER;
				else
				if(lvc.fmt & LVCFMT_RIGHT)
					dwFlags |= DT_RIGHT;
				else
					dwFlags |= DT_LEFT;

				CRect rcText = *i;
				pDC->DrawText( csText, &rcText, dwFlags );
			}
			break;
			case ct_progress:
			{
				CRect rcProgress = *i;
				rcProgress.DeflateRect( 2, 2 );
				m_ProgressTheme.render( GetSafeHwnd(), pDC->GetSafeHdc(), pUpload ? pUpload->getprogress() : 0, rcProgress, rcProgress );
			}
			break;
			default:ASSERT(false);break;
		}
	}
	pDC->SetTextColor( crOldTextColour );
	pDC->SetBkMode( nOldMode );
	pDC->SelectObject( pOldFont );
	
    // focus
    if( lpDIS->itemState & ODS_FOCUS )
		pDC->DrawFocusRect( &rcItemRect );
}

void uploadlistctrl::append( const std::vector<std::shared_ptr<const dropboxupload>>& v )
{
	auto nPreSize = GetItemCount();
	auto i = v.cbegin(), end = v.cend();
	for( ; i != end ; ++i )
	{
		const int nVectorItem = nPreSize + static_cast<int>(std::distance( v.cbegin(), i ));
		
		LVITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = nVectorItem;
		lvi.iSubItem = 0;
		lvi.pszText = const_cast<wchar_t*>( (*i)->getpath().c_str() );
		const int nItem = InsertItem( &lvi );
		if( false )
		{
			LVITEM lvi;
			lvi.mask = LVIF_TEXT;
			std::wstring sItem = L"sub item";
			lvi.iItem = nItem;
			lvi.iSubItem = 1;
			lvi.pszText = const_cast<wchar_t*>( sItem.c_str() );
			const int nItem = SetItem( &lvi );
		}
		SetItemData( nItem, reinterpret_cast<DWORD_PTR>((*i).get()) );
	}
}

bool uploadlistctrl::invalidateprogress( const int nUpload )
{
	if( nUpload < 0 || nUpload >= GetItemCount() )
		return false;

	if( ::GetCurrentThreadId() == theApp.getthreadid() )
	{
		CRect rcItem;
		ASSERT( ::GetCurrentThreadId() == theApp.getthreadid() );
		if( !GetItemRect( nUpload, &rcItem, LVIR_BOUNDS ) ) // not thread safe uses sendmessage
			return false;

		std::vector<CRect> vColumns;
		const CRect rcItemRect( rcItem );
		getitemcolumns( rcItemRect, vColumns );

		InvalidateRect( vColumns[getcolumnfromtype( ct_progress )], false );
	}
	else
	{
		ASSERT(false);
		InvalidateRect( nullptr, false );
	}
	
	return true;
}

int uploadlistctrl::getcolumnfromtype( const columntype t ) const
{
	switch( t )
	{
		case ct_path:return 0;
		case ct_progress:return 1;
		default:ASSERT( false );return 0;
	}
}
