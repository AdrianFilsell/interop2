
#include "pch.h"
#include "upload.h"
#include "utils.h"
#include "interop2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool curl::init( void )
{
	if(get())
		return true;
	CURL *easy_handle = curl_easy_init();
	if( !easy_handle )
		return false;
	m_easy_handle = easy_handle;
	return true;
}

void curl::cleanup( void )
{
	if(!get())
		return;
	curl_easy_cleanup( m_easy_handle );
	m_easy_handle = nullptr;
}

int curl::xferfn(void *clientp, curl_off_t dltotal, curl_off_t dlnow,curl_off_t ultotal,curl_off_t ulnow)
{
	std::pair<dropboxupload*,HWND> *pProgress = reinterpret_cast<std::pair<dropboxupload*,HWND>*>(clientp);
	
	if( pProgress && pProgress->first )
	{
		const double dProgress = ultotal ? ulnow/static_cast<double>(ultotal) : 0;
		pProgress->first->setprogress( dProgress );

		ASSERT(pProgress->second && ::GetCurrentThreadId() != theApp.getthreadid() );
		if(pProgress->second)
			::PostMessage(pProgress->second,WM_UPLOADPROGRESS,0,reinterpret_cast<LPARAM>(pProgress->first));

		if( pProgress->first->getcancelled() )
			return 1;
		
		return CURL_PROGRESSFUNC_CONTINUE;
	}
	
	return CURL_PROGRESSFUNC_CONTINUE;
}

size_t curl::readfn(char *ptr, size_t size, size_t nmemb, void *clientp)
{
	FILE *readhere = reinterpret_cast<FILE*>(clientp);
 
	const size_t retcode = fread(ptr, size, nmemb, readhere);
	return retcode;
}

size_t curl::writefn(void *contents, size_t size, size_t nmemb, std::string *s)
{
	ASSERT( size == 1 );
	const size_t nAppend = size * nmemb;
	const size_t nNewSize = s->size() + nAppend;

	s->append(reinterpret_cast<char*>(contents), nAppend);
	ASSERT( s->size() == nNewSize );

	return nAppend;
}

void dropboxupload::setprogress( const double d )
{
	// simple atomic op performed so no thread syncronisation objects required
	m_dProgress = d;
}

bool gettokendropboxrequest::call( void )
{
	// curl https://api.dropboxapi.com/oauth2/token -d code=<AUTHORIZATION_CODE> -d grant_type=authorization_code -u <APP_KEY>:<APP_SECRET>
	
	// connection
	std::shared_ptr<curl> spCurl = m_spConn ? m_spConn : std::shared_ptr<curl>( new curl );
	if( !spCurl->init() )
		return false;
	
	// options
	curl_easy_reset(spCurl->get()); // dont want 'sticky' options as we are only making one transfer not multiple and who knows what options are currently set

	CURLcode res;

	res = curl_easy_setopt(spCurl->get(), CURLOPT_URL, "https://api.dropboxapi.com/oauth2/token");
	if( res != CURLE_OK )
		return false;

	const std::string sPost = utils::string::szfmt<std::string>("code=%s&grant_type=authorization_code", m_sAuthorization.c_str());
	res = curl_easy_setopt(spCurl->get(), CURLOPT_POSTFIELDS, sPost.c_str());
	if( res != CURLE_OK )
		return false;

	const bool bVerifyPeer = true;
	if( bVerifyPeer )
	res = bVerifyPeer ? curl_easy_setopt(spCurl->get(), CURLOPT_CAINFO, ( utils::string::getpwd() + "curl-ca-bundle.crt" ).c_str()) :
						curl_easy_setopt(spCurl->get(), CURLOPT_SSL_VERIFYPEER, 0);
	if( res != CURLE_OK )
		return false;
	
	res = curl_easy_setopt(spCurl->get(), CURLOPT_USERNAME, m_sAppKey.c_str());
	if( res != CURLE_OK )
		return false;
	res = curl_easy_setopt(spCurl->get(), CURLOPT_PASSWORD, m_sAppSecret.c_str());
	if( res != CURLE_OK )
		return false;

	std::string s;
	res = curl_easy_setopt(spCurl->get(), CURLOPT_WRITEFUNCTION, curl::writefn);
	if( res != CURLE_OK )
		return false;
    res = curl_easy_setopt(spCurl->get(), CURLOPT_WRITEDATA, &s);
	if( res != CURLE_OK )
		return false;

	// blocking transfer
	res = curl_easy_perform( spCurl->get() );
	if( res != CURLE_OK )
		return false;
	
	// json result
	m_sJSONResult = s;

	return true;
}

