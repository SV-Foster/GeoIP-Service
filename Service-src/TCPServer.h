/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


#define TCPSErrorWSAStartup 1
#define TCPSErrorSocketOpen 2
#define TCPSErrorIPAddrConvert 3
#define TCPSErrorIPFamilyMisconifg 4
#define TCPSErrorSocketBind 5
#define TCPSErrorSocketListen 6
#define TCPSErrorShutdownThreadCreate 7
#define TCPSErrorMainThreadCreate 8
#define TCPSErrorAccept 9
#define TCPSErrorWorkerThreadCreate 10
#define TCPSErrorWorkerThreadMisconfig 11
#define TCPSErrorSocketConfig 12
#define TCPSErrorRecve 13
#define TCPSErrorProcess 14
#define TCPSErrorSend 15


#define TCPIOTimeoutDefault 30000


class ITCPServerEvents
{
public:
    ITCPServerEvents() = default;
    virtual ~ITCPServerEvents() = default;
    ITCPServerEvents(const ITCPServerEvents& oth) = default;
    ITCPServerEvents(ITCPServerEvents&& oth) = default;
    ITCPServerEvents& operator=(const ITCPServerEvents& oth) = default;
    ITCPServerEvents& operator=(ITCPServerEvents&& oth) = default;


    virtual VOID OnMainThreadStart(std::wstring& IPAddr, WORD IPPort) const abstract;
    virtual VOID OnMainThreadExit(std::wstring& IPAddr, WORD IPPort, DWORD Result, HANDLE StopEvent) const abstract;

    virtual VOID OnShutdownThreadStart(std::wstring& IPAddr, WORD IPPort) const abstract;
    virtual VOID OnShutdownThreadExit(std::wstring& IPAddr, WORD IPPort, HANDLE StopEvent) const abstract;

    virtual VOID OnWorkerThreadStart(std::wstring& IPAddr, WORD IPPort) const abstract;
    virtual VOID OnWorkerThreadExit(std::wstring& IPAddr, WORD IPPort, DWORD Result, HANDLE StopEvent) const abstract;
};

class TCPServerException:
    public std::exception
{
public:
    TCPServerException(DWORD Code) noexcept;

    // R/O Getters
    DWORD CodeGet() noexcept;
    CONST DWORD CodeGet() const noexcept;

private:
    DWORD Code;
};

class TTCPServer:
    public TBaseServer
{
public:
    TTCPServer(TDatabaseManager* Database, HANDLE StopEvent, IDBFilesConfigUpdater* CU, ADDRESS_FAMILY IPFamily, std::wstring IPAddr, WORD IPPort, DWORD TimeoutR, DWORD TimeoutS);
    ~TTCPServer() noexcept;
    TTCPServer(const TTCPServer& oth) = delete;
    TTCPServer& operator=(const TTCPServer& oth) = delete;
    TTCPServer(TTCPServer&& oth) = delete;
    TTCPServer& operator=(TTCPServer&& oth) = delete;


    std::shared_ptr<ITCPServerEvents> Log;


    BOOL StartMainThread() noexcept;
    // R/O Getters
    DWORD MainThreadIdGet() noexcept;
    CONST DWORD MainThreadIdGet() const noexcept;
    DWORD ShutdownThreadIdGet() noexcept;
    CONST DWORD ShutdownThreadIdGet() const noexcept;
    ADDRESS_FAMILY IPFamilyGet() noexcept;
    CONST ADDRESS_FAMILY IPFamilyGet() const noexcept;
    std::wstring& IPAddrGet() noexcept;
    CONST std::wstring& IPAddrGet() const noexcept;
    WORD IPPortGet() noexcept;
    CONST WORD IPPortGet() const noexcept;


private:
    WSADATA wsaData;
    HANDLE StopEvent;
    SOCKET SocketListen;
    struct sockaddr_in serverAddr;
    struct sockaddr_in6 serverAddr6;
    PVOID PSA;
    int PSASize;
    HANDLE ShutdownThreadHandle;
    DWORD ShutdownThreadId;
    HANDLE MainThreadHandle;
    DWORD MainThreadId;
    ADDRESS_FAMILY IPFamily;
    std::wstring IPAddr;
    WORD IPPort;
    DWORD IOTimeoutReciveMS;
    DWORD IOTimeoutSendMS;


    BOOL MessageRecieve(PBYTE BufferRequest, DWORD BufferRequestSize, SOCKET clientSocket, PBOOL Disconnect) noexcept;

    static DWORD WINAPI MainThread(LPVOID lpvParam);
    static DWORD WINAPI ShutdownThread(LPVOID lpvParam);
    static DWORD WINAPI WorkerThread(LPVOID lpvParam);


    struct TWorkerThreadData
    {
        SOCKET clientSocket;
        TTCPServer* th;

        TWorkerThreadData() noexcept;
        TWorkerThreadData(SOCKET ck, TTCPServer* t) noexcept;
        ~TWorkerThreadData() noexcept;
        TWorkerThreadData(const TWorkerThreadData& oth) = delete;
        TWorkerThreadData(TWorkerThreadData&& oth) = delete;
        TWorkerThreadData& operator=(const TWorkerThreadData& oth) = delete;
        TWorkerThreadData& operator=(TWorkerThreadData&& oth) = delete;


        BOOL IsSocketValid() noexcept;
    };
};
