/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#include <string>
#include <locale>
#include "StringCoverter.h"


std::string TStringConverter::UCS2toANSI
(
    const wchar_t* PStr,
    const size_t LengthChar,
    const std::locale& Locale,
    const char DefaultChar
)
{
    if (LengthChar == 0)
        return std::string();

    const std::ctype<wchar_t>& facet = std::use_facet<std::ctype<wchar_t>>(Locale);
    const wchar_t* PStrEnd = PStr + LengthChar;
    std::string Result(LengthChar, '\0');
    facet.narrow(PStr, PStrEnd, DefaultChar, &Result[0]);

    return Result;
}

std::string TStringConverter::UCS2toANSI
(
    const std::wstring& Str,
    const std::locale& Locale,
    const char DefaultChar
)
{
    return UCS2toANSI(Str.c_str(), Str.length(), Locale, DefaultChar);
}
