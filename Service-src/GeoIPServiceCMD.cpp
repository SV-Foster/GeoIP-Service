/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#ifdef _CONSOLE
#define _WINSOCKAPI_
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdio.h>
#include <string>
#include <locale>
#include <array>
#include <vector>
#include <memory>
#include "SharedHeaders.h"
#include "SharedOptionsDefaults.h"
#include "maxminddb.h"
#include "LanguageRes.h"
#include "EventLog.h"
#include "resource.h"
#include "Protocol.h"
#include "DatabaseManager.h"
#include "BaseServer.h"
#include "GlobalOptions.h"
#include "EventLogWriter.h"
#include "TCPServer.h"
#include "PIPEServer.h"
#include "GeoIPService.h"
#include "GeoIPServiceCMD.h"
#include "GlobalVariables.h"


static TGeoIPServiceCMD* sm_pService = nullptr;

TGeoIPServiceCMD::TGeoIPServiceCMD(std::wstring& NameShort) :
    TGeoIPService(NameShort) // inherited Create
{
    sm_pService = this;
}

TGeoIPServiceCMD::~TGeoIPServiceCMD() noexcept
{
    sm_pService = nullptr;
}

DWORD TGeoIPServiceCMD::Run()
{
    SetErrorMode(SEM_FAILCRITICALERRORS); // Don't popup on floppy query and etc.

    return CMDMain();
}

DWORD TGeoIPServiceCMD::CMDMain()
{
    DWORD Result = EXIT_SUCCESS;


    _tprintf_s(TEXT("GeoIP Service is in the debug mode\nPress Ctrl-C or Ctrl-Break to stop\n\n"));

    if (!EventsInit())
        ExitFunction(EXIT_FAILURE);

    if (!AppInit())
        ExitFunction(EXIT_FAILURE);

    SetConsoleCtrlHandler(WinAPICMDCtrlHandler, TRUE);

    if (!AppResume())
        ExitFunction(EXIT_FAILURE);

    AppMain();


function_end:
    EventLog(ELMSG_SHUTDOWN);
    SetConsoleCtrlHandler(WinAPICMDCtrlHandler, FALSE);

    AppCease();
    AppCleanup();

    _tprintf_s(TEXT("Stopped. Exit code is %lu\n\n"), Result);
    return Result;
}

BOOL WINAPI TGeoIPServiceCMD::WinAPICMDCtrlHandler(DWORD dwCtrlType)
{
    if (sm_pService)
        return sm_pService->CMDCtrlHandler(dwCtrlType);

    return FALSE;
}

// Handled console control events
BOOL TGeoIPServiceCMD::CMDCtrlHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
    case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
    case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
        EventLog(ELMSG_SERVICE_CONTROL_STOP);
        SetEvent(this->SvcStopEvent);
        return TRUE;

    default:
        break;
    }

    return FALSE;
}

#endif //  _CONSOLE
