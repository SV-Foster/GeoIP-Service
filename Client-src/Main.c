/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#define _WINSOCKAPI_
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h>
#include "PrintfBinaryPattern.h"
#include "SharedHeaders.h"
#include "resource.h"
#include "Protocol.h"
#include "GlobalOptions.h"
#include "SharedOptionsDefaults.h"
#include "LanguageRes.h"
#include "CommandLineInterface.h"
#include "Main.h"

#pragma comment(lib, "ws2_32.lib")


extern TGlobalOptions GlobalOptions;
BOOL WSAReady = FALSE;

DWORD _tmain(DWORD argc, LPCTSTR argv[], LPCTSTR envp[])
{
    DWORD Result = EXIT_SUCCESS;


    CLISetModeUTF16();
    SetErrorMode(SEM_FAILCRITICALERRORS); // Don't popup on floppy query and etc.
    CLIWriteLN();

    // set options
    GlobalOptionsDefaultsSet(&GlobalOptions);
    if (!CLIWorkModeGet(argc, argv, &GlobalOptions))
        return EXIT_FAILURE;
    if (GlobalOptions.OperatingMode == OperatingModeHelp)
    {
        CLILogoPrint();
        CLIHelpPrint();
        return EXIT_SUCCESS;
    }
    if (!CLISwitchesGet(argc, argv, &GlobalOptions))
        return EXIT_FAILURE;

    if (!GlobalOptions.NoCopyrightLogo)
        CLILogoPrint();

  
    // make some work
    switch (GlobalOptions.OperatingMode)
    {
    case OperatingModeIPRequest:
        Result = IP4or6Request();

        break;

    case OperatingModePing:
        Result = ServerPing();

        break;

    case OperatingModeDBHotplug:
        Result = DBHotplug();

        break;

    case OperatingModeASNRequest:
        Result = ASNRequest();

        break;

    default:
        Result = EXIT_FAILURE;
    }


    if (WSAReady)
        WSACleanup();
    
    return Result;
}

