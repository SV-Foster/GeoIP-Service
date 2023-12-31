/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


#define SecurityAttributesPolicyStandard 0x00
#define SecurityAttributesPolicyUseCustom 0x01
#define TimeoutIODefaultMS 30000

#define PIPESErrorCreateThreadMain 1
#define PIPESErrorMainThreadCreateWaitEvents 2
#define PIPESErrorMainThreadCreateNamedPipe 3
#define PIPESErrorMainThreadConnectNamedPipe 4
#define PIPESErrorMainThreadConnectNamedPipeWait 5
#define PIPESErrorCreateThreadWorker 6
#define PIPESErrorThreadWorkerCreateEvent 7
#define PIPESErrorRecieve 8
#define PIPESErrorProcess 9
#define PIPESErrorSend 10
#define PIPESErrorSecurityDescriptorInit 11
#define PIPESErrorSecurityDescriptorGenerate 12
#define PIPESErrorThreadWorkerMisconfig 13


class PIPEServerException:
    public std::exception
{
public:
    PIPEServerException(DWORD Code) noexcept;
    
    // R/O Getters
    DWORD CodeGet() noexcept;
    CONST DWORD CodeGet() const noexcept;

private:
    DWORD Code;
};

class TDACLGenerator
{
public:
    TDACLGenerator() = delete;
    ~TDACLGenerator() = delete;
    TDACLGenerator(const TDACLGenerator& oth) = delete;
    TDACLGenerator& operator=(const TDACLGenerator& oth) = delete;
    TDACLGenerator(TDACLGenerator&& oth) = delete;
    TDACLGenerator& operator=(TDACLGenerator&& oth) = delete;

    static BOOL Generate(SECURITY_ATTRIBUTES* pSA, CONST std::wstring& StringSecurityDescriptor) noexcept;
};

class IPIPEServerEvents
{
public:
    IPIPEServerEvents() = default;
    virtual ~IPIPEServerEvents() = default;
    IPIPEServerEvents(const IPIPEServerEvents& oth) = delete;
    IPIPEServerEvents& operator=(const IPIPEServerEvents& oth) = delete;
    IPIPEServerEvents(IPIPEServerEvents&& oth) = delete;
    IPIPEServerEvents& operator=(IPIPEServerEvents&& oth) = delete;


    virtual VOID OnMainThreadStart(std::wstring& PIPEName) const abstract;
    virtual VOID OnMainThreadExit(std::wstring& PIPEName, DWORD Result, HANDLE StopEvent) const abstract;

    virtual VOID OnWorkerThreadStart(std::wstring& PIPEName) const abstract;
    virtual VOID OnWorkerThreadExit(std::wstring& PIPEName, DWORD Result, HANDLE StopEvent) const abstract;
};

class TPIPEServer:
    public TBaseServer
{
public:
    std::shared_ptr<IPIPEServerEvents> Log;


    TPIPEServer(TDatabaseManager* CONST Database, CONST HANDLE StopEvent, IDBFilesConfigUpdater* CU, CONST std::wstring& PipeName, CONST DWORD SecurityAttributesPolicy, CONST std::wstring& StringSecurityDescriptor, CONST DWORD tio);
    ~TPIPEServer() noexcept;
    TPIPEServer(const TPIPEServer& oth) = delete;
    TPIPEServer& operator=(const TPIPEServer& oth) = delete;
    TPIPEServer(TPIPEServer&& oth) = delete;
    TPIPEServer& operator=(TPIPEServer&& oth) = delete;

    BOOL StartMainThread() noexcept;
    // R/O Getters
    DWORD MainThreadIdGet() noexcept;
    CONST DWORD MainThreadIdGet() const noexcept;
    std::wstring& PipeNameGet() noexcept;
    CONST std::wstring& PipeNameGet() const noexcept;


private:
    HANDLE MainThreadHandle;
    DWORD MainThreadId;
    std::unique_ptr<BYTE[]> SecDescr;
    SECURITY_ATTRIBUTES SecAttr;
    HANDLE StopEvent;
    std::wstring PipeName;
    DWORD IOTimeoutMS;


    static DWORD WINAPI MainThread(LPVOID lpvParam);
    static DWORD WINAPI WorkerThread(LPVOID lpvParam);
    static BOOL MessageRecieve(DWORD WaitEventsCnt, HANDLE WaitEvents[], PBYTE BufferRequest, DWORD BufferRequestSize, HANDLE Pipe, PBOOL Disconnect) noexcept;
    static BOOL MessageSend(DWORD WaitEventsCnt, HANDLE WaitEvents[], CONST PBYTE BufferResponse, DWORD BufferResponseSize, HANDLE Pipe, PBOOL Disconnect) noexcept;


    struct TWorkerThreadData
    {
        TPIPEServer* th;
        HANDLE Pipe;


        TWorkerThreadData() noexcept;
        TWorkerThreadData(TPIPEServer* t, HANDLE p) noexcept;
        ~TWorkerThreadData() noexcept;
        TWorkerThreadData(const TWorkerThreadData& oth) = delete;
        TWorkerThreadData(TWorkerThreadData&& oth) = delete;
        TWorkerThreadData& operator=(const TWorkerThreadData& oth) = delete;
        TWorkerThreadData& operator=(TWorkerThreadData&& oth) = delete;


        BOOL IsHandleValid() noexcept;
    };
};
