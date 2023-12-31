/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#define _WINSOCKAPI_
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdio.h>
#include <string>
#include <locale>
#include <array>
#include <vector>
#include <memory>
#include "SharedHeaders.h"
#include "SharedOptionsDefaults.h"
#include "maxminddb.h"
#include "LanguageRes.h"
#include "EventLog.h"
#include "resource.h"
#include "Protocol.h"
#include "DatabaseManager.h"
#include "BaseServer.h"
#include "GlobalOptions.h"
#include "EventLogWriter.h"
#include "TCPServer.h"
#include "PIPEServer.h"
#include "GeoIPService.h"
#include "GlobalVariables.h"


TGeoIPService::TGeoIPService(std::wstring& NameShort):
    NameShort(NameShort),
    SvcStatus({ 0 }),
    SvcStatusHandle(0),
    SvcStopEvent(0),
    EventLogWriter(),
    Database(),
    GlobalOptions(),
    ServersArray(),
    PIPEEvents(),
    TCPEvents()
{
    this->SvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; // The service runs in its own process    
    TGlobalVariables::PServiceThis = this;
}

TGeoIPService::~TGeoIPService() noexcept
{
    TGlobalVariables::PServiceThis = nullptr;
}

DWORD TGeoIPService::Run()
{
    SetErrorMode(SEM_FAILCRITICALERRORS); // Don't popup on floppy query and etc.

    // services for the process
    CONST std::array<SERVICE_TABLE_ENTRY, 2> DispatchTable
    {{
        { &NameShort[0], WinAPISvcMain },
        { NULL, NULL }
    }};

    // When the service control manager starts a service process, it waits for the process to call the
    // StartServiceCtrlDispatcher function. The main thread of a service process should make this call as soon as possible
    // after it starts up (within 30 seconds)
    // 
    // The process should simply terminate when the call returns (this call returns when the service has stopped)
    if (!StartServiceCtrlDispatcher(DispatchTable.data()))
    {
        switch (GetLastError())
        {
        // the program is being run as a console application rather than as a service
        case ERROR_FAILED_SERVICE_CONTROLLER_CONNECT:
            MessageBox(NULL, LangGet(UIMSG_100_CANT_BE_RUN_AS_STD_APP), NULL, MB_OK | MB_ICONWARNING);

        default:
            return EXIT_FAILURE;

        }
    }

    return EXIT_SUCCESS;
}

VOID WINAPI TGeoIPService::WinAPISvcMain(_In_ DWORD dwArgc, _In_opt_ LPTSTR* pszArgv)
{
    if (TGlobalVariables::PServiceThis)
        TGlobalVariables::PServiceThis->SvcMain(dwArgc, pszArgv);
}

//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service
VOID TGeoIPService::SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    DWORD Result = NO_ERROR;


    // perform initialization
    // A common bug is for the service to have the main thread perform the initialization while a separate thread continues to call
    // SetServiceStatus to prevent the service control manager from marking it as hung. However, if the main thread hangs, then the service
    // start ends up in an infinite loop because the worker thread continues to report that the main thread is making progress
    if (!SvcInit())
        ExitFunction(EXIT_FAILURE);

    if (!AppInit())
        ExitFunction(EXIT_FAILURE);

    if (!AppResume())
        ExitFunction(EXIT_FAILURE);

    // working state
    AppMain();


function_end:
    // time to shut down
    EventLog(ELMSG_SHUTDOWN);

    AppCease();    
    AppCleanup();

    SvcReportStatus(SERVICE_STOPPED, Result, 0);
    // Do not attempt to perform any additional work after calling SetServiceStatus with SERVICE_STOPPED, 
    // because the service process can be terminated at any time

    // If a service calls SetServiceStatus with the dwCurrentState member set to SERVICE_STOPPED and the dwWin32ExitCode
    // member set to a nonzero value, the entry is written into the System event log
}

