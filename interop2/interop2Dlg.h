
// interop2Dlg.h : header file
//

#pragma once

#include "uploadlistctrl.h"
#include "thread.h"
#include <map>
#include <set>

// Cinterop2Dlg dialog
class Cinterop2Dlg : public CDialogEx
{
// Construction
public:
	Cinterop2Dlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INTEROP2_DIALOG };
#endif
	uploadlistctrl m_ListCtrl;
	int m_nProcessType;
	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	std::vector<std::shared_ptr<dropboxupload>> m_vUploads;
	std::map<std::shared_ptr<uploadthread>,std::set<std::shared_ptr<dropboxupload>>> m_mThreads;

	// Generated message map functions
	virtual BOOL OnInitDialog()override;
	virtual void EndModalLoop(int nResult) override;
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedAppend();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg LRESULT OnUploadProgress(WPARAM,LPARAM);
	afx_msg LRESULT OnUploadDone(WPARAM,LPARAM);

	void append( const std::vector<std::wstring>& v );
	void enabledisable( void );
	void getitems( std::vector<int>& vItems ) const;
	void getuploads( std::vector<std::shared_ptr<dropboxupload>>& vUploads ) const;
	bool getupload( const dropboxupload *p,int& nUpload ) const;

	void start( const std::vector<std::shared_ptr<dropboxupload>>& vUploads, const std::wstring& sAccessToken, const uploadthreadrequest::type t, std::shared_ptr<const scheduler> spSched );
	void stop( const std::vector<std::shared_ptr<dropboxupload>>& vUploads, const bool bResetProgress );
};
