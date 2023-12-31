/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#define _WINSOCKAPI_
#include <windows.h>
#include <tchar.h>
#include <sddl.h>
#include <string>
#include <locale>
#include <vector>
#include <cassert>
#include "SharedHeaders.h"
#include "SharedOptionsDefaults.h"
#include "maxminddb.h"
#include "Protocol.h"
#include "DatabaseManager.h"
#include "StringCoverter.h"
#include "WorkerThreadsManager.h"
#include "BaseServer.h"
#include "TCPServer.h"

#pragma comment(lib, "ws2_32.lib")


TTCPServer::TTCPServer
(
    TDatabaseManager* Database, 
    HANDLE StopEvent, 
    IDBFilesConfigUpdater* CU, 
    ADDRESS_FAMILY IPFamily, 
    std::wstring IPAddr, 
    WORD IPPort, 
    DWORD TimeoutR, 
    DWORD TimeoutS
):
    TBaseServer::TBaseServer(Database, CU), // inherited Create    
    Log(),
    wsaData({}),
    StopEvent(StopEvent),
    SocketListen(INVALID_SOCKET),
    serverAddr({ 0 }),
    serverAddr6({ 0 }),
    PSA(nullptr),
    PSASize(0),
    ShutdownThreadHandle(0),
    ShutdownThreadId(0),
    MainThreadHandle(0),
    MainThreadId(0),
    IPFamily(IPFamily),
    IPAddr(IPAddr),
    IPPort(IPPort),
    IOTimeoutReciveMS(TimeoutR),
    IOTimeoutSendMS(TimeoutS)
{
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &this->wsaData)) 
        throw TCPServerException(TCPSErrorWSAStartup);

    // Create a socket for listening
    this->SocketListen = socket(IPFamily, SOCK_STREAM, IPPROTO_TCP);
    if (this->SocketListen == INVALID_SOCKET)
        throw TCPServerException(TCPSErrorSocketOpen);

    // Setup server address structure
    switch (IPFamily)
    {
    case AF_INET:
        serverAddr.sin_family = IPFamily;
        serverAddr.sin_port = htons(IPPort);

        // converts an IPv4 or IPv6 Internet network address in its standard text presentation form into its numeric binary form
        if (!InetPton(IPFamily, IPAddr.c_str(), &serverAddr.sin_addr.s_addr))
            throw TCPServerException(TCPSErrorIPAddrConvert);

        PSA = &serverAddr;
        PSASize = sizeof(serverAddr);

        break;

    case AF_INET6:
        serverAddr6.sin6_family = IPFamily;
        serverAddr6.sin6_port = htons(IPPort);

        if (!InetPton(IPFamily, IPAddr.c_str(), &serverAddr6.sin6_addr.u))
            throw TCPServerException(TCPSErrorIPAddrConvert);

        PSA = &serverAddr6;
        PSASize = sizeof(serverAddr6);

        break;

    default:
        throw TCPServerException(TCPSErrorIPFamilyMisconifg);
    }
   
    // asociates a local address with a socket
    if (bind(this->SocketListen, static_cast<struct sockaddr*>(PSA), PSASize))
        throw TCPServerException(TCPSErrorSocketBind);

}

TTCPServer::~TTCPServer() noexcept
{
    // wait for main and shutdown threads to stop
    if (this->MainThreadHandle)
    {
        WaitForSingleObject(this->MainThreadHandle, INFINITE);
        CloseHandle(this->MainThreadHandle);
    }

    if (this->ShutdownThreadHandle)
    {
        WaitForSingleObject(this->ShutdownThreadHandle, INFINITE);
        CloseHandle(this->ShutdownThreadHandle);
    }

    closesocket(this->SocketListen);
    WSACleanup();
}

BOOL TTCPServer::StartMainThread() noexcept
{
    // start shutdown thred
    this->ShutdownThreadHandle = CreateThread
    (
        NULL,
        ThreadStackSizeDefault,
        ShutdownThread,
        this,
        0,
        &this->ShutdownThreadId
    );
    if (!this->ShutdownThreadHandle) // If the function fails, the return value is NULL
        return FALSE; // TCPSErrorShutdownThreadCreate

    // start main thread
    this->MainThreadHandle = CreateThread
    (
        NULL,                   // no security attribute 
        ThreadStackSizeDefault, // default stack size 
        MainThread,             // thread proc
        this,                   // thread parameter 
        0,                      // not suspended 
        &this->MainThreadId     // returns thread ID 
    );
    if (!this->MainThreadHandle) // If the function fails, the return value is NULL
        return FALSE; // TCPSErrorMainThreadCreate

    return TRUE;
}

