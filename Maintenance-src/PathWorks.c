/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "PathWorks.h"

#pragma warning( disable : 6255)


BOOL PathStringQuoteIfHasSpaces(LPTSTR PathString, SIZE_T LengthChar)
{
    LPTSTR TempPtr = PathString;
    BOOL HasSpaces = FALSE;
    LPTSTR BufferS;


    if (PathString == NULL)
        return FALSE;

    while (TRUE)
    {
        // bad string, terminating NULL char not found or length = 0
        if (TempPtr == (PathString + LengthChar))
            return FALSE;

        // found the end
        if (*TempPtr == TEXT('\0'))
            break;

        // found space
        if (*TempPtr == TEXT(' '))
        {
            HasSpaces = TRUE;
            break;
        }

        ++TempPtr;
    }

    // no need to quote
    if (!HasSpaces)
        return TRUE;

    // is already quoted?
    if (*PathString == TEXT('\"'))
        return TRUE;

    BufferS = _alloca(LengthChar * sizeof(TCHAR));
    if (FAILED(StringCbPrintf(BufferS, LengthChar, TEXT("\"%s\""), PathString)))
        return FALSE;

	// copy the end result
	_tcscpy_s(PathString, LengthChar, BufferS);

    return TRUE;
}

VOID PathStringSlashTrailingIclude(LPTSTR PathString, CONST DWORD LengthMax)
{
	SIZE_T Length = _tcslen(PathString);
	if (Length > 1)
		if (PathString[Length - 1] == TEXT('\\'))
			return;

	_tcscat_s(PathString, LengthMax, TEXT("\\"));
}

BOOL PathStringSlashFindLast(LPCTSTR PathString, PSIZE_T Position)
{
	LPCTSTR StrEnd = PathString + _tcslen(PathString);
	while (StrEnd != PathString) {
		if (StrEnd[0] == TEXT('\\'))
		{
			*Position = StrEnd - PathString;
			return TRUE;
		}
		--StrEnd;
	}

	return FALSE;
}

VOID PathStringFileNameRemove(LPTSTR PathString)
{
	SIZE_T pos;
	LPTSTR Buffer;


	if (PathString == NULL)
		return;

	// ignore UNC and long path slashes
	if (_tcsstr(PathString, TEXT("\\\\")) == PathString)
	{
		// long path
		if (_tcsstr(PathString, TEXT("\\\\?\\")) == PathString)
		{
			PathString += 4;
			goto sub_end;
		}

		// local DOS device namespace
		if (_tcsstr(PathString, TEXT("\\\\.\\")) == PathString)
		{
			PathString += 4;
			goto sub_end;
		}

		// UNC
		// server name
		PathString += 2;
		Buffer = _tcsstr(PathString, TEXT("\\"));
		if (!Buffer)
			goto sub_end;
		PathString = Buffer;
		// share name
		Buffer = _tcsstr(PathString, TEXT("\\"));
		if (!Buffer)
			goto sub_end;
		PathString = Buffer;

	}
sub_end:

	// just a drive name. like D:
	if (_tcslen(PathString) == 2)
		if (_tcsstr(PathString, TEXT(":")) == (PathString + 1))
			return;

	// no slashes at all -- this is a file in the current folder
	if (!PathStringSlashFindLast(PathString, &pos))
	{
		PathString[0] = TEXT('\0');
		return;
	}

	/*
	   slash at the end means this is path to a folder, aka
	   D:\ or C:\Windows\Temp\
	*/
	if (PathString[pos + 1] == TEXT('\0'))
		return;

	PathString[pos + 1] = TEXT('\0');
}
