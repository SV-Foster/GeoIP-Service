/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


#define ExtendedPathMaxChar 32767

// name of the service. maximum string length is 256 characters
#define SVCNAME TEXT("GeoIPSVC")
// use SC_GROUP_IDENTIFIER to be distinguished from a service name, if group name is used
// list of all groups can be found at HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\GroupOrderList
#define SERVICE_DEPENDENCIES TEXT("tcpip\0afd\0+File System\0+Base\0\0")

#define RegistryOptionsKey HKEY_LOCAL_MACHINE
#define RegistryOptionsPath TEXT("SOFTWARE\\SV Foster\\GeoIP Service")
#define RegistryOptionsServerList TEXT("Server")
#define RegistryValDatabaseFileNameASN TEXT("DatabaseFileNameASN")
#define RegistryValDatabaseFileNameGeo TEXT("DatabaseFileNameGeo")
#define RegistryValDatabasePath TEXT("DatabasePath")
#define RegistryValInstalledPath TEXT("InstalledPath")

#define RegistryOptionsInitValuesFile TEXT("init options.hiv")
#define DatabaseDefaultFolderName TEXT("Database")

#define DefaultPipeName TEXT("\\\\.\\PIPE\\GeoIPSVCv1")
#define DefaultTCPServer TEXT("localhost")
#define DefaultTCPPort 28780 // chosen by fair dice roll. guaranteed to be random.
#define DefaultTCPPortStr TEXT("28780")

#define DefaultBufferSize 2048
#define ThreadStackSizeDefault 8192 // 2 pages