BOOL TGeoIPService::SvcInit()
{
    // A ServiceMain function first calls the RegisterServiceCtrlHandler[Ex] function to get the service's SERVICE_STATUS_HANDLE. 
    // Then it immediately calls the SetServiceStatus function to notify the service control manager that its status is SERVICE_START_PENDING. 
    // During initialization, the service can provide updated status to indicate that it is making progress but it needs more time
    if (!EventsInit())
        return FALSE;
    
    // Register the handler function for the service
    this->SvcStatusHandle = RegisterServiceCtrlHandler
    (
        &NameShort[0],
        WinAPISvcCtrlHandler
    );
    if (!this->SvcStatusHandle) // If the function fails, the return value is zero
    {
        EventLog(ELMSG_NO_SERVICE_CTRL_HANDLER);
        return FALSE;
    }

    // Report initial status to the SCM
    SvcReportStatus(SERVICE_START_PENDING, NO_ERROR, SvcReportStatusTimeHint);

    return TRUE;
}

BOOL TGeoIPService::AppInit()
{
    BOOL CallResult;
    std::shared_ptr<TServerOptionsBase> SRV;
    DWORD Count = 0;


    // read options
    try
    {
        this->GlobalOptions = std::make_unique<TGlobalOptions>();
    }
    catch (GlobalOptionsInitException&)
    {
        EventLog(ELMSG_REGISTRY_OPTIONS_MISCONFIG);
        return FALSE;
    }
    SvcReportStatus(SERVICE_START_PENDING, NO_ERROR, SvcReportStatusTimeHint);

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code
    this->SvcStopEvent = CreateEvent
    (
        NULL,    // default security attributes
        TRUE,    // manual reset event
        FALSE,   // not signaled
        NULL     // no name
    );
    if (!this->SvcStopEvent)
    {
        EventLog(ELMSG_NO_SERVICE_STOP_EVENT);
        return FALSE;
    }
    SvcReportStatus(SERVICE_START_PENDING, NO_ERROR, SvcReportStatusTimeHint);

    // TO_DO: Declare and set any required variables.
    // Be sure to periodically call ReportSvcStatus() with 
    // SERVICE_START_PENDING. If initialization fails, call
    // ReportSvcStatus with SERVICE_STOPPED.
    CallResult = this->Database.Open
    (
        this->GlobalOptions->DatabaseFileFullPathGeoGet(),
        this->GlobalOptions->DatabaseFileFullPathASNGet()
    );
    if (!CallResult)
    {
        EventLog(ELMSG_DB_FILES_INIT_ERR);
        return FALSE;
    }
    SvcReportStatus(SERVICE_START_PENDING, NO_ERROR, SvcReportStatusTimeHint);

    // create servers as configured
    while (this->GlobalOptions->ServerOptionsEnumNext(SRV))
    {
        SvcReportStatus(SERVICE_START_PENDING, NO_ERROR, SvcReportStatusTimeHint);

        if (!SRV)
            continue;

        // skip disabled
        if (!SRV->Enabled)
            continue;

        // create by type
        std::shared_ptr<TServerOptionsPIPE> OptPIPE = std::dynamic_pointer_cast<TServerOptionsPIPE>(SRV);
        if (OptPIPE)
        try
        {
            this->ServersArray.emplace_back(std::make_shared
                <TPIPEServer>
                (
                    &this->Database,
                    this->SvcStopEvent,
                    this->GlobalOptions.get(),
                    OptPIPE->PipeName,
                    OptPIPE->SecurityAttributesPolicy,
                    OptPIPE->SecurityAttributesCustom,
                    OptPIPE->IOTimeoutMS
                ));

            ++Count;
            continue;
        }
        catch (PIPEServerException&)
        {
            EventLog(ELMSG_PIPESRV_INIT_ERROR);
            return FALSE;
        }

        std::shared_ptr<TServerOptionsTCP> OptTCP = std::dynamic_pointer_cast<TServerOptionsTCP>(SRV);
        if (OptTCP)
        try
        {
            ADDRESS_FAMILY IPFamily = AF_INET;
            if (SRV->Type == ServerTypeTCP6)
                IPFamily = AF_INET6;

            this->ServersArray.emplace_back(std::make_shared
                <TTCPServer>
                (
                    &this->Database,
                    this->SvcStopEvent,
                    this->GlobalOptions.get(),
                    IPFamily,
                    OptTCP->Address,
                    OptTCP->Port,
                    OptTCP->TimeoutRecieveMS,
                    OptTCP->TimeoutSendMS
                ));

            ++Count;
            continue;
        }
        catch (TCPServerException&)
        {
            EventLog(ELMSG_TCPSRV_INIT_ERROR);
            return FALSE;
        }

        // unknown class
        EventLog(ELMSG_REGISTRY_OPTIONS_MISCONFIG);
        return FALSE;
    }

    if (!Count)
    {
        EventLog(ELMSG_REGISTRY_OPTIONS_MISCONFIG);
        return FALSE;
    }

    return TRUE;
}

