/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#define _WINSOCKAPI_
#include <windows.h>
#include <string>
#include <locale>
#include <vector>
#include "maxminddb.h"
#include "Protocol.h"
#include "DatabaseManager.h"
#include "BaseServer.h"
#include "GlobalOptions.h"
#include "EventLogWriter.h"
#include "TCPServer.h"
#include "PIPEServer.h"
#include "GeoIPService.h"
#include "GlobalVariables.h"


TGeoIPService* TGlobalVariables::PServiceThis = nullptr;
