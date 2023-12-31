/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


#define DefaultCharForConversionSubsANSI   '?'
#define DefaultCharForConversionSubsUCS2  u'?'
#define DefaultCharForConversionSubsUTF8 u8'?'


class TStringConverter
{
public:
	TStringConverter() = delete;
	~TStringConverter() = delete;
	TStringConverter(const TStringConverter& oth) = delete;
	TStringConverter& operator=(const TStringConverter& oth) = delete;
	TStringConverter(TStringConverter&& oth) = delete;
	TStringConverter& operator=(TStringConverter&& oth) = delete;


	static std::string UCS2toANSI(const wchar_t* PStr, const size_t LengthChar, const std::locale& Locale = std::locale(), const char DefaultChar = DefaultCharForConversionSubsANSI);
	static std::string UCS2toANSI(const std::wstring& Str, const std::locale& Locale = std::locale(), const char DefaultChar = DefaultCharForConversionSubsANSI);
};