BOOL TGeoIPService::AppResume()
{
    BOOL CallResult;


    // Start all servers
    auto i = this->ServersArray.begin();
    while( i != this->ServersArray.end())
    {
        SvcReportStatus(SERVICE_START_PENDING, NO_ERROR, SvcReportStatusTimeHint);

        std::shared_ptr<TPIPEServer> sp = std::dynamic_pointer_cast<TPIPEServer>(*i);
        if (sp)
        {
            sp->Log = this->PIPEEvents;
            CallResult = sp->StartMainThread();
            if (!CallResult)
            {
                EventLog(ELMSG_PIPESRV_INIT_ERROR);
                return FALSE;
            }
            
            // log server started
            EventLogPIPEServerStart
            (
                sp->PipeNameGet(),
                sp->MainThreadIdGet()
            );

            ++i;
            continue;
        }

        std::shared_ptr<TTCPServer> st = std::dynamic_pointer_cast<TTCPServer>(*i);
        if (st)
        {            
            st->Log = this->TCPEvents;
            CallResult = st->StartMainThread();
            if (!CallResult)
            {
                EventLog(ELMSG_TCPSRV_INIT_ERROR);
                return FALSE;
            }

            // log server started
            EventLogTCPServerStart
            (
                st->IPAddrGet(),
                st->IPPortGet(),
                st->MainThreadIdGet(),
                st->ShutdownThreadIdGet()
            );

            ++i;
            continue;
        }

        EventLog(ELMSG_REGISTRY_OPTIONS_MISCONFIG);
        return FALSE;
    }

    return TRUE;
}

BOOL TGeoIPService::AppMain()
{
    // Report running status
    SvcReportStatus(SERVICE_RUNNING, NO_ERROR, 0);

    // log up and running
    EventLog(ELMSG_RUNNING);

    // Check whether to stop the service
    WaitForSingleObject(this->SvcStopEvent, INFINITE);

    return TRUE;
}

BOOL TGeoIPService::AppCease() noexcept
{
    SvcReportStatus(SERVICE_STOP_PENDING, NO_ERROR, SvcReportStatusTimeHint);

    // Signal to all working threads to stop
    SetEvent(this->SvcStopEvent);
    
    return TRUE;
}

BOOL TGeoIPService::AppCleanup()
{
    this->ServersArray.clear();
    this->Database.Close();
    SvcReportStatus(SERVICE_STOP_PENDING, NO_ERROR, SvcReportStatusTimeHint);

    // destroy objects    
    this->PIPEEvents.reset();
    this->TCPEvents.reset();
    CloseHandle(this->SvcStopEvent);
    this->GlobalOptions.reset();
    SvcReportStatus(SERVICE_STOP_PENDING, NO_ERROR, SvcReportStatusTimeHint);

    // last log
    EventLog(ELMSG_DOWN);
    this->EventLogWriter.reset();
    SvcReportStatus(SERVICE_STOP_PENDING, NO_ERROR, SvcReportStatusTimeHint);

    return TRUE;
}

VOID WINAPI TGeoIPService::WinAPISvcCtrlHandler(DWORD dwCtrl)
{
    if (TGlobalVariables::PServiceThis)
        TGlobalVariables::PServiceThis->SvcCtrlHandler(dwCtrl);
}