DWORD IP4or6Request()
{
    DWORD Result = EXIT_SUCCESS;
    DWORD IPFamily;
    BYTE IPBinary[16];
    TGeoIPSRVRequestGeoIPv4 Request4;
    TGeoIPSRVRequestGeoIPv6 Request6;
    PVOID PRequest;
    DWORD RequestSize;
    TGeoIPSRVResponseGeoIP Response;


    if (GlobalOptions.ProcessIP)
        _tprintf_s(LangGet(UIMSG_103_REQUESTING_DATA_4IP), GlobalOptions.ProcessIP);
    else
    {
        _tprintf_s(LangGet(UIMSG_112_ERR_NO_IP_PROVIDED));
        ExitFunction(EXIT_FAILURE);
    }

    // get an IP adress in binary form
    if (!IPAddrConvert(GlobalOptions.ProcessIP, &IPFamily, (PBYTE)&IPBinary))
    {
        _tprintf_s(LangGet(UIMSG_105_ERROR_CANT_CONVERT_IP));
        ExitFunction(EXIT_FAILURE);
    }

    // form request packet
    switch (IPFamily)
    {
    case AF_INET:
        ZeroMemory(&Request4, sizeof(TGeoIPSRVRequestGeoIPv4));
        Request4.Header.Type = GeoIPSRVRequestGeoIPv4;
        Request4.Header.Length = sizeof(TGeoIPSRVRequestGeoIPv4);
        memcpy_s(&Request4.Addr, 4, &IPBinary, 4);
        PRequest = &Request4;
        RequestSize = Request4.Header.Length;

        break;

    case AF_INET6:
        ZeroMemory(&Request6, sizeof(TGeoIPSRVRequestGeoIPv6));
        Request6.Header.Type = GeoIPSRVRequestGeoIPv6;
        Request6.Header.Length = sizeof(TGeoIPSRVRequestGeoIPv6);
        memcpy_s(&Request6.Addr, 16, &IPBinary, 16);
        PRequest = &Request6;
        RequestSize = Request6.Header.Length;

        break;

    default:
        ExitFunction(FALSE);
    }

    // call the server
    if (!TransportRequest((PBYTE)PRequest, RequestSize, (PBYTE)&Response, sizeof(TGeoIPSRVResponseGeoIP)))
        ExitFunction(EXIT_FAILURE);

    if (Response.Header.Type != GeoIPSRVReplyGeoIP)
    {
        _tprintf_s(LangGet(UIMSG_114_BAD_SERVER_REPLY));
        ExitFunction(EXIT_FAILURE);
    }

    // prepare some strings
    // The _snprintf_s function formats and stores count or fewer characters in buffer and appends a terminating NULL
    TCHAR BufferFlags[33];
    _sntprintf_s(BufferFlags, _countof(BufferFlags), 32, PRINTF_BINARY_PATTERN_INT32, PRINTF_BYTE_TO_BINARY_INT32(Response.Flags));
    TCHAR BufferContinentID[11];
    _sntprintf_s(BufferContinentID, _countof(BufferContinentID), 10, TEXT("%lu"), Response.ContinentID);
    TCHAR BufferCountryID[11];
    _sntprintf_s(BufferCountryID, _countof(BufferCountryID), 10, TEXT("%lu"), Response.CountryID);
    TCHAR BufferCityID[11];
    _sntprintf_s(BufferCityID, _countof(BufferCityID), 10, TEXT("%lu"), Response.CityID);
    TCHAR Bufferlatitude[25];
    _sntprintf_s(Bufferlatitude, _countof(Bufferlatitude), 24, TEXT("%f"), Response.Location.latitude);
    TCHAR Bufferlongitude[25];
    _sntprintf_s(Bufferlongitude, _countof(Bufferlongitude), 24, TEXT("%f"), Response.Location.longitude);
    TCHAR Bufferaccuracy_radius[11];
    _sntprintf_s(Bufferaccuracy_radius, _countof(Bufferaccuracy_radius), 10, TEXT("%u"), Response.Location.accuracy_radius);

    _tprintf_s
    (
        LangGet(UIMSG_108_DATA_RECIVED_PRINT),

        Response.Header.Type,
        Response.Header.Length,
        BufferFlags, //Response.Flags,
        (Response.Flags & ReplySharedFlagsDBOperational) ? TEXT("True") : TEXT("False"), // DB Operational
        (Response.Flags & ReplyGeoIPFlagsFoundContinent) ? BufferContinentID     : LangGet(UIMSG_122_NOTFOUND), // Response.ContinentID,
        (Response.Flags & ReplyGeoIPFlagsFoundCountry)   ? BufferCountryID       : LangGet(UIMSG_122_NOTFOUND), // Response.CountryID,
        (Response.Flags & ReplyGeoIPFlagsFoundCity)      ? BufferCityID          : LangGet(UIMSG_122_NOTFOUND), // Response.CityID,
        (Response.Flags & ReplyGeoIPFlagsFoundLocation)  ? Bufferlatitude        : LangGet(UIMSG_122_NOTFOUND), // Response.Location.latitude,
        (Response.Flags & ReplyGeoIPFlagsFoundLocation)  ? Bufferlongitude       : LangGet(UIMSG_122_NOTFOUND), // Response.Location.longitude,
        (Response.Flags & ReplyGeoIPFlagsFoundLocation)  ? Bufferaccuracy_radius : LangGet(UIMSG_122_NOTFOUND)  // Response.Location.accuracy_radius
    );


function_end:
    return Result;
}

