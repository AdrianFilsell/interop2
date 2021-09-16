
#include "pch.h"
#include "thread.h"
#include "upload.h"
#include "interop2.h"
#include <openssl/crypto.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(uploadthread, CWinThread)

BEGIN_MESSAGE_MAP(uploadthread, CWinThread)
END_MESSAGE_MAP()

uploadthread::uploadthread()
{
	m_hNotify=NULL;
	m_bAutoDelete = false;
}

uploadthread::~uploadthread()
{
	stop( INFINITE );
}

bool uploadthread::start( std::shared_ptr<const uploadthreadrequest> sp,std::shared_ptr<const scheduler> spSched, HWND hNotify )
{
	stop( INFINITE );

	if( !sp || !spSched || !hNotify )
		return false;
	
	m_sp = sp;
	m_spSched = spSched;
	m_hNotify = hNotify;

	CreateThread();
	SetThreadPriority( THREAD_PRIORITY_BELOW_NORMAL );

	return true;
}

bool uploadthread::stop( const DWORD dwTimeOut )
{
	ASSERT( dwTimeOut == INFINITE );
	
	if( m_sp )
		m_sp->setcancelled(true);
	
	const DWORD dw = ::WaitForSingleObject(m_hThread, dwTimeOut);
	m_hThread = NULL;

	if( m_sp )
		m_sp->setcancelled(false);

	m_sp = nullptr;
	m_spSched = nullptr;

	return true;
}

BOOL uploadthread::InitInstance( void )
{
	// http
	switch( m_sp->gettype() )
	{
		case uploadthreadrequest::t_parallel:
		case uploadthreadrequest::t_serial:
		{
			ASSERT( m_sp->getuploads().size() == 1 || m_sp->gettype() == uploadthreadrequest::t_serial );
			auto i = m_sp->getuploads().begin();
			auto end = m_sp->getuploads().cend();
			for( ; i != end ; ++i )
			{
				if( (*i)->getcancelled() )
					continue;
				
				uploadfiledropboxrequest fileupload(nullptr, m_sp->getaccesstoken(), (*i)->getpath(), (*i).get(), m_hNotify);
				fileupload.call();
			}
		}
		break;
		case uploadthreadrequest::t_serial_tbb:
		{
			class serial_tbb_task
			{
			public:
				serial_tbb_task( const DWORD dwNonCleanup, const std::vector<std::shared_ptr<dropboxupload>>& vUploads, const std::wstring& sAccessToken, HWND hNotify ):m_dwNonCleanup(dwNonCleanup),m_hNotify(hNotify),m_vUploads(vUploads),m_sAccessToken(sAccessToken){}
				void operator()( const int nFrom, const int nTo, const taskinfo *pInfo = nullptr ) const
				{
					int n = nFrom;
					for( ; n <= nTo ; ++n )
					{
						if( m_vUploads[n]->getcancelled() )
							continue;
				
						uploadfiledropboxrequest fileupload(nullptr, m_sAccessToken, m_vUploads[n]->getpath(), m_vUploads[n].get(), m_hNotify);
						fileupload.call();
					}
					
					// should not need this... but required for reasons discussed here https://github.com/curl/curl/issues/5282
					if( GetCurrentThreadId() != m_dwNonCleanup )
						OPENSSL_thread_stop();
				}
			protected:
				const DWORD m_dwNonCleanup;
				HWND m_hNotify;
				const std::vector<std::shared_ptr<dropboxupload>>& m_vUploads;
				const std::wstring& m_sAccessToken;
			};

			const int nItems = static_cast<int>( m_sp->getuploads().size() );
			m_spSched->parallel_for( 0, nItems, m_spSched->getcores(), serial_tbb_task( GetCurrentThreadId(), m_sp->getuploads(), m_sp->getaccesstoken(), m_hNotify ), nullptr );			
		}
		break;
		default:ASSERT(false);
	}
	
	// should not need this... but required for reasons discussed here https://github.com/curl/curl/issues/5282
	OPENSSL_thread_stop();

	::PostMessage(m_hNotify,WM_UPLOADDONE,0,reinterpret_cast<LPARAM>(this));

	// don't enter run loop
	return FALSE;
}

void uploadthreadrequest::setcancelled( const bool b ) const
{
	// immutable vector with simple atomic op performed on ref counted objects so no thread syncronisation objects required
	auto i = m_vUploads.begin();
	auto end = m_vUploads.cend();
	for( ; i != end ; ++i )
		(*i)->setcancelled( b );
}
