
#include "pch.h"
#include "utils.h"
#include <codecvt>

namespace utils
{

namespace string
{
	std::string ws2s(const std::wstring& wstr)
	{
		typedef std::codecvt_utf8<wchar_t> convert_typeX;
		std::wstring_convert<convert_typeX, wchar_t> converterX;
		return converterX.to_bytes(wstr);
	}
	std::wstring s2ws(const std::string& str)
	{
		typedef std::codecvt_utf8<wchar_t> convert_typeX;
		std::wstring_convert<convert_typeX, wchar_t> converterX;
		return converterX.from_bytes(str);
	}
	std::string getpwd(void)
	{
		std::string s;

		char filename[_MAX_PATH];
		HMODULE hModule = AfxGetInstanceHandle();
		const DWORD len = hModule ? GetModuleFileNameA( hModule, filename, sizeof(filename)/sizeof(filename[0] ) ) : 0;
		if( len < 1 )
		{
			ASSERT( false );
			return s;
		}

		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		_splitpath_s( filename,drive,_MAX_DRIVE,dir,_MAX_DIR,nullptr,0,nullptr,0);

		s += drive;
		s += dir;
		
		return s;
	}
}

}