// Called by SCM whenever a control code is sent to the service using the ControlService function
VOID TGeoIPService::SvcCtrlHandler(DWORD dwCtrl)
{
    // Handle the requested control code
    switch (dwCtrl)
    {
    case SERVICE_CONTROL_STOP:
        SvcReportStatus(SERVICE_STOP_PENDING, NO_ERROR, SvcReportStatusTimeHint);
        EventLog(ELMSG_SERVICE_CONTROL_STOP);

        // Signal the service to stop
        SetEvent(SvcStopEvent);
        SvcReportStatus(SERVICE_STOP_PENDING, NO_ERROR, SvcReportStatusTimeHint);

        break;

    case SERVICE_CONTROL_INTERROGATE:
        // Notifies a service that it should report its current status information to the service control manager
        // Note that this control is not generally useful as the SCM is aware of the current state of the service
        SvcReportStatus(this->SvcStatus.dwCurrentState, NO_ERROR, 0);

        break;

    default:
        break;
    }
}

//   Sets the current service status and reports it to the SCM
VOID TGeoIPService::SvcReportStatus(CONST DWORD CurrentState, CONST DWORD Win32ExitCode, CONST DWORD WaitHint) noexcept
{
    static DWORD dwCheckPoint = 1;


    // Fill in the SERVICE_STATUS structure.
    //dwServiceType
    this->SvcStatus.dwCurrentState = CurrentState;
    this->SvcStatus.dwControlsAccepted = 0;
    this->SvcStatus.dwWin32ExitCode = Win32ExitCode;
    //dwServiceSpecificExitCode // This value is ignored unless the dwWin32ExitCode member is set to ERROR_SERVICE_SPECIFIC_ERROR
    //dwCheckPoint
    this->SvcStatus.dwWaitHint = WaitHint;
    

    if (CurrentState != SERVICE_START_PENDING)
        // we don't we support pause/continue, now service will be reported by SCM as not supporting pause/continue
        this->SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((CurrentState == SERVICE_RUNNING) || (CurrentState == SERVICE_STOPPED))
        this->SvcStatus.dwCheckPoint = 0; // This value is not valid and should be zero when the service does not have a start, stop, pause, or continue operation pending
    else
        this->SvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM
    SetServiceStatus(this->SvcStatusHandle, &this->SvcStatus);
}

BOOL TGeoIPService::EventsInit()
{
    try
    {
        this->EventLogWriter = std::make_shared<TEventLogWriter>(this->NameShort);
        EventLog(ELMSG_START_INIT);
        this->PIPEEvents = std::make_shared<TPIPEServerEvents>(this->EventLogWriter);
        this->TCPEvents = std::make_shared<TTCPServerEvents>(this->EventLogWriter);
    }
    catch (TEventLogWriterCreateException&)
    {
        return FALSE;
    }

    return TRUE;
}

VOID TGeoIPService::EventLog(CONST DWORD EventID) const
{
    if (!this->EventLogWriter)
        return;

    *(this->EventLogWriter) << EventID;
}

VOID TGeoIPService::EventLogPIPEServerStart(CONST std::wstring& PIPEName, DWORD MainThreadID) const
{
    this->EventLogWriter->Write
    (
        ELMSG_PIPESRV_INIT_DONE,
        {
            std::to_wstring(MainThreadID),
            PIPEName 
        }
    );
}

VOID TGeoIPService::EventLogTCPServerStart(CONST std::wstring& IPAddr, WORD IPPort, DWORD MainThreadID, DWORD ShutdwThreadID) const
{
    this->EventLogWriter->Write
    (
        ELMSG_TCPSRV_INIT_DONE,
        {
            std::to_wstring(MainThreadID),
            IPAddr + TEXT(":") + std::to_wstring(IPPort),
            std::to_wstring(ShutdwThreadID)
        }
    );
}


//
// PIPE Server Events
//

TPIPEServerEvents::TPIPEServerEvents(std::shared_ptr<TEventLogWriter> EventLogWriter) noexcept:
    EventLogWriter(EventLogWriter)
{
    return; // NOP
}

VOID TPIPEServerEvents::OnMainThreadStart(std::wstring& PIPEName) const
{
    return; // NOP
}

