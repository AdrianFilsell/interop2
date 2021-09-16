#pragma once

#pragma warning( push )
#pragma warning( disable: 4297 )
#pragma warning( disable: 4334 )
#define TBB_USE_CAPTURED_EXCEPTION 1 // work around for tbb 4 exception bug
#include <tbb/tbb.h>
#pragma warning( pop )

#include <memory>

class dropboxupload;

class taskinfo
{
public:
	taskinfo(){}
	~taskinfo(){}
};

template <typename T> class task
{
public:
	task(const T& other,const taskinfo *pInfo):m_Task( other ), m_pTaskInfo( pInfo ){}
	task(const task& other):m_Task( other.m_Task ),m_pTaskInfo( other.m_pTaskInfo ){}
	
	void operator()( const tbb::blocked_range<int>& r ) const
	{
		m_Task( r.begin(), r.end() - 1, m_pTaskInfo );
	}
	
private:
	const T& m_Task;
	const taskinfo *m_pTaskInfo;
};

class scheduler
{
public:
	scheduler()
	{
		m_nCores = getsystemcores();
		m_pTbb = nullptr;
		createimpl();
	}
	virtual ~scheduler(){ destroyimpl(); }

	int getcores( void ) const { return m_nCores; }

	// T - task type
	template <typename T> void serial_for( const int nFrom, const int nCount, const T& t, const taskinfo *pInfo = AFNULL ) const
	{
		if( nCount <= 0 )
			return;
		t( nFrom, nFrom + nCount - 1, pInfo ); 
	}
	template <typename T> void parallel_for( const int nFrom, const int nCount, const int nCores, const T& t, const taskinfo *pInfo = AFNULL ) const
	{
		if( nCount <= 0 )
			return;
		int nGrain = 0;
		if( getgrain( nCount, nCores, nGrain ) )
			tbb::parallel_for( tbb::blocked_range<int>( nFrom, nCount, nGrain ), task<T>( t, pInfo ) );
		else
			t( nFrom, nFrom + nCount - 1, pInfo ); 
	}
protected:
	void *m_pTbb;
	int m_nCores;
	
	static int getsystemcores( void ) { SYSTEM_INFO si; GetSystemInfo( &si ); return ( si.dwNumberOfProcessors > 0 ) ? static_cast<int>(si.dwNumberOfProcessors) : 1; }
	bool getgrain( const int nItems, const int nCores, int& nGrain ) const
	{
		if( ( nCores < 2 ) || ( nItems < 2 ) )
			return false;
		nGrain = static_cast<int>( ( nItems / static_cast<float>( nCores ) ) + 0.5 );
		nGrain = ( nGrain < 1 ) ? 1 : nGrain;
		return true;
	}
	void createimpl( void )
	{
		destroyimpl();
		tbb::task_scheduler_init *pTbb = new tbb::task_scheduler_init();
		m_pTbb = pTbb;
	}
	void destroyimpl( void )
	{
		if( m_pTbb )
		{
			tbb::task_scheduler_init *pTbb = reinterpret_cast<tbb::task_scheduler_init*>( m_pTbb );
			pTbb->terminate();
			delete pTbb;
			m_pTbb = nullptr;
		}
	}
};

class uploadthreadrequest
{
public:
	enum type {t_serial,t_serial_tbb,t_parallel};
	
	uploadthreadrequest(const std::vector<std::shared_ptr<dropboxupload>>& vUploads,const std::wstring& sAccessToken,type t):m_vUploads(vUploads),m_Type(t),m_sAccessToken(sAccessToken){}
	~uploadthreadrequest(){}

	const std::vector<std::shared_ptr<dropboxupload>>& getuploads( void ) const { return m_vUploads; }
	const std::wstring& getaccesstoken( void ) const { return m_sAccessToken; }
	type gettype( void ) const { return m_Type; }

	void setcancelled( const bool b ) const;
protected:
	const std::vector<std::shared_ptr<dropboxupload>> m_vUploads;
	const std::wstring m_sAccessToken;
	const type m_Type;
};

class uploadthread : public CWinThread
{

DECLARE_DYNAMIC( uploadthread )

public:
	uploadthread();
	virtual ~uploadthread();

	virtual bool start( std::shared_ptr<const uploadthreadrequest> sp, std::shared_ptr<const scheduler> spSched,HWND hNotify );
	virtual bool stop( const DWORD dwTimeOut );
	
	virtual BOOL InitInstance( void ) override;
protected:	
	DECLARE_MESSAGE_MAP()
	
	HWND m_hNotify;
	std::shared_ptr<const uploadthreadrequest> m_sp;
	std::shared_ptr<const scheduler> m_spSched;
};
