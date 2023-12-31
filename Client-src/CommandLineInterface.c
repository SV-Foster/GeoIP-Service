/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#define _WINSOCKAPI_
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>
#include "SharedHeaders.h"
#include "resource.h"
#include "GlobalOptions.h"
#include "LanguageRes.h"
#include "Protocol.h"
#include "CommandLineInterface.h"

#pragma comment(lib, "Version.lib")

#pragma warning( disable : 6255 )


BOOL CLIWorkModeGet(CONST DWORD argc, LPCTSTR argv[], PGlobalOptions glo)
{
	glo->OperatingMode = OperatingModeHelp;

	if (argc <= 1)
		return TRUE;

	for (DWORD i = 1; i < argc; ++i)
	{
		if (_tcsicmp(argv[i], TEXT("/Help")) == 0)
			return TRUE;

		if (_tcsicmp(argv[i], TEXT("/?")) == 0)
			return TRUE;
	}

	// default is PIPE mode
	glo->OperatingMode = OperatingModeIPRequest;
	for (DWORD i = 1; i < argc; ++i)
		if (_tcsicmp(argv[i], TEXT("/Mode")) == 0)
			if ((i + 1) < argc)
			{
				if (_tcsicmp(argv[i + 1], TEXT("Request")) == 0)
					return TRUE; // was set by default

				if (_tcsicmp(argv[i + 1], TEXT("Ping")) == 0)
				{
					glo->OperatingMode = OperatingModePing;
					return TRUE;
				}

				if (_tcsicmp(argv[i + 1], TEXT("Hotplug")) == 0)
				{
					glo->OperatingMode = OperatingModeDBHotplug;
					return TRUE;
				}

				if (_tcsicmp(argv[i + 1], TEXT("RequestASN")) == 0)
				{
					glo->OperatingMode = OperatingModeASNRequest;
					return TRUE;
				}

				// wrong params
				CLILogoPrint();
				_tprintf_s(LangGet(UIMSG_119_ERR_INVALID_VALUE), argv[i+1]);
				return FALSE;
			}
			else
			{
				// wrong params
				CLILogoPrint();
				_tprintf_s(LangGet(UIMSG_101_ERR_PARAMS_NO_MODE));
				return FALSE;
			}

	return TRUE;
}

