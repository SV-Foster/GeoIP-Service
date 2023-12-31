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
#include <utility>
#include <cassert>
#include "SharedHeaders.h"
#include "SharedOptionsDefaults.h"
#include "maxminddb.h"
#include "Protocol.h"
#include "DatabaseManager.h"
#include "WorkerThreadsManager.h"
#include "BaseServer.h"
#include "PIPEServer.h"


TPIPEServer::TPIPEServer
(    
    TDatabaseManager* CONST Database,
    CONST HANDLE StopEvent,
    IDBFilesConfigUpdater* CU,
    CONST std::wstring& PipeName,
    CONST DWORD SecurityAttributesPolicy, 
    CONST std::wstring& StringSecurityDescriptor,
    CONST DWORD tio
):
    TBaseServer::TBaseServer(Database, CU), // inherited Create
    Log(),
    MainThreadHandle(0),
    MainThreadId(0),
    SecDescr(),
    SecAttr({ 0 }),
    StopEvent(StopEvent),
    PipeName(PipeName),
    IOTimeoutMS(tio)
{
    // security descriptor
    switch (SecurityAttributesPolicy)
    {
    case SecurityAttributesPolicyUseCustom:
    {
        // create a security descriptor that allows anyone to write to the pipe
        SecDescr = std::make_unique<BYTE[]>(SECURITY_DESCRIPTOR_MIN_LENGTH); //malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);

        if (!InitializeSecurityDescriptor(SecDescr.get(), SECURITY_DESCRIPTOR_REVISION)) //reinterpret_cast<PSECURITY_DESCRIPTOR>()
            throw PIPEServerException(PIPESErrorSecurityDescriptorInit);

        SecAttr.nLength = sizeof(SecAttr);
        SecAttr.bInheritHandle = TRUE;
        SecAttr.lpSecurityDescriptor = SecDescr.get(); //reinterpret_cast<PSECURITY_DESCRIPTOR>()

        if (!TDACLGenerator::Generate(&(this->SecAttr), StringSecurityDescriptor))
            throw PIPEServerException(PIPESErrorSecurityDescriptorGenerate);

        break;
    }
    default: // SecurityAttributesPolicyStandard
        // The ACLs in the default security descriptor for a named pipe grant full control to the LocalSystem account, administrators, 
        // and the creator owner. They also grant read access to members of the Everyone group and the anonymous account
        break;

    }
}

TPIPEServer::~TPIPEServer() noexcept
{
    // wait for the main thread to stop
    if (this->MainThreadHandle)
    {
        WaitForSingleObject(this->MainThreadHandle, INFINITE);
        CloseHandle(this->MainThreadHandle);
    }
}

BOOL TPIPEServer::StartMainThread() noexcept
{
    // create main thread
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
        return FALSE; // PIPESErrorCreateThreadMain
    
    return TRUE;
}

