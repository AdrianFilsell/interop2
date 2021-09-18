#pragma once

#include <afxcmn.h>
#include <vector>
#include <memory>
#include <string>
#include <curl/curl.h>

#define WM_UPLOADPROGRESS ( WM_USER + 0x100 )
#define WM_UPLOADDONE ( WM_USER + 0x101 )

class uploadthread;

class globalcurl
{
public:
	globalcurl(){m_bValid = CURLE_OK == curl_global_init(CURL_GLOBAL_DEFAULT);}
	virtual ~globalcurl(){if(m_bValid)curl_global_cleanup();}
protected:
	bool m_bValid;
};

class curl
{
public:
	curl(){m_easy_handle=nullptr;}
	virtual ~curl(){if(get()){cleanup();}}
	
	bool init( void );
	void cleanup( void );

	CURL *get( void ) const { return m_easy_handle; }

	static size_t writefn(void *contents, size_t size, size_t nmemb, std::string *s);
	static size_t readfn(char *ptr, size_t size, size_t nmemb, void *userdata);
	static int xferfn(void *clientp, curl_off_t dltotal, curl_off_t dlnow,curl_off_t ultotal,curl_off_t ulnow);
protected:
	CURL *m_easy_handle;
};

class dropboxupload
{
public:
	dropboxupload(const std::wstring& s){m_pThread=nullptr;m_bCancelled=false;m_sPath=s;m_dProgress=0;}
	~dropboxupload(){}

	const std::wstring& getpath( void ) const { return m_sPath; }

	double getprogress( void ) const { return m_dProgress; }
	void setprogress( const double d );

	uploadthread *getthread( void ) const { return m_pThread; }
	void setthread( uploadthread *p ) { m_pThread=p; }
	
	bool getcancelled( void ) const { return m_bCancelled; }
	void setcancelled( const bool b ) { m_bCancelled=b; /* simple atomic op performed so no thread syncronisation objects required*/ }
protected:
	std::wstring m_sPath;
	double m_dProgress;
	bool m_bCancelled;
	uploadthread *m_pThread;
};

#error add your dropbox key/secret

class dropboxrequest
{
public:
	virtual bool call( void ) = 0;
protected:
	dropboxrequest(	std::shared_ptr<curl> spConn ):m_spConn(spConn){m_sAppKey="...";m_sAppSecret="...";}
	virtual ~dropboxrequest(){}

	std::string m_sAppKey;
	std::string m_sAppSecret;

	std::shared_ptr<curl> m_spConn;
};

class gettokendropboxrequest : public dropboxrequest
{
public:
	gettokendropboxrequest(	std::shared_ptr<curl> spConn, const std::wstring& sAuthorization ):dropboxrequest(spConn),m_sAuthorization(sAuthorization){}
	virtual ~gettokendropboxrequest(){}

	const std::string& getjsonresult( void ) const { return m_sJSONResult; }
	virtual bool call( void ) override;
protected:
	const std::wstring m_sAuthorization;
	std::string m_sJSONResult;
};

class uploadfiledropboxrequest : public dropboxrequest
{
public:
	uploadfiledropboxrequest( std::shared_ptr<curl> spConn, const std::wstring& sAccessToken, const std::wstring& sPath, dropboxupload *pUploadProgress, HWND hNotify ):dropboxrequest(spConn),m_sAccessToken(sAccessToken),m_sPath(sPath),m_pUploadProgress(pUploadProgress),m_hNotify(hNotify){}
	virtual ~uploadfiledropboxrequest(){}

	const std::string& getjsonresult( void ) const { return m_sJSONResult; }
	virtual bool call( void ) override;
protected:
	const std::wstring m_sAccessToken;
	std::string m_sJSONResult;
	std::wstring m_sPath;
	dropboxupload *m_pUploadProgress;
	HWND m_hNotify;
};