bool uploadfiledropboxrequest::call( void )
{
	/*
		curl -X POST https://content.dropboxapi.com/2/files/upload \
		--header "Authorization: Bearer " \
		--header "Dropbox-API-Arg: {\"path\": \"/Homework/math/Matrices.txt\",\"mode\": \"add\",\"autorename\": true,\"mute\": false,\"strict_conflict\": false}" \
		--header "Content-Type: application/octet-stream" \
		--data-binary @local_file.txt
	*/
	
	// connection
	std::shared_ptr<curl> spCurl = m_spConn ? m_spConn : std::shared_ptr<curl>( new curl );
	if( !spCurl->init() )
		return false;
	
	FILE *fd;
	if( _wfopen_s(&fd,m_sPath.c_str(), L"rb") )
		return false;
	struct stat file_info;
	if(fstat(_fileno(fd), &file_info) != 0)
	{
		fclose(fd);
		return false;
	}
	
	// options
	curl_easy_reset(spCurl->get()); // dont want 'sticky' options as we are only making one transfer not multiple and who knows what options are currently set
	
	CURLcode res;

	const bool bVerifyPeer = true;
	if( bVerifyPeer )
	res = bVerifyPeer ? curl_easy_setopt(spCurl->get(), CURLOPT_CAINFO, ( utils::string::getpwd() + "curl-ca-bundle.crt" ).c_str()) :
						curl_easy_setopt(spCurl->get(), CURLOPT_SSL_VERIFYPEER, 0);
	if( res != CURLE_OK )
	{
		fclose(fd);
		return false;
	}

	res = curl_easy_setopt(spCurl->get(), CURLOPT_URL, "https://content.dropboxapi.com/2/files/upload");
	if( res != CURLE_OK )
	{
		fclose(fd);
		return false;
	}

	struct curl_slist *slist=nullptr;
	struct curl_slist *temp=nullptr;
	{
		const std::string sHdr = utils::string::szfmt<std::string>("Authorization: Bearer %s",m_sAccessToken.c_str());
		temp = curl_slist_append(slist, sHdr.c_str());
	}
	slist = temp?temp:slist;
	{
		wchar_t fname[_MAX_FNAME];
		wchar_t ext[_MAX_EXT];
		_wsplitpath_s( m_sPath.c_str(),nullptr,0,nullptr,0,fname,_MAX_FNAME,ext,_MAX_EXT);
		const std::wstring sDropboxPath=utils::string::szfmt<std::wstring>(L"/%s.%s",fname,ext);
		const std::string sHdr = utils::string::szfmt<std::string>("Dropbox-API-Arg: {\"path\": \"%s\",\"mode\": \"add\",\"autorename\": true,\"mute\": false,\"strict_conflict\": false}",sDropboxPath.c_str());
		temp = curl_slist_append(slist, sHdr.c_str());
	}
	slist = temp?temp:slist;
	temp = temp?curl_slist_append(slist, "Content-Type: application/octet-stream"):temp;
	slist = temp?temp:slist;
	if( !temp )
	{
		fclose(fd);
		curl_slist_free_all(slist);
		return false;
	}
	
	std::string s;
	std::pair<dropboxupload*,HWND> progress = std::make_pair(m_pUploadProgress,m_hNotify);
	if( ( ( res = curl_easy_setopt(spCurl->get(), CURLOPT_UPLOAD, 1L) ) != CURLE_OK ) ||
		( ( res = curl_easy_setopt(spCurl->get(), CURLOPT_READFUNCTION, curl::readfn) ) != CURLE_OK ) ||
		( ( res = curl_easy_setopt(spCurl->get(), CURLOPT_READDATA, fd) ) != CURLE_OK ) ||
		( ( res = curl_easy_setopt(spCurl->get(), CURLOPT_WRITEFUNCTION, curl::writefn) ) != CURLE_OK ) ||
		( ( res = curl_easy_setopt(spCurl->get(), CURLOPT_WRITEDATA, &s) ) != CURLE_OK ) ||
		( ( res = curl_easy_setopt(spCurl->get(), CURLOPT_HTTPHEADER, slist) ) != CURLE_OK ) ||
		( ( res = curl_easy_setopt(spCurl->get(), CURLOPT_XFERINFOFUNCTION, curl::xferfn) ) != CURLE_OK ) ||
		( ( res = curl_easy_setopt(spCurl->get(), CURLOPT_XFERINFODATA, &progress) ) != CURLE_OK ) ||
		( ( res = curl_easy_setopt(spCurl->get(), CURLOPT_NOPROGRESS, 0L) ) != CURLE_OK ) ||
		( ( res = curl_easy_setopt(spCurl->get(), CURLOPT_INFILESIZE_LARGE, static_cast<curl_off_t>(file_info.st_size)) ) != CURLE_OK ) ||
		( ( res = curl_easy_setopt(spCurl->get(), CURLOPT_CUSTOMREQUEST, "POST") ) != CURLE_OK ) )
	{
		fclose(fd);
		curl_slist_free_all(slist);
		return false;
	}

	// blocking transfer
	res = curl_easy_perform( spCurl->get() );
	fclose(fd);
	curl_slist_free_all(slist);
	spCurl = nullptr;
	if( m_pUploadProgress )
	{
		m_pUploadProgress->setprogress( 1 );
		if(m_hNotify)
			::PostMessage(m_hNotify,WM_UPLOADPROGRESS,0,reinterpret_cast<LPARAM>(m_pUploadProgress));
	}
	if( res != CURLE_OK )
		return false;
	
	// json result
	m_sJSONResult = s;

	return true;
}
