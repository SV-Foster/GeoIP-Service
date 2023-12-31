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
#include "SharedHeaders.h"
#include "SharedOptionsDefaults.h"
#include "maxminddb.h"
#include "GlobalVariables.h"
#include "Protocol.h"
#include "BaseServer.h"
#include "PIPEServer.h"

#pragma warning (disable : 6255)


extern TGlobalVariables GlobalVariables;

static HANDLE MainThreadWaitEvent = NULL;
static PSECURITY_DESCRIPTOR SecDescr = NULL;
static SECURITY_ATTRIBUTES SecAttr;

BOOL PIPEInit()
{
    BOOL Result = TRUE;


    _tprintf(TEXT("PipeInit started\n"));

    // create the event object to use in overlapped i/o    
    MainThreadWaitEvent = CreateEvent
    (
        NULL,    // no security attributes
        TRUE,    // manual reset event
        FALSE,   // not-signalled
        NULL     // no name
    );
    if (MainThreadWaitEvent == NULL)
        ExitFunction(FALSE);

    // create a security descriptor that allows anyone to write to the pipe
    SecDescr = malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (SecDescr == NULL)
        ExitFunction(FALSE);

    if (!InitializeSecurityDescriptor(SecDescr, SECURITY_DESCRIPTOR_REVISION))
        ExitFunction(FALSE);

    SecAttr.nLength = sizeof(SecAttr);
    SecAttr.bInheritHandle = TRUE;
    SecAttr.lpSecurityDescriptor = SecDescr;
    if (!CreateMyDACL(&SecAttr))
        ExitFunction(FALSE);

function_end:
    _tprintf(TEXT("PipeInit exiting with result %d\n"), Result);
    return Result;
}

BOOL PIPECleanup()
{
    _tprintf(TEXT("PIPECleanup started\n"));

    if (MainThreadWaitEvent)
        CloseHandle(MainThreadWaitEvent);

    free(SecDescr);

    return TRUE;
}

DWORD WINAPI PIPEMainThread(LPVOID lpvParam)
{
    DWORD Result = EXIT_SUCCESS;
    CONST DWORD WaitEventsCnt = 2;
    CONST HANDLE WaitEvents[2] = { GlobalVariables.SvcStopEvent, MainThreadWaitEvent };
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    OVERLAPPED OVL;
    LPTSTR lpszPipeName = DefaultPipeName;
    DWORD WaitResult;
    HANDLE ThreadHandle = NULL;
    DWORD ThreadId;
    BOOL CallResult;


    _tprintf(TEXT("PipeMain started\n"));
    while (TRUE)
    {
        // open our named pipe...
        hPipe = CreateNamedPipe
        (
            lpszPipeName,              // name of pipe
            FILE_FLAG_OVERLAPPED |     // pipe open mode
              PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE |        // pipe IO type
              PIPE_READMODE_MESSAGE |
              PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,  // number of instances
            0,                         // size of outbuf (0 == allocate as necessary)
            0,                         // size of inbuf
            0,                         // default time-out value
            &SecAttr                   // security attributes
        );

        if (hPipe == INVALID_HANDLE_VALUE)
        {
            _tprintf(TEXT("PipeMain CreateNamedPipe failed, GLE %lu.\n"), GetLastError());
            ExitFunction(EXIT_FAILURE);
        }

        // init the overlapped structure
        ZeroMemory(&OVL, sizeof(OVL));
        OVL.hEvent = WaitEvents[1];
        ResetEvent(WaitEvents[1]);

        // wait for a connection...
        CallResult = ConnectNamedPipe(hPipe, &OVL);
        if (!CallResult && (GetLastError() == ERROR_IO_PENDING))
        {
            WaitResult = WaitForMultipleObjects(WaitEventsCnt, WaitEvents, FALSE, INFINITE);

            if (WaitResult == WAIT_FAILED)
                ExitFunction(EXIT_FAILURE);

            if (WaitResult != WAIT_OBJECT_0 + 1)     // not overlapped i/o event - error occurred, or server stop signaled
                ExitFunction(EXIT_SUCCESS);
        }

        // Create a thread for this client
        ThreadHandle = CreateThread
        (
            NULL,              // no security attribute 
            ThreadStackSizeDefault,  // default stack size 
            PIPEWorkerThread,  // thread proc
            (LPVOID)hPipe,     // thread parameter 
            0,                 // not suspended 
            &ThreadId          // returns thread ID 
        );

        if (ThreadHandle == NULL)
        {
            _tprintf(TEXT("PipeMain CreateThread failed, GLE %lu.\n"), GetLastError());
            ExitFunction(EXIT_FAILURE);
        }
        CloseHandle(ThreadHandle);
    }

function_end:
    if ((hPipe != INVALID_HANDLE_VALUE) && (hPipe != 0))
        CloseHandle(hPipe);

    _tprintf(TEXT("PipeMain exiting with result %lu\n"), Result);
    return Result;
}