DWORD WINAPI TPIPEServer::MainThread(LPVOID lpvParam)
{
    // get class back
    assert(lpvParam != nullptr);
    TPIPEServer* CONST ths = static_cast<TPIPEServer*>(lpvParam);


    DWORD Result = EXIT_SUCCESS;
    constexpr DWORD WaitEventsCnt = 2;
    HANDLE WaitEvents[2] = { ths->StopEvent, NULL };
    HANDLE NamedPipeHandle = INVALID_HANDLE_VALUE;
    OVERLAPPED OVL;
    DWORD WaitResult;
    HANDLE ThreadHandle;
    WorkerThreadsManager TMgr;
    DWORD ThreadId;
    DWORD LastError;
    BOOL CallResult;
    LPSECURITY_ATTRIBUTES PSecAttr = NULL;    
    if (ths->SecAttr.nLength) // set security descriptor pointer, if data presents
        PSecAttr = &(ths->SecAttr);


    if (ths->Log)
        ths->Log->OnMainThreadStart(ths->PipeName);

    // create the event object to use in overlapped i/o    
    WaitEvents[1] = CreateEvent
    (
        NULL,    // no security attributes
        TRUE,    // manual reset event
        FALSE,   // not-signalled
        NULL     // no name
    );
    if (!WaitEvents[1]) // If the function fails, the return value is NULL. To get extended error information, call GetLastError
        ExitFunction(PIPESErrorMainThreadCreateWaitEvents);

    while (TRUE)
    {
        // create the named pipe
        NamedPipeHandle = CreateNamedPipe
        (
            ths->PipeName.c_str(),     // the name of the pipe
            FILE_FLAG_OVERLAPPED |     
             PIPE_ACCESS_DUPLEX,       // The pipe is bi-directional; both server and client processes can read from and write to the pipe
            PIPE_TYPE_MESSAGE |        // Data is written to the pipe as a stream of messages
             PIPE_READMODE_MESSAGE |   // Data is read from the pipe as a stream of messages
             PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,  // the number of pipe instances that can be created is limited only by the availability of system resources
            0,                         // size of outbuf (0 == allocate as necessary)
            0,                         // size of inbuf
            ths->IOTimeoutMS,          // the default time-out value if the WaitNamedPipe() specifies NMPWAIT_USE_DEFAULT_WAIT, A value of zero will result in a default time-out of 50 milliseconds
            PSecAttr                   // security attributes
        );
        if (NamedPipeHandle == INVALID_HANDLE_VALUE) // If the function fails, the return value is INVALID_HANDLE_VALUE
            ExitFunction(PIPESErrorMainThreadCreateNamedPipe);

        // init the overlapped structure
        ZeroMemory(&OVL, sizeof(OVL));
        OVL.hEvent = WaitEvents[1];
        ResetEvent(WaitEvents[1]);

        // wait for a connection...
        // If the operation is asynchronous, ConnectNamedPipe returns immediately. If the operation is still pending
        // the return value is zero and GetLastError returns ERROR_IO_PENDING
        CallResult = ConnectNamedPipe(NamedPipeHandle, &OVL);
        if (!CallResult)
        {
            LastError = GetLastError();
            switch (LastError)
            {
            case ERROR_IO_PENDING:
                WaitResult = WaitForMultipleObjects(WaitEventsCnt, &WaitEvents[0], FALSE, INFINITE);

                if (WaitResult == WAIT_FAILED)
                    ExitFunction(PIPESErrorMainThreadConnectNamedPipeWait);

                if (WaitResult != WAIT_OBJECT_0 + 1) // not overlapped I/O event - error occurred, or server stop signaled
                    ExitFunction(EXIT_SUCCESS);

                break;

            default:
                ExitFunction(PIPESErrorMainThreadConnectNamedPipe);
            }
        }

        {
            // Create an object to pass data to the new thread
            // this object should be destroyed inside the thread or if failed to start one, at the end of the current scope
            std::unique_ptr<TWorkerThreadData> WrkData = std::make_unique<TWorkerThreadData>(ths, NamedPipeHandle);

            // Create a thread for this client
            ThreadHandle = CreateThread
            (
                NULL,              // no security attribute 
                ThreadStackSizeDefault, // default stack size 
                WorkerThread,      // thread proc
                WrkData.get(),     // thread parameter 
                0,                 // not suspended 
                &ThreadId          // returns thread ID 
            );
            if (!ThreadHandle)
                ExitFunction(PIPESErrorCreateThreadWorker);

            // Releases ownership of its stored pointer, by returning its value and replacing it with a null pointer
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

    if (WaitEvents[1])
        CloseHandle(WaitEvents[1]);

    if (ths->Log)
        ths->Log->OnMainThreadExit(ths->PipeName, Result, ths->StopEvent);
    return Result;
}

// This routine is a thread processing function to read from and reply to a client
// via the open pipe connection passed from the main loop. Note this allows
// the main loop to continue executing, potentially creating more threads of
// of this procedure to run concurrently, depending on the number of incoming
// client connections
DWORD WINAPI TPIPEServer::WorkerThread(LPVOID lpvParam)
{
    // get data back
    assert(lpvParam != nullptr);
    std::unique_ptr<TWorkerThreadData> WrkData;
    WrkData.reset(static_cast<TWorkerThreadData*>(lpvParam));    
    TPIPEServer* ths = WrkData->th;


    DWORD Result = EXIT_SUCCESS;
    BOOL CallResult;
    constexpr DWORD WaitEventsCnt = 2;
    HANDLE WaitEvents[2] = { ths->StopEvent, NULL };
    BYTE BufferRequest[DefaultBufferSize];
    BYTE BufferResponse[DefaultBufferSize];
    DWORD ResponseDataSize;
    BOOL Disconnect;


    // Log
    if (ths->Log)
        ths->Log->OnWorkerThreadStart(ths->PipeName);

    // Do some extra error checking since the app will keep running even if this thread fails
    if (!WrkData->IsHandleValid())
        ExitFunction(PIPESErrorThreadWorkerMisconfig);

    // create the event object object use in overlapped i/o
    WaitEvents[1] = CreateEvent
    (
        NULL,    // no security attributes
        TRUE,    // manual reset event
        FALSE,   // not-signalled
        NULL     // no name
    );
    if (WaitEvents[1] == NULL)
        ExitFunction(PIPESErrorThreadWorkerCreateEvent);


    // Loop until disconnect, shutdown or error
    while (TRUE)
    {
        // read request
        CallResult = MessageRecieve
        (
            WaitEventsCnt,
            &WaitEvents[0],
            &BufferRequest[0],
            DefaultBufferSize,
            WrkData->Pipe,
            &Disconnect
        );
        if (!CallResult)
            ExitFunction(PIPESErrorRecieve);

        if (Disconnect)
            ExitFunction(EXIT_SUCCESS);

        // process command
        CallResult = ths->RequestProcess
        (
            &BufferRequest[0], 
            DefaultBufferSize, 
            &BufferResponse[0], 
            DefaultBufferSize, 
            &ResponseDataSize
        );
        if (!CallResult)
            ExitFunction(PIPESErrorProcess);

        // send response
        CallResult = MessageSend
        (
            WaitEventsCnt,
            WaitEvents,
            BufferResponse,
            ResponseDataSize,
            WrkData->Pipe, 
            &Disconnect
        );
        if (!CallResult)
            ExitFunction(PIPESErrorSend);

        if (Disconnect)
            ExitFunction(EXIT_SUCCESS);
    }


function_end:
    if (ths->Log)
        ths->Log->OnWorkerThreadExit(ths->PipeName, Result, ths->StopEvent);

    if (WrkData->Pipe)
    {
        // Flush the pipe to allow the client to read the pipe's contents before disconnecting. Then disconnect the pipe, and close the
        // handle to this pipe instance
        FlushFileBuffers(WrkData->Pipe);
        DisconnectNamedPipe(WrkData->Pipe);
        //CloseHandle(WrkData->Pipe); // performed automatically by the std::unique_ptr<TWorkerThreadData> WrkData object
    }

    // overlapped i/o event
    if (WaitEvents[1])
        CloseHandle(WaitEvents[1]);    

    return Result;
}

BOOL TPIPEServer::MessageRecieve(DWORD WaitEventsCnt, HANDLE WaitEvents[], PBYTE BufferRequest, DWORD BufferRequestSize, HANDLE Pipe, PBOOL Disconnect) noexcept
{
    *Disconnect = FALSE;
    OVERLAPPED OVL;
    BOOL CallResult;
    DWORD LastError;
    DWORD WaitResult;


    while (TRUE)
    {
        // init the overlapped structure
        ZeroMemory(&OVL, sizeof(OVL));
        OVL.hEvent = WaitEvents[1];
        ResetEvent(WaitEvents[1]);

        // grab whatever's coming through the pipe...
        CallResult = ReadFile
        (
            Pipe,
            BufferRequest,
            BufferRequestSize,
            NULL,
            &OVL
        );

        if (!CallResult)
        {
            LastError = GetLastError();
            switch (LastError)
            {
            case ERROR_IO_PENDING:
                WaitResult = WaitForMultipleObjects(WaitEventsCnt, WaitEvents, FALSE, INFINITE);

                if (WaitResult == WAIT_FAILED)
                    return FALSE;

                if (WaitResult != WAIT_OBJECT_0 + 1) // not overlapped i/o event - error occurred, or server stop signaled
                    return FALSE;

                continue;

            case ERROR_SUCCESS:
                return TRUE;

            case ERROR_BROKEN_PIPE:
                *Disconnect = TRUE;
                return TRUE;

            default:
                return FALSE;
            }
        }

        return TRUE;
    }
}

BOOL TPIPEServer::MessageSend(DWORD WaitEventsCnt, HANDLE WaitEvents[], CONST PBYTE BufferResponse, DWORD BufferResponseSize, HANDLE Pipe, PBOOL Disconnect) noexcept
{
    *Disconnect = FALSE;
    OVERLAPPED OVL;
    BOOL CallResult;
    DWORD LastError;
    DWORD WaitResult;


    while (TRUE)
    {
        // init the overlapped structure
        ZeroMemory(&OVL, sizeof(OVL));
        OVL.hEvent = WaitEvents[1];
        ResetEvent(WaitEvents[1]);

        // send it back out...
        CallResult = WriteFile
        (
            Pipe,
            BufferResponse,
            BufferResponseSize,
            NULL,
            &OVL
        );

        if (!CallResult)
        {
            LastError = GetLastError();
            switch (LastError)
            {
            case ERROR_IO_PENDING:
                WaitResult = WaitForMultipleObjects(WaitEventsCnt, WaitEvents, FALSE, INFINITE);

                if (WaitResult == WAIT_FAILED)
                    return FALSE;

                if (WaitResult != WAIT_OBJECT_0 + 1) // not overlapped i/o event - error occurred, or server stop signaled
                    return FALSE;

                continue;

            case ERROR_SUCCESS:
                return TRUE;

            case ERROR_NO_DATA:
                *Disconnect = TRUE;
                return TRUE;

            default:
                return FALSE;
            }
        }

        return TRUE;
    }
}

// R/O Getters
DWORD TPIPEServer::MainThreadIdGet() noexcept { return this->MainThreadId; };
CONST DWORD TPIPEServer::MainThreadIdGet() const noexcept { return this->MainThreadId; };
std::wstring& TPIPEServer::PipeNameGet() noexcept { return this->PipeName; };
CONST std::wstring& TPIPEServer::PipeNameGet() const noexcept { return this->PipeName; };

TPIPEServer::TWorkerThreadData::TWorkerThreadData() noexcept:
    th(nullptr),
    Pipe(INVALID_HANDLE_VALUE)
{
    return; // NOP
};

TPIPEServer::TWorkerThreadData::TWorkerThreadData(TPIPEServer* t, HANDLE p) noexcept:
    th(t),
    Pipe(p)
{
    return; // NOP
};

TPIPEServer::TWorkerThreadData::~TWorkerThreadData() noexcept
{
    if (this->IsHandleValid())
        CloseHandle(this->Pipe);
}

BOOL TPIPEServer::TWorkerThreadData::IsHandleValid() noexcept
{
    return !((this->Pipe == NULL) || (this->Pipe == INVALID_HANDLE_VALUE));
}

PIPEServerException::PIPEServerException(DWORD Code) noexcept:
    Code(Code)
{
    return; // NOP
}

DWORD PIPEServerException::CodeGet() noexcept { return this->Code; };
CONST DWORD PIPEServerException::CodeGet() const noexcept { return this->Code; };


// Creates a security descriptor containing the
// desired DACL. This function uses SDDL to make Deny and Allow ACEs.
//
// Parameter:
//     SECURITY_ATTRIBUTES * pSA
// Address to a SECURITY_ATTRIBUTES structure. It is the caller's
// responsibility to properly initialize the structure, and to free 
// the structure's lpSecurityDescriptor member when done (by calling
// the LocalFree function).
// 
// Return value:
//    FALSE if the address to the structure is NULL. 
//    Otherwise, this function returns the value from the
//    ConvertStringSecurityDescriptorToSecurityDescriptor function.
BOOL TDACLGenerator::Generate(SECURITY_ATTRIBUTES* pSA, CONST std::wstring& StringSecurityDescriptor) noexcept
{
    if (!pSA)
        return FALSE;

    return ConvertStringSecurityDescriptorToSecurityDescriptor
    (
        StringSecurityDescriptor.c_str(),
        SDDL_REVISION_1,
        &(pSA->lpSecurityDescriptor),
        NULL
    );
}
