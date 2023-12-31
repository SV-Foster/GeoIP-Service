/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>
#include "SharedHeaders.h"
#include "resource.h"
#include "GlobalOptions.h"
#include "LanguageRes.h"
#include "CommandLineInterface.h"

#pragma comment(lib, "Version.lib")

#pragma warning( disable : 6255 )


BOOL CLIWorkModeGet(CONST DWORD argc, LPCTSTR argv[], PGlobalOptions glo)
{
	glo->OperatingMode = OperatingModeHelp;

	if (argc <= 1)
	{
		return TRUE;
	}

	for (DWORD i = 0; i < argc; ++i)
	{
		if (_tcsicmp(argv[i], TEXT("/Help")) == 0)
		{
			return TRUE;
		}

		if (_tcsicmp(argv[i], TEXT("/?")) == 0)
		{
			return TRUE;
		}
	}

	if (_tcsicmp(argv[1], TEXT("Install")) == 0)
	{
		glo->OperatingMode = OperatingModeInstall;
		return TRUE;
	}

	if (_tcsicmp(argv[1], TEXT("Uninstall")) == 0)
	{
		glo->OperatingMode = OperatingModeUninstall;
		return TRUE;
	}

	CLILogoPrint();
	_tprintf_s(LangGet(UIMSG_116_NO_COMMAND));
	return FALSE;
}

BOOL CLIPathsGet(CONST DWORD argc, LPCTSTR argv[], PGlobalOptions glo)
{
	switch (glo->OperatingMode)
	{
	case OperatingModeInstall:
		if (argc <= 2)
		{
			CLILogoPrint();
			_tprintf_s(LangGet(UIMSG_106_NO_FILENAME_OR_PATH));
			break;
		}

		if (argc == 3)
		{
			glo->PathToServiceEXE = argv[2];
			return TRUE;
		}

		// argc > 3
		CLILogoPrint();
		_tprintf_s(LangGet(UIMSG_119_ERR_PARAMS_TOO_MANY_AGRS), argv[3]);
		break;

	case OperatingModeUninstall:
		if (argc == 2)
			return TRUE;

		// argc > 2
		CLILogoPrint();
		_tprintf_s(LangGet(UIMSG_119_ERR_PARAMS_TOO_MANY_AGRS), argv[2]);
		break;

	default:
		break;
	}

	return FALSE;
}

VOID CLILogoPrint()
{
	TCHAR FileName[MAX_PATH + 1];
	DWORD FileNameSize;
	DWORD Handle;
	DWORD FileVersionInfoSize;
	LPVOID BufferData;
	UINT len;
	LPVOID CopyrightString;
	LPVOID ProductNameString;
	LPVOID versionInfo;
	VS_FIXEDFILEINFO* fileInfo;


	FileNameSize = GetModuleFileName(NULL, (LPTSTR)&FileName, MAX_PATH);
	if (!FileNameSize)
		return;
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		return;
	FileName[FileNameSize] = TEXT('\0');

	FileVersionInfoSize = GetFileVersionInfoSize((LPTSTR)&FileName, &Handle);
	if (!FileVersionInfoSize)
		return;

	BufferData = _alloca(FileVersionInfoSize);
	if (!BufferData)
		return;

	if (!GetFileVersionInfo((LPTSTR)&FileName, 0, FileVersionInfoSize, BufferData))
		return;

	if (!VerQueryValue(BufferData, TEXT("\\"), &versionInfo, &len))
		return;

	fileInfo = (VS_FIXEDFILEINFO*)versionInfo;
	if (fileInfo->dwSignature != 0xfeef04bd)
		return;

	if (!VerQueryValue(BufferData, TEXT("\\StringFileInfo\\040904B0\\FileDescription"), &ProductNameString, &len))
		return;

	if (!VerQueryValue(BufferData, TEXT("\\StringFileInfo\\040904B0\\LegalCopyright"), &CopyrightString, &len))
		return;

	_tprintf_s(LangGet(UIMSG_118_LOGO_TEXT),
		ProductNameString,
		HIWORD(fileInfo->dwProductVersionMS),
		LOWORD(fileInfo->dwProductVersionMS),
		ArchString,
		CopyrightString
	);
}

VOID CLIHelpPrint()
{
	_tprintf_s(LangGet(UIMSG_117_HELP_TEXT));
}

VOID CLIWriteLN()
{
	_tprintf_s(TEXT("\n"));
}

BOOL CLISetModeUTF16()
{
	if (_setmode(_fileno(stdout), _O_U16TEXT) == -1)
		return FALSE;

	if (_setmode(_fileno(stdin), _O_U16TEXT) == -1)
		return FALSE;

	if (_setmode(_fileno(stderr), _O_U16TEXT) == -1)
		return FALSE;

	return TRUE;
}
