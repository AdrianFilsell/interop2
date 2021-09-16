
#pragma once

#include <afxdialogex.h>
#include <string>

class authorizedlg : public CDialogEx
{
// Construction
public:
	authorizedlg(const std::wstring& sURL, CWnd* pParent = nullptr);	// standard constructor

	virtual INT_PTR DoModal( void ) override;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AUTHORIZEDLG_DIALOG };
#endif
	CString m_csCopy;
	CString m_csPaste;
	std::wstring m_sVerifier;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	void parsepaste( void );

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

protected:
	virtual BOOL OnInitDialog() override;
public:
	afx_msg void OnBnClickedCopybutton();
	afx_msg void OnBnClickedPastebutton();
	afx_msg void OnKillFocusPasteEdit();
};
