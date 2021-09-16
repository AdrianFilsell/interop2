#pragma once

#include <afxcmn.h>
#include "upload.h"

class theme
{
public:
	theme(){m_hTheme=NULL;}
	~theme(){close();}
	
	bool open( const HWND h, LPCWSTR lp )
	{
		close();
		m_hTheme = ::OpenThemeData( h, lp );
		return m_hTheme != NULL;
	}
	void close( void )
	{
		if( m_hTheme )
		{
			::CloseThemeData( m_hTheme );
			m_hTheme = NULL;
		}
	}
	bool render( HWND h, HDC hDC, const int nPartID, const int nStateID, const CRect& rcPaint, const CRect& rcClip ) const
	{
		if( !m_hTheme )
			return false;
		if( IsThemeBackgroundPartiallyTransparent( m_hTheme, nPartID, nStateID ) )
			DrawThemeParentBackground( h, hDC, rcPaint );
		DrawThemeBackground( m_hTheme, hDC, nPartID, nStateID, rcPaint, rcClip );
		return true;
	}
protected:
	HTHEME m_hTheme;
};

class progresstheme
{
public:
	progresstheme(){}
	~progresstheme(){}

	bool open( const HWND h)
	{
		m_spTheme = std::shared_ptr<theme>( new theme );
		if( m_spTheme->open( h, L"PROGRESS" ) )
			return true;
		m_spTheme = nullptr;
		return false;
	}
	bool render( HWND h, HDC hDC, const double dPos, const CRect& rcPaint, const CRect& rcClip ) const
	{
		if( !m_spTheme )
			return false;
		const double d = dPos < 0 ? 0 : ( dPos > 1 ? 1 : dPos );
		m_spTheme->render( h, hDC, PP_TRANSPARENTBAR, PBBS_PARTIAL, rcPaint, rcClip );
		const CRect rcFill( rcPaint.left, rcPaint.top, rcPaint.left + int( 0.5 + ( rcPaint.Width() * d ) ), rcPaint.bottom );
		m_spTheme->render( h, hDC, PP_FILL, PBFS_NORMAL, rcFill, rcClip );
		if( false )
			m_spTheme->render( h, hDC, PP_PULSEOVERLAY, 0, rcPaint, rcClip );
		return true;
	}
protected:
	std::shared_ptr<theme> m_spTheme;
};

class uploadlistctrl : public CListCtrl
{
public:
	uploadlistctrl(){}
	virtual ~uploadlistctrl(){}

	virtual void PreSubclassWindow() override;

	void append( const std::vector<std::shared_ptr<const dropboxupload>>& v );

	bool invalidateprogress( const int nUpload );
protected:
	//{{AFX_VIRTUAL(uploadlistctrl)
	virtual void DrawItem(LPDRAWITEMSTRUCT);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	//{{AFX_MSG(uploadlistctrl)
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	afx_msg int OnCreate(LPCREATESTRUCT lpCIS);
	afx_msg void OnSetFocus(CWnd *p);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	progresstheme m_ProgressTheme;

	void setuphdr( void );
	void getitemcolumns( const CRect& rcItemRect, std::vector<CRect>& vColumns ) const;
	enum columntype { ct_path, ct_progress };
	columntype gettypefromcolumn( const int nColumn ) const { ASSERT(nColumn>=ct_path && nColumn<=ct_progress); return columntype(nColumn); }
	int getcolumnfromtype( const columntype t ) const;
};
