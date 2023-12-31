/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


DWORD IP4or6Request();
DWORD ASNRequest();
DWORD ServerPing();
DWORD DBHotplug();
BOOL  IPAddrConvert(LPCTSTR IPStr, PDWORD IPFamily, PBYTE IPBinary);
BOOL  TransportRequest(PBYTE Request, DWORD RequestSize, PBYTE Response, DWORD ResponseSize);
BOOL  PIPERequest(LPTSTR PipeName, PBYTE Request, DWORD RequestSize, PBYTE Response, DWORD ResponseSize);
BOOL  TCPRequest(LPCTSTR TCPServer, LPCTSTR TCPPort, int TCPVersion, PBYTE Request, DWORD RequestSize, PBYTE Response, DWORD ResponseSize);
BOOL  WSAStartupLocal();