DWORD WINAPI TTCPServer::MainThread(LPVOID lpvParam)
{
    // get class back
    assert(lpvParam != nullptr);
    TTCPServer* CONST ths = static_cast<TTCPServer*>(lpvParam);


    DWORD Result = EXIT_SUCCESS;
    SOCKET clientSocket;
    HANDLE ThreadHandle;
    WorkerThreadsManager TMgr;
    constexpr int addrLen = sizeof(struct sockaddr_in);
    int LastError;

    
    if (ths->Log)
        ths->Log->OnMainThreadStart(ths->IPAddr, ths->IPPort);

    // places a socket in a state in which it is listening for an incoming connection
    if (listen(ths->SocketListen, FD_SETSIZE))
        ExitFunction(TCPSErrorSocketListen);

    // Accept incoming connections in a loop
    while (TRUE)
    {
        // permits an incoming connection attempt on a socket
        clientSocket = accept(ths->SocketListen, static_cast<struct sockaddr*>(ths->PSA), &ths->PSASize);
        if (clientSocket == INVALID_SOCKET)
        {
            LastError = WSAGetLastError();
            if (LastError == WSAEINTR)
                ExitFunction(EXIT_SUCCESS); // shutdown

            ExitFunction(TCPSErrorAccept);
        }

        {
            // Create an object to pass data to the new thread
            // this object should be destroyed inside the thread
            std::unique_ptr<TWorkerThreadData> WrkData = std::make_unique<TWorkerThreadData>(clientSocket, ths);

            // Start a new thread to handle the client
            ThreadHandle = CreateThread
            (
                NULL,
                ThreadStackSizeDefault,
                WorkerThread,
                WrkData.get(),
                0,
                NULL
            );
            if (!ThreadHandle)
                ExitFunction(TCPSErrorWorkerThreadCreate);

            // Releases ownership of its stored pointer, by returning its value and replacing it with a nullptr
            WrkData.release();
        }

        // remove handles of finished threads
        TMgr.RemoveFinished();
        // and add new one
        TMgr.AddNew(ThreadHandle);
    }
    

function_end:
    // wait for all working threads to finish
    TMgr.WaitAllToStop();

    if (ths->Log)
        ths->Log->OnMainThreadExit(ths->IPAddr, ths->IPPort, Result, ths->StopEvent);
    return Result;
}

DWORD WINAPI TTCPServer::ShutdownThread(LPVOID lpvParam)
{
    // get class back
    assert(lpvParam != nullptr);
    TTCPServer* CONST ths = static_cast<TTCPServer*>(lpvParam);


    SOCKET sockTemp = INVALID_SOCKET;


    if (ths->Log)
        ths->Log->OnShutdownThreadStart(ths->IPAddr, ths->IPPort);
    WaitForSingleObject(ths->StopEvent, INFINITE);

    if (ths->Log)
        ths->Log->OnShutdownThreadExit(ths->IPAddr, ths->IPPort, ths->StopEvent);
    // We want to make closesocket the last call in the handler because it will
    // cause the WSAAccept to return in the main thread
    sockTemp = ths->SocketListen;
    ths->SocketListen = INVALID_SOCKET;
    closesocket(sockTemp);

    return EXIT_SUCCESS;
}

DWORD WINAPI TTCPServer::WorkerThread(LPVOID lpvParam)
{
    // get data back
    assert(lpvParam != nullptr);
    std::unique_ptr<TWorkerThreadData> WrkData;
    WrkData.reset(static_cast<TWorkerThreadData*>(lpvParam));
    assert(WrkData->th != nullptr);
    TTCPServer* ths = WrkData->th;


    DWORD Result = EXIT_SUCCESS;
    BOOL CallResult;
    BYTE BufferRequest[DefaultBufferSize];
    BYTE BufferResponse[DefaultBufferSize];
    DWORD ResponseDataSize;
    int bytesSent;
    BOOL Disconnect;

    
    if (ths->Log)
        ths->Log->OnWorkerThreadStart(ths->IPAddr, ths->IPPort);


    if (!WrkData->IsSocketValid())
        ExitFunction(TCPSErrorWorkerThreadMisconfig);

    // set IO timeouts
    CallResult = setsockopt
    (
        WrkData->clientSocket,
        SOL_SOCKET,
        SO_RCVTIMEO,
        reinterpret_cast<const char*>(&ths->IOTimeoutReciveMS),
        sizeof(ths->IOTimeoutReciveMS)
    );
    if (CallResult) /// If no error occurs, setsockopt returns zero
        ExitFunction(TCPSErrorSocketConfig);

    CallResult = setsockopt
    (
        WrkData->clientSocket,
        SOL_SOCKET,
        SO_SNDTIMEO,
        reinterpret_cast<const char*>(&ths->IOTimeoutSendMS),
        sizeof(ths->IOTimeoutSendMS)
    );
    if (CallResult) /// If no error occurs, setsockopt returns zero
        ExitFunction(TCPSErrorSocketConfig);

    while (TRUE) 
    {
        // read request
        CallResult = ths->MessageRecieve
        (
            &BufferRequest[0],
            DefaultBufferSize,
            WrkData->clientSocket,
            &Disconnect
        );
        if (!CallResult)
            ExitFunction(TCPSErrorRecve);

        if (Disconnect)
            ExitFunction(EXIT_SUCCESS);

        // process command
        CallResult = ths->RequestProcess(&BufferRequest[0], DefaultBufferSize, &BufferResponse[0], DefaultBufferSize, &ResponseDataSize);
        if (!CallResult)
            ExitFunction(TCPSErrorProcess);

        // send data to the client
        bytesSent = send(WrkData->clientSocket, reinterpret_cast<char*>(&BufferResponse[0]), ResponseDataSize, 0);
        if (bytesSent == SOCKET_ERROR)
            ExitFunction(TCPSErrorSend);
    }


function_end:
    if (ths->Log)
        ths->Log->OnWorkerThreadExit(ths->IPAddr, ths->IPPort, Result, ths->StopEvent);

    // socket is closed automatically by std::unique_ptr<TWorkerThreadData> WrkData destructor

    return Result;
}

