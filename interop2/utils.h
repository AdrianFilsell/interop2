#pragma once

// Created by Adrian Filsell 2018
// Copyright 2018 Adrian Filsell. All rights reserved.

#include <string>
#include <cctype>
#include <cwctype>
#include <stdarg.h>

namespace utils
{

inline std::wstring wszfmt( const std::wstring& fmt, va_list *pArgs )
{
	int cchNeeded = __stdio_common_vswprintf( _CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_STANDARD_SNPRINTF_BEHAVIOR | _CRT_INTERNAL_PRINTF_LEGACY_WIDE_SPECIFIERS,
													nullptr, 0, fmt.c_str(), nullptr, *pArgs );
	std::wstring wszBuf;
	if( cchNeeded >= 0 )
	{
		wszBuf.resize( size_t(cchNeeded + 1) );
		int const vsnwprintf_result = __stdio_common_vsnwprintf_s( _CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_LEGACY_WIDE_SPECIFIERS,
																		&(*wszBuf.begin()), size_t(cchNeeded + 1), cchNeeded, fmt.c_str(), nullptr, *pArgs );
		wszBuf.resize( size_t(cchNeeded) );
	}
	return wszBuf;
}

namespace string
{
	std::string getpwd(void);

	std::string ws2s(const std::wstring& wstr);
	std::wstring s2ws(const std::string& str);
	// S - string type
	template <typename S> inline S szfmt(const S s,...);
	// S - string type
	template <typename S> S makeguid(void);
	template <typename S> S& makeupper(S& s);
	template <typename S> S& makelower(S& s);
	template <typename S> S toupper(const S& s) {S t=s;makeupper<S>(t);return t;}
	template <typename S> S tolower(const S& s) {S t=s;makelower<S>(t);return t;}
}

template <> inline std::string& string::makeupper(std::string& s)
{
	auto i = s.begin(), end = s.end();
	for( ; i != end ; ++i )
		*i=std::toupper(*i);
	return s;
}

template <> inline std::wstring& string::makeupper(std::wstring& s)
{
	auto i = s.begin(), end = s.end();
	for( ; i != end ; ++i )
		*i=std::towupper(*i);
	return s;
}

template <> inline std::string& string::makelower(std::string& s)
{
	auto i = s.begin(), end = s.end();
	for( ; i != end ; ++i )
		*i=std::tolower(*i);
	return s;
}

template <> inline std::wstring& string::makelower(std::wstring& s)
{
	auto i = s.begin(), end = s.end();
	for( ; i != end ; ++i )
		*i=std::towlower(*i);
	return s;
}

template <> inline std::string string::szfmt(const std::string s,...)
{
	va_list args;va_start( args, s );
	std::string out = ws2s( wszfmt( s2ws( s ), &args ) );
	va_end( args );
	return out;
}
template <> inline std::wstring string::szfmt(const std::wstring s,...)
{
	va_list args;va_start( args, s );
	std::wstring out = wszfmt( s, &args );
	va_end( args );
	return out;
}

}