DWORD ASNRequest()
{
    DWORD Result = EXIT_SUCCESS;
    DWORD IPFamily;
    BYTE IPBinary[16];
    TGeoIPSRVRequestASNIPv4 Request4;
    TGeoIPSRVRequestASNIPv6 Request6;
    PVOID PRequest;
    DWORD RequestSize;
    TGeoIPSRVResponseASN Response;


    if (GlobalOptions.ProcessIP)
        _tprintf_s(LangGet(UIMSG_124_REQUESTING_DATA_ASN_4IP), GlobalOptions.ProcessIP);
    else
    {
        _tprintf_s(LangGet(UIMSG_112_ERR_NO_IP_PROVIDED));
        ExitFunction(EXIT_FAILURE);
    }

    // get an IP adress in binary form
    if (!IPAddrConvert(GlobalOptions.ProcessIP, &IPFamily, (PBYTE)&IPBinary))
    {
        _tprintf_s(LangGet(UIMSG_105_ERROR_CANT_CONVERT_IP));
        ExitFunction(EXIT_FAILURE);
    }

    // form request packet
    switch (IPFamily)
    {
    case AF_INET:
        ZeroMemory(&Request4, sizeof(TGeoIPSRVRequestASNIPv4));
        Request4.Header.Type = GeoIPSRVRequestASNIPv4;
        Request4.Header.Length = sizeof(TGeoIPSRVRequestASNIPv4);
        memcpy_s(&Request4.Addr, 4, &IPBinary, 4);
        PRequest = &Request4;
        RequestSize = Request4.Header.Length;

        break;

    case AF_INET6:
        ZeroMemory(&Request6, sizeof(TGeoIPSRVRequestASNIPv6));
        Request6.Header.Type = GeoIPSRVRequestASNIPv6;
        Request6.Header.Length = sizeof(TGeoIPSRVRequestASNIPv6);
        memcpy_s(&Request6.Addr, 16, &IPBinary, 16);
        PRequest = &Request6;
        RequestSize = Request6.Header.Length;

        break;

    default:
        ExitFunction(FALSE);
    }

    // call the server
    if (!TransportRequest((PBYTE)PRequest, RequestSize, (PBYTE)&Response, sizeof(TGeoIPSRVResponseASN)))
        ExitFunction(EXIT_FAILURE);

    if (Response.Header.Type != GeoIPSRVReplyASN)
    {
        _tprintf_s(LangGet(UIMSG_114_BAD_SERVER_REPLY));
        ExitFunction(EXIT_FAILURE);
    }

    // prepare some strings
    // The _snprintf_s function formats and stores count or fewer characters in buffer and appends a terminating NULL
    TCHAR BufferFlags[33];
    _sntprintf_s(BufferFlags, _countof(BufferFlags), 32, PRINTF_BINARY_PATTERN_INT32, PRINTF_BYTE_TO_BINARY_INT32(Response.Flags));
    TCHAR BufferASN[11];
    _sntprintf_s(BufferASN, _countof(BufferASN), 10, TEXT("%lu"), Response.ASN);

    _tprintf_s
    (
        LangGet(UIMSG_123_DATA_ASN_RECIVED_PRINT),

        Response.Header.Type,
        Response.Header.Length,
        BufferFlags, //Response.Flags
        (Response.Flags & ReplySharedFlagsDBOperational) ? TEXT("True") : TEXT("False"), // DB Operational
        (Response.Flags & ReplyASNFlagsFoundASN) ? BufferASN : LangGet(UIMSG_122_NOTFOUND) // Response.ASN
    );


function_end:
    return Result;
}

DWORD ServerPing()
{
    DWORD Result = EXIT_SUCCESS;
    TGeoIPSRVRequestPing Request = { 0 };
    TGeoIPSRVResponsePong Response;
    SYSTEMTIME systime;
    TCHAR szHour[3], szMinute[3], szSecond[3];


    _tprintf_s(LangGet(UIMSG_115_PING));

    Request.Header.Type = GeoIPSRVRequestPing;
    Request.Header.Length = sizeof(TGeoIPSRVRequestPing);

    if (!TransportRequest((PBYTE)&Request, Request.Header.Length, (PBYTE)&Response, sizeof(TGeoIPSRVResponsePong)))
        ExitFunction(EXIT_FAILURE);

    if (Response.Header.Type != GeoIPSRVReplyPong)
    {
        _tprintf_s(LangGet(UIMSG_114_BAD_SERVER_REPLY));
        ExitFunction(EXIT_FAILURE);
    }

    // Get the current local time
    GetLocalTime(&systime);

    // Convert the time values to strings
    _stprintf_s(szHour, 3, TEXT("%02hu"), systime.wHour);
    _stprintf_s(szMinute, 3, TEXT("%02hu"), systime.wMinute);
    _stprintf_s(szSecond, 3, TEXT("%02hu"), systime.wSecond);

    _tprintf_s(LangGet(UIMSG_120_PONG), szHour, szMinute, szSecond);


function_end:
    return Result;
}

DWORD DBHotplug()
{
    DWORD Result = EXIT_SUCCESS;
    TGeoIPSRVRequestHotplug Request = { 0 };
    TGeoIPSRVResponseHotplug Response;
    

    _tprintf_s(LangGet(UIMSG_116_DBHOTPLUG), GlobalOptions.FileGeo, GlobalOptions.FileASN);

    Request.Header.Type = GeoIPSRVRequestHotplug;
    Request.Header.Length = sizeof(TGeoIPSRVRequestHotplug);
    Request.Flags |= DBHotplugFlagsRestoreOnFailure;
    if (GlobalOptions.FileGeo)
        wcscpy_s((LPWSTR)&Request.Geo, DBFileLengthMaxCharWZero, GlobalOptions.FileGeo);
    if (GlobalOptions.FileASN)
        wcscpy_s((LPWSTR)&Request.ASN, DBFileLengthMaxCharWZero, GlobalOptions.FileASN);

    if (!TransportRequest((PBYTE)&Request, Request.Header.Length, (PBYTE)&Response, sizeof(TGeoIPSRVResponseHotplug)))
        ExitFunction(EXIT_FAILURE);

    if (Response.Header.Type != GeoIPSRVReplyHotplug)
    {
        _tprintf_s(LangGet(UIMSG_114_BAD_SERVER_REPLY));
        ExitFunction(EXIT_FAILURE);
    }

    _tprintf_s(LangGet(UIMSG_113_HOTPLUG_RESULT), Response.Result ? TEXT("True") : TEXT("False"));
    if (!Response.Result)
        Result = EXIT_FAILURE;


function_end:    
    return Result;
}