BOOL TTCPServer::MessageRecieve(PBYTE BufferRequest, DWORD BufferRequestSize, SOCKET clientSocket, PBOOL Disconnect) noexcept
{
    assert(sizeof(TGeoIPSRVSharedHeader) <= BufferRequestSize);

    *Disconnect = FALSE;
    int bytesRead;
    PBYTE BufferPtr = BufferRequest;
    DWORD BufferRecTotal = 0;
    PCGeoIPSRVSharedHeader Hdr = reinterpret_cast<PCGeoIPSRVSharedHeader>(BufferRequest);


    for(;;) // read data from the network 'till whole message is saved into the buffer
    {
        bytesRead = recv(clientSocket, reinterpret_cast<char*>(BufferPtr), (BufferRequestSize - BufferRecTotal), 0);
        if (bytesRead == SOCKET_ERROR)
            return FALSE;

        if (!bytesRead) // If the connection has been gracefully closed, the return value is zero
        {
            *Disconnect = TRUE;
            return TRUE;
        }

        BufferRecTotal += bytesRead;
        BufferPtr += bytesRead;

        // check buffer is full
        if (BufferRecTotal >= BufferRequestSize)
            return (BufferRequestSize >= Hdr->Length);

        // check at least the header is recieved
        if (BufferRecTotal < sizeof(TGeoIPSRVSharedHeader))
            continue;

        // check the header is valid
        if (BufferRequestSize < Hdr->Length)
            return FALSE; // messge is too big

        // whole message is NOT recieved yet, coutinue reading
        if (BufferRecTotal < Hdr->Length)
            continue;

        // done!
        break;
    };

    return TRUE;
}

// R/O Getters
DWORD TTCPServer::MainThreadIdGet() noexcept { return this->MainThreadId; };
CONST DWORD TTCPServer::MainThreadIdGet() const noexcept { return this->MainThreadId; };
DWORD TTCPServer::ShutdownThreadIdGet() noexcept { return this->ShutdownThreadId; };
CONST DWORD TTCPServer::ShutdownThreadIdGet() const noexcept { return this->ShutdownThreadId; };
ADDRESS_FAMILY TTCPServer::IPFamilyGet() noexcept { return this->IPFamily; };
CONST ADDRESS_FAMILY TTCPServer::IPFamilyGet() const noexcept { return this->IPFamily; };
std::wstring& TTCPServer::IPAddrGet() noexcept { return this->IPAddr; };
CONST std::wstring& TTCPServer::IPAddrGet() const noexcept { return this->IPAddr; };
WORD TTCPServer::IPPortGet() noexcept { return this->IPPort; };
CONST WORD TTCPServer::IPPortGet() const noexcept { return this->IPPort; };

TTCPServer::TWorkerThreadData::TWorkerThreadData() noexcept:
    clientSocket(INVALID_SOCKET),
    th(nullptr)
{
    return; // NOP
};

TTCPServer::TWorkerThreadData::TWorkerThreadData(SOCKET ck, TTCPServer* t) noexcept:
    clientSocket(ck),
    th(t)
{
    return; // NOP
};

TTCPServer::TWorkerThreadData::~TWorkerThreadData() noexcept
{
    if (this->IsSocketValid())
        closesocket(this->clientSocket);
}

BOOL TTCPServer::TWorkerThreadData::IsSocketValid() noexcept
{
    return (this->clientSocket != INVALID_SOCKET);
}


TCPServerException::TCPServerException(DWORD Code) noexcept :
    Code(Code)
{
    return; // NOP
}

DWORD TCPServerException::CodeGet() noexcept { return this->Code; };
CONST DWORD TCPServerException::CodeGet() const noexcept { return this->Code; };
