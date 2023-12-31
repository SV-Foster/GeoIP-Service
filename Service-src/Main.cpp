/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#define _WINSOCKAPI_
#include <windows.h>
#include <tchar.h>
#include <string>
#include <locale>
#include <vector>
#include "maxminddb.h"
#include "SharedOptionsDefaults.h"
#include "Protocol.h"
#include "DatabaseManager.h"
#include "BaseServer.h"
#include "GlobalOptions.h"
#include "EventLogWriter.h"
#include "TCPServer.h"
#include "PIPEServer.h"
#include "GeoIPService.h"
#include "GeoIPServiceCMD.h"
#include "Main.h"


#ifdef _CONSOLE
int _tmain(DWORD argc, LPCTSTR argv[], LPCTSTR envp[])
{
    std::wstring ServiceName(SVCNAME);
    TGeoIPServiceCMD Service(ServiceName);
#endif // _CONSOLE
#ifdef _WINDOWS
int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
        std::wstring ServiceName(SVCNAME);
        TGeoIPService Service(ServiceName);
#endif // _WINDOWS

    return Service.Run();
}
