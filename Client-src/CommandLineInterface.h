/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


#if defined(ENV64BIT)
	#define ArchString TEXT("x64 (x86-64)")
#elif defined (ENV32BIT)
	#define ArchString TEXT("x86 (IA-32)")
#else
	#error "Must define either ENV32BIT or ENV64BIT"
#endif


BOOL  CLIWorkModeGet(CONST DWORD argc, LPCTSTR argv[], PGlobalOptions glo);
BOOL  CLISwitchesGet(CONST DWORD argc, LPCTSTR argv[], PGlobalOptions glo);
VOID  CLILogoPrint();
VOID  CLIHelpPrint();
VOID  CLIWriteLN();
BOOL  CLISetModeUTF16();
BOOL  StrToDWORD(PDWORD Data, LPCTSTR Str);