BOOL CLISwitchesGet(CONST DWORD argc, LPCTSTR argv[], PGlobalOptions glo)
{
	BOOL LengthError = FALSE;
	long PortTest;


	for (DWORD i = 1; i < argc; ++i)
	{
		if (_tcsicmp(argv[i], TEXT("/Mode")) == 0)
		{
			++i;
			continue; // just skip, processed in CLIWorkModeGet()
		}

		if (_tcsicmp(argv[i], TEXT("/NoLogo")) == 0)
		{
			glo->NoCopyrightLogo = TRUE;
			continue;
		}

		if (_tcsicmp(argv[i], TEXT("/IP")) == 0)
			if ((i + 1) < argc)
			{
				glo->ProcessIP = (LPTSTR)argv[++i];
				continue;
			}
			else
				LengthError = TRUE;

		if (_tcsicmp(argv[i], TEXT("/Transport")) == 0)
			if ((i + 1) < argc)
			{
				if (_tcsicmp(argv[i + 1], TEXT("PIPE")) == 0)
				{
					glo->TransportToServer = TransportToServerPIPE;
					++i;
					continue;
				}

				if (_tcsicmp(argv[i + 1], TEXT("TCP4")) == 0)
				{
					glo->TransportToServer = TransportToServerTCP;
					glo->TCPVersion = AF_INET;
					++i;
					continue;
				}

				if (_tcsicmp(argv[i + 1], TEXT("TCP6")) == 0)
				{
					glo->TransportToServer = TransportToServerTCP;
					glo->TCPVersion = AF_INET6;
					++i;
					continue;
				}

				// wrong params
				CLILogoPrint();
				_tprintf_s(LangGet(UIMSG_119_ERR_INVALID_VALUE), argv[i + 1]);
				return FALSE;
			}
			else
				LengthError = TRUE;

		if (_tcsicmp(argv[i], TEXT("/PipeName")) == 0)
			if ((i + 1) < argc)
			{
				glo->PipeName = (LPTSTR)argv[++i];
				continue;
			}
			else
				LengthError = TRUE;

		if (_tcsicmp(argv[i], TEXT("/PIPETimeoutIO")) == 0)
			if ((i + 1) < argc)
			{
				// is this a number?
				if (!StrToDWORD(&glo->PIPEIOTimeout, argv[i + 1]))
				{
					// wrong params
					CLILogoPrint();
					_tprintf_s(LangGet(UIMSG_119_ERR_INVALID_VALUE), argv[i + 1]);
					return FALSE;
				}				
				switch (glo->PIPEIOTimeout)
				{
				case 0xffffffff: // Waits indefinitely
				case 0x00000001: // Does not wait for the named pipe. If the named pipe is not available, the function returns an error
				//case 0x00000000: //  Uses the default time-out specified in a call to the CreateNamedPipe() function
					break;

				default:
					glo->PIPEIOTimeout = glo->PIPEIOTimeout * 1000;
					break;
				}

				++i;
				continue;
			}
			else
				LengthError = TRUE;

		if (_tcsicmp(argv[i], TEXT("/NetServer")) == 0)
			if ((i + 1) < argc)
			{
				glo->NetServerAdress = (LPTSTR)argv[++i];
				continue;
			}
			else
				LengthError = TRUE;

		if (_tcsicmp(argv[i], TEXT("/NetPort")) == 0)
			if ((i + 1) < argc)
			{
				// is this a number?
				if (!StrToDWORD(&PortTest, argv[i + 1]))
				{
					// wrong params
					CLILogoPrint();
					_tprintf_s(LangGet(UIMSG_119_ERR_INVALID_VALUE), argv[i + 1]);
					return FALSE;
				}
				// Make sure the wildcard port wasn't specified
				if ((PortTest < 1 ) || (PortTest > 65535 ))
				{
					// wrong params
					CLILogoPrint();
					_tprintf_s(LangGet(UIMSG_119_ERR_INVALID_VALUE), argv[i + 1]);
					return FALSE;
				}

				glo->NetServerPort = (LPTSTR)argv[++i];
				continue;
			}
			else
				LengthError = TRUE;

		if (_tcsicmp(argv[i], TEXT("/NetTimeoutConnect")) == 0)
			if ((i + 1) < argc)
			{
				// is this a number?
				if (!StrToDWORD(&glo->TCPConnectTimeoutSec, argv[i + 1]))
				{
					// wrong params
					CLILogoPrint();
					_tprintf_s(LangGet(UIMSG_119_ERR_INVALID_VALUE), argv[i + 1]);
					return FALSE;
				}

				++i;
				continue;
			}
			else
				LengthError = TRUE;

		if (_tcsicmp(argv[i], TEXT("/NetTimeoutIO")) == 0)
			if ((i + 1) < argc)
			{
				// is this a number?
				if (!StrToDWORD(&glo->TCPIOTimeoutMSec, argv[i + 1]))
				{
					// wrong params
					CLILogoPrint();
					_tprintf_s(LangGet(UIMSG_119_ERR_INVALID_VALUE), argv[i + 1]);
					return FALSE;
				}

				++i;
				continue;
			}
			else
				LengthError = TRUE;

		if (_tcsicmp(argv[i], TEXT("/FileGeo")) == 0)
			if ((i + 1) < argc)
			{
				glo->FileGeo = (LPTSTR)argv[++i];
				
				size_t l = _tcslen(glo->FileGeo);
				if ((l >= DBFileLengthMaxCharWZero) || (l <= 0))
				{
					_tprintf_s(LangGet(UIMSG_119_ERR_INVALID_VALUE), glo->FileGeo);
					return FALSE;
				}

				continue;
			}
			else
				LengthError = TRUE;

		if (_tcsicmp(argv[i], TEXT("/FileASN")) == 0)
			if ((i + 1) < argc)
			{
				glo->FileASN = (LPTSTR)argv[++i];

				size_t l = _tcslen(glo->FileASN);
				if ((l >= DBFileLengthMaxCharWZero) || (l <= 0))
				{
					_tprintf_s(LangGet(UIMSG_119_ERR_INVALID_VALUE), glo->FileASN);
					return FALSE;
				}

				continue;
			}
			else
				LengthError = TRUE;

		if (LengthError)
		{
			CLILogoPrint();
			_tprintf_s(LangGet(UIMSG_102_ERR_PARAMS_LENGTH), argv[i]);
			return FALSE;
		}

		if (argv[i][0] == TEXT('/'))
		{
			CLILogoPrint();
			_tprintf_s(LangGet(UIMSG_121_ERR_PARAMS_BAD_SW), argv[i]);
			return FALSE;
		}

		CLILogoPrint();
		_tprintf_s(LangGet(UIMSG_119_ERR_INVALID_VALUE), argv[i]);
		return FALSE;
	}

	return TRUE;
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

BOOL StrToDWORD(PDWORD Data, LPCTSTR Str)
{
	LPTSTR EndS;


	*Data = _tcstol(Str, &EndS, 10);
	return !((*EndS != TEXT('\0')) || (errno == ERANGE) || (errno == EINVAL));
}