VOID TPIPEServerEvents::OnMainThreadExit(std::wstring& PIPEName, DWORD Result, HANDLE StopEvent) const
{
    //
    // THIS CODE IS EXECUTED BY THE SERVER THREAD, NOT THE MAIN ONE
    //
    DWORD EventID = ELMSG_PIPESRV_SHUTDOWN;
    std::vector<std::wstring> StrArray;
    StrArray.reserve(3);
    StrArray.emplace_back(std::to_wstring(GetCurrentThreadId()));
    StrArray.emplace_back(PIPEName);

    // in case of an error
    if (Result)
    {
        EventID = ELMSG_PIPESRV_SHUTDOWN_ERROR;
        StrArray.emplace_back(std::to_wstring(Result)); // std::format("{:#010x}", your_int);

        // Signal the service to stop
        this->EventLogWriter->Write(ELMSG_SERVICE_CRITICAL_THREAD_STOPPED);
        SetEvent(StopEvent);
    }

    this->EventLogWriter->Write(EventID, StrArray);
}

VOID TPIPEServerEvents::OnWorkerThreadStart(std::wstring& PIPEName) const
{
    return; // NOP
}

VOID TPIPEServerEvents::OnWorkerThreadExit(std::wstring& PIPEName, DWORD Result, HANDLE StopEvent) const
{
    //
    // THIS CODE IS EXECUTED BY THE WORKER THREAD, NOT THE MAIN ONE
    //
    if (!Result) // log only errors
        return;

    this->EventLogWriter->Write
    (
        ELMSG_PIPEWORKER_SHUTDOWN_ERROR,
        {
            PIPEName,
            std::to_wstring(GetCurrentThreadId()),
            std::to_wstring(Result)
        }
    );
}


//
// TCP Server Events
//
TTCPServerEvents::TTCPServerEvents(std::shared_ptr<TEventLogWriter> EventLogWriter) noexcept:
    EventLogWriter(EventLogWriter)
{
    return; // NOP
}

VOID TTCPServerEvents::OnMainThreadStart(std::wstring& IPAddr, WORD IPPort) const
{
    return; // NOP
}

VOID TTCPServerEvents::OnMainThreadExit(std::wstring& IPAddr, WORD IPPort, DWORD Result, HANDLE StopEvent) const
{
    //
    // THIS CODE IS EXECUTED BY THE SERVER THREAD, NOT THE MAIN ONE
    //
    DWORD EventID = ELMSG_TCPSRV_SHUTDOWN;
    std::vector<std::wstring> StrArray;
    StrArray.reserve(3);
    StrArray.emplace_back(std::to_wstring(GetCurrentThreadId()));
    StrArray.emplace_back(IPAddr + TEXT(":") + std::to_wstring(IPPort));

    // in case of an error
    if (Result)
    {
        EventID = ELMSG_TCPSRV_SHUTDOWN_ERROR;
        StrArray.emplace_back(std::to_wstring(Result)); // std::format("{:#010x}", your_int);
        
        // Signal the service to stop
        this->EventLogWriter->Write(ELMSG_SERVICE_CRITICAL_THREAD_STOPPED);
        SetEvent(StopEvent);
    }

    this->EventLogWriter->Write(EventID, StrArray);
}

VOID TTCPServerEvents::OnShutdownThreadStart(std::wstring& IPAddr, WORD IPPort) const
{
    return; // NOP
}

VOID TTCPServerEvents::OnShutdownThreadExit(std::wstring& IPAddr, WORD IPPort, HANDLE StopEvent) const
{
    // Signal the service to stop
    this->EventLogWriter->Write(ELMSG_SERVICE_CRITICAL_THREAD_STOPPED);
    SetEvent(StopEvent);
}

VOID TTCPServerEvents::OnWorkerThreadStart(std::wstring& IPAddr, WORD IPPort) const
{
    return; // NOP
}

VOID TTCPServerEvents::OnWorkerThreadExit(std::wstring& IPAddr, WORD IPPort, DWORD Result, HANDLE StopEvent) const
{
    //
    // THIS CODE IS EXECUTED BY THE SERVER THREAD, NOT THE MAIN ONE
    //
    if (!Result) // log only errors
        return;

    this->EventLogWriter->Write
    (
        ELMSG_TCPWORKER_SHUTDOWN_ERROR,
        {
            IPAddr + TEXT(":") + std::to_wstring(IPPort),
            std::to_wstring(GetCurrentThreadId()),
            std::to_wstring(Result)
        }
    );
}