BOOL IPAddrConvert(LPCTSTR IPStr, PDWORD IPFamily, PBYTE IPBinary)
{
    BOOL Result = TRUE;
    int iResult;
    ADDRINFOW* Addrs = NULL;
    ADDRINFOW hints = 
    {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
        .ai_flags = AI_NUMERICHOST
    };
    PBYTE IPPtr;


    if (!WSAStartupLocal())
        ExitFunction(FALSE);

    // Call GetAddrInfo for conversion
    iResult = GetAddrInfo(IPStr, NULL, &hints, &Addrs);
    if (iResult) 
        ExitFunction(FALSE);

    // copy the array ob bytes
    *IPFamily = Addrs->ai_family;
    switch (Addrs->ai_family)
    {
    case AF_INET: // IPv4 address        
        IPPtr = (PBYTE) & (((struct sockaddr_in*)Addrs->ai_addr)->sin_addr);
        memcpy_s(IPBinary, 4, IPPtr, 4);
        
        break;

    case AF_INET6: // IPv6 address        
        IPPtr = (PBYTE) & (((struct sockaddr_in6*)Addrs->ai_addr)->sin6_addr);
        memcpy_s(IPBinary, 16, IPPtr, 16);
        
        break;

    default:
        ExitFunction(FALSE);
    }


function_end:
    FreeAddrInfo(Addrs);

    return Result;
}

BOOL TransportRequest(PBYTE Request, DWORD RequestSize, PBYTE Response, DWORD ResponseSize)
{
    BOOL CallResult = FALSE;


    switch (GlobalOptions.TransportToServer)
    {
    case TransportToServerPIPE:
        CallResult = PIPERequest(GlobalOptions.PipeName, Request, RequestSize, Response, ResponseSize);
        if (!CallResult)
            _tprintf_s(LangGet(UIMSG_106_ERROR_PIPE_IO));

        break;

    case TransportToServerTCP:
        CallResult = TCPRequest(GlobalOptions.NetServerAdress, GlobalOptions.NetServerPort, GlobalOptions.TCPVersion, Request, RequestSize, Response, ResponseSize);
        if (!CallResult)
            _tprintf_s(LangGet(UIMSG_107_ERROR_TCP_IO));

        break;
    }

    return CallResult;
}

BOOL PIPERequest(LPTSTR PipeName, PBYTE Request, DWORD RequestSize, PBYTE Response, DWORD ResponseSize)
{
    BOOL CallResult;
    DWORD BytesRead;


    _tprintf_s(LangGet(UIMSG_111_ESTAB_CONNECTION_PIPE), PipeName);

    // Connects to a message-type pipe (and waits if an instance of the pipe is not available),
    // writes to and reads from the pipe, and then closes the pipe
    CallResult = CallNamedPipe
    (
        PipeName,
        Request,
        RequestSize,
        Response,
        ResponseSize,
        &BytesRead,
        GlobalOptions.PIPEIOTimeout
    );

    if (!CallResult)
        return FALSE;

    if (BytesRead < ResponseSize)
        return FALSE;


    return TRUE;
}

