/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#define _WINSOCKAPI_
#include <windows.h>
#include <winsock2.h>
#include "GlobalOptions.h"
#include "SharedOptionsDefaults.h"


TGlobalOptions GlobalOptions;

VOID GlobalOptionsDefaultsSet(PGlobalOptions glo)
{
    ZeroMemory(glo, sizeof(TGlobalOptions));

    glo->OperatingMode = OperatingModeHelp;
    glo->TransportToServer = TransportToServerPIPE;
    glo->PipeName = DefaultPipeName;
    glo->PIPEIOTimeout = NMPWAIT_USE_DEFAULT_WAIT;
    glo->NetServerAdress = DefaultTCPServer;
    glo->NetServerPort = DefaultTCPPortStr;
    glo->TCPVersion = AF_INET6;
    glo->TCPConnectTimeoutSec = 3;
    glo->TCPIOTimeoutMSec = 5000;
}