// This routine is a thread processing function to read from and reply to a client
// via the open pipe connection passed from the main loop. Note this allows
// the main loop to continue executing, potentially creating more threads of
// of this procedure to run concurrently, depending on the number of incoming
// client connections
DWORD WINAPI PIPEWorkerThread(LPVOID lpvParam)
{
    DWORD Result = EXIT_SUCCESS;
    DWORD WaitResult;
    BOOL CallResult;
    OVERLAPPED OVL;
    CONST DWORD WaitEventsCnt = 2;
    HANDLE WaitEvents[2] = { GlobalVariables.SvcStopEvent, NULL };
    BYTE Request[DefaultBufferSize];
    HANDLE hPipe = (HANDLE)lpvParam;
    DWORD LastError;
    TGeoIPDBResponse Response;
    BYTE IPAddr[16];
    ADDRESS_FAMILY IPFamily;


    // Print verbose messages. In production code, this should be for debugging only.
    printf("Pipe %lu created, receiving and processing messages\n", GetCurrentThreadId());

    // Do some extra error checking since the app will keep running even if this thread fails
    if ((hPipe == NULL) || (hPipe == INVALID_HANDLE_VALUE))
        ExitFunction(EXIT_FAILURE);

    // create the event object object use in overlapped i/o
    WaitEvents[1] = CreateEvent
    (
        NULL,    // no security attributes
        TRUE,    // manual reset event
        FALSE,   // not-signalled
        NULL     // no name
    );
    if (WaitEvents[1] == NULL)
        ExitFunction(EXIT_FAILURE);


    // Loop until done reading
    while (TRUE)
    {
        // init the overlapped structure
        ZeroMemory(&OVL, sizeof(OVL));
        OVL.hEvent = WaitEvents[1];
        ResetEvent(WaitEvents[1]);

        // Set the buffer to all NULLs otherwise we get leftover characters
        ZeroMemory(&Request, DefaultBufferSize);

        // grab whatever's coming through the pipe...
        CallResult = ReadFile
        (
            hPipe,
            Request,
            DefaultBufferSize,
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
                    ExitFunction(EXIT_FAILURE);

                if (WaitResult != WAIT_OBJECT_0 + 1) // not overlapped i/o event - error occurred, or server stop signaled
                    ExitFunction(EXIT_FAILURE);

            case ERROR_SUCCESS:
                break;

            case ERROR_BROKEN_PIPE:
                _tprintf(TEXT("Pipe %lu client disconnected\n"), GetCurrentThreadId());
                ExitFunction(EXIT_SUCCESS);

            default:
                _tprintf(TEXT("Pipe %lu ReadFile failed, GLE %lu\n"), GetCurrentThreadId(), LastError);
                ExitFunction(EXIT_FAILURE);
            }
        }

        // 
        switch (((PGeoIPDBRequestHeader)&Request)->RequestType)
        {
        case RequestTypePing:
            break;

        case RequestTypeIPv4:
            memcpy_s(&IPAddr, 4, ((PGeoIPDBRequestIPv4)&Request)->Addr, 4);
            IPFamily = AF_INET;
            if (!ResponseForm((PBYTE)&IPAddr, IPFamily, &Response))
                ExitFunction(EXIT_FAILURE);

            break;

        case RequestTypeIPv6:
            memcpy_s(&IPAddr, 16, ((PGeoIPDBRequestIPv6)&Request)->Addr, 16);
            IPFamily = AF_INET6;
            if (!ResponseForm((PBYTE)&IPAddr, IPFamily, &Response))
                ExitFunction(EXIT_FAILURE);

            break;

        default:
            ExitFunction(EXIT_FAILURE);
        }

        // init the overlapped structure
        ZeroMemory(&OVL, sizeof(OVL));
        OVL.hEvent = WaitEvents[1];
        ResetEvent(WaitEvents[1]);

        // send it back out...
        CallResult = WriteFile
        (
            hPipe,
            &Response,
            sizeof(Response),
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
                    ExitFunction(EXIT_FAILURE);
                
                if (WaitResult != WAIT_OBJECT_0 + 1) // not overlapped i/o event - error occurred, or server stop signaled
                    ExitFunction(EXIT_FAILURE);

            case ERROR_SUCCESS:
                break;

            case ERROR_NO_DATA:
                _tprintf(TEXT("Pipe %lu client disconnected\n"), GetCurrentThreadId());
                ExitFunction(EXIT_SUCCESS);

            default:
                _tprintf(TEXT("Pipe %lu WriteFile failed, GLE %lu\n"), GetCurrentThreadId(), LastError);
                ExitFunction(EXIT_FAILURE);
            }
        }
    }

function_end:
    if (hPipe)
    {
        // Flush the pipe to allow the client to read the pipe's contents before disconnecting. Then disconnect the pipe, and close the
        // handle to this pipe instance
        FlushFileBuffers(hPipe);
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }

    // overlapped i/o event
    if (WaitEvents[1])
        CloseHandle(WaitEvents[1]);

    printf("Pipe %lu exiting, code %lu\n", GetCurrentThreadId(), Result);
    return Result;
}

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
BOOL CreateMyDACL(SECURITY_ATTRIBUTES* pSA)
{
    // Define the SDDL for the DACL. This example sets 
    // the following access:
    //     Built-in guests are denied all access.
    //     Anonymous Logon is denied all access.
    //     Authenticated Users are allowed read/write/execute access.
    //     Administrators are allowed full control.
    // Modify these values as needed to generate the proper
    // DACL for your application. 
    if (!pSA)
        return FALSE;

    LPCTSTR szSD =
        L"D:"                   // Discretionary ACL
        L"(D;OICI;GA;;;BG)"     // Deny access to Built-in Guests
        L"(D;OICI;GA;;;AN)"     // Deny access to Anonymous Logon
        L"(A;OICI;GRGWGX;;;AU)" // Allow read/write/execute to Authenticated Users
        L"(A;OICI;GA;;;BA)";    // Allow full control to Administrators

    return ConvertStringSecurityDescriptorToSecurityDescriptor
    (
        szSD,
        SDDL_REVISION_1,
        &(pSA->lpSecurityDescriptor),
        NULL
    );
}