BOOL TCPRequest(LPCTSTR TCPServer, LPCTSTR TCPPort, int TCPVersion, PBYTE Request, DWORD RequestSize, PBYTE Response, DWORD ResponseSize)
{
    BOOL Result = TRUE;
    ADDRINFOW hints = { 0 };
    ADDRINFOW* Addrs = NULL;
    ADDRINFOW* AddrCurrent;
    int CallResult;
    SOCKET ConnectinSoc = INVALID_SOCKET;
    TCHAR HostName[NI_MAXHOST];
    TCHAR PortNumber[NI_MAXSERV];


    if (!WSAStartupLocal())
        ExitFunction(FALSE);

    hints.ai_family = TCPVersion;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    CallResult = GetAddrInfo
    (
        TCPServer,
        TCPPort,
        &hints,
        &Addrs
    );
    if (CallResult)
        ExitFunction(FALSE);

    // Make sure we got at least one address
    if (!Addrs)
        ExitFunction(FALSE);

    // Walk through the list of addresses returned and try to connect to each one
    // Take the first successful connection
    AddrCurrent = Addrs;
    while (AddrCurrent)
    {
        CallResult = GetNameInfo
        (
            AddrCurrent->ai_addr,
            (socklen_t)AddrCurrent->ai_addrlen,
            HostName,
            NI_MAXHOST,
            PortNumber,
            NI_MAXSERV,
            NI_NUMERICHOST | NI_NUMERICSERV
        );
        if (CallResult)
            ExitFunction(FALSE);

        _tprintf_s(LangGet(UIMSG_109_ESTAB_CONNECTION), HostName, PortNumber);

        ConnectinSoc = socket(AddrCurrent->ai_family, AddrCurrent->ai_socktype, AddrCurrent->ai_protocol);
        if (ConnectinSoc == INVALID_SOCKET)
            ExitFunction(FALSE);

        // Set socket to non-blocking mode
        u_long mode = 1; // 0 for blocking, non-zero for non-blocking
        if (ioctlsocket(ConnectinSoc, FIONBIO, &mode))
            ExitFunction(FALSE);

        CallResult = connect(ConnectinSoc, AddrCurrent->ai_addr, (int)AddrCurrent->ai_addrlen);
        if (CallResult != SOCKET_ERROR)
            ExitFunction(FALSE);

        if (WSAGetLastError() != WSAEWOULDBLOCK)
            ExitFunction(FALSE);
            
        mode = 0; // 0 for blocking, non-zero for non-blocking
        if (ioctlsocket(ConnectinSoc, FIONBIO, &mode))
            ExitFunction(FALSE);

        fd_set Write, Err;
        FD_ZERO(&Write);
        FD_ZERO(&Err);
        FD_SET(ConnectinSoc, &Write);
        FD_SET(ConnectinSoc, &Err);
        TIMEVAL Timeout;
        Timeout.tv_sec = GlobalOptions.TCPConnectTimeoutSec;
        Timeout.tv_usec = 0;

        // check if the socket is ready
        CallResult = select(0, NULL, &Write, &Err, &Timeout);
        if (CallResult <= 0)
            ExitFunction(FALSE);

        if (FD_ISSET(ConnectinSoc, &Write))
            break; // connected, leave the loop

        closesocket(ConnectinSoc);
        ConnectinSoc = INVALID_SOCKET;
        AddrCurrent = AddrCurrent->ai_next;
    }
    CLIWriteLN();

    // Make sure we got a connection established
    if (ConnectinSoc == INVALID_SOCKET)
        ExitFunction(FALSE);
    
    _tprintf_s(LangGet(UIMSG_110_CONNECTED_OK));

    // set IO timeouts
    CallResult = setsockopt
    (
        ConnectinSoc,
        SOL_SOCKET,
        SO_RCVTIMEO,
        (const char*)&GlobalOptions.TCPIOTimeoutMSec,
        sizeof(GlobalOptions.TCPIOTimeoutMSec)
    );
    if (CallResult) /// If no error occurs, setsockopt returns zero
        ExitFunction(FALSE);

    CallResult = setsockopt
    (
        ConnectinSoc,
        SOL_SOCKET,
        SO_SNDTIMEO,
        (const char*)&GlobalOptions.TCPIOTimeoutMSec,
        sizeof(GlobalOptions.TCPIOTimeoutMSec)
    );
    if (CallResult) /// If no error occurs, setsockopt returns zero
        ExitFunction(FALSE);

    // Send the request
    CallResult = send(ConnectinSoc, Request, RequestSize, 0);
    if (CallResult == SOCKET_ERROR)
        ExitFunction(FALSE);

    // Receive the data back
    CallResult = recv(ConnectinSoc, (char*)Response, ResponseSize, 0);
    if (CallResult == SOCKET_ERROR)
        ExitFunction(FALSE);

    // CallResult are BytesRead here
    if (ResponseSize > (DWORD)CallResult)
        ExitFunction(FALSE);


function_end:
    // clean up the client connection
    if (ConnectinSoc != INVALID_SOCKET)
    {
        // Indicate no more data to send
        shutdown(ConnectinSoc, SD_SEND);
        // Close the socket
        closesocket(ConnectinSoc); 
    }

    if (Addrs)
        FreeAddrInfo(Addrs);

    return Result;
}

BOOL WSAStartupLocal()
{
    int CallResult;
    WSADATA wsaData;


    if (WSAReady)
        return TRUE;

    CallResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (CallResult)
    {
        _tprintf_s(LangGet(UIMSG_104_ERROR_WSA_STARTUP), CallResult);
        return FALSE;
    }

    WSAReady = TRUE;
    return TRUE;
}
