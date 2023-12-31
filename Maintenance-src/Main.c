/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#include <windows.h>
#include <tchar.h>
#include "SharedHeaders.h"
#include "resource.h"
#include "SharedOptionsDefaults.h"
#include "GlobalOptions.h"
#include "CommandLineInterface.h"
#include "LanguageRes.h"
#include "PathWorks.h"
#include "Main.h"

#pragma warning(disable: 6262)


TGlobalOptions GlobalOptions = { 0 };
BOOL PrintGeneralErrorAtEnd = TRUE;

DWORD _tmain(DWORD argc, LPCTSTR argv[], LPCTSTR envp[])
{
    // prepare enviroment
    CLISetModeUTF16();
    SetErrorMode(SEM_FAILCRITICALERRORS); // Don't popup on floppy query and etc.
    SetProcessShutdownParameters(0x1FF, 0);
    SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
    CLIWriteLN();

    // collect parameters
    if (!CLIWorkModeGet(argc, argv, &GlobalOptions))
        return EXIT_FAILURE;
    if (GlobalOptions.OperatingMode == OperatingModeHelp)
        return ModeHelp();
    if (!CLIPathsGet(argc, argv, &GlobalOptions))
        return EXIT_FAILURE;

    if (!GlobalOptions.NoCopyrightLogo)
        CLILogoPrint();

    // do the job
    switch (GlobalOptions.OperatingMode)
    {
    case OperatingModeInstall:
        return ModeInstall();
        break;

    case OperatingModeUninstall:
        return ModeUninstall();
        break;

    default:
        return EXIT_FAILURE;
    }
}

DWORD ModeHelp()
{
    CLILogoPrint();
    CLIHelpPrint();

    return EXIT_SUCCESS;
}

DWORD ModeInstall()
{
    DWORD Result = EXIT_SUCCESS;
    TCHAR PathToServiceEXE[ExtendedPathMaxChar];


    _tprintf_s(LangGet(UIMSG_103_INSTALLING_SVC));
    // get full path
    if (!GetFullPathName(GlobalOptions.PathToServiceEXE, ExtendedPathMaxChar, PathToServiceEXE, NULL))
        ExitFunction(FALSE);

    // In case the path contains a space, it must be quoted so that it is correctly interpreted. For example,
    //  d:\my share\myservice.exe  should be specified as
    // "d:\my share\myservice.exe"
    if (!PathStringQuoteIfHasSpaces(PathToServiceEXE, ExtendedPathMaxChar))
        ExitFunction(EXIT_FAILURE);

    if (!FileExists(PathToServiceEXE))
    {
        _tprintf_s(LangGet(UIMSG_108_NOT_FOUND_FILE));
        PrintGeneralErrorAtEnd = FALSE;
        ExitFunction(EXIT_FAILURE);
    }

    if (!SvcInstall((LPCTSTR)&PathToServiceEXE))
        ExitFunction(EXIT_FAILURE);

    if (!AddEventSource((LPCTSTR)&PathToServiceEXE, 0))
        ExitFunction(EXIT_FAILURE);

    if (!SetOptions((LPCTSTR)&PathToServiceEXE))
        ExitFunction(EXIT_FAILURE);   


function_end:
    if (!Result)
        _tprintf_s(LangGet(UIMSG_105_OK_INSTALL));
    else
        if (PrintGeneralErrorAtEnd)
            _tprintf_s(LangGet(UIMSG_104_FAILED_INSTALL));

    return Result;
}

DWORD ModeUninstall()
{
    DWORD Result = EXIT_SUCCESS;


    _tprintf_s(LangGet(UIMSG_112_UNINSTALLING_SVC));

    if (!SvcUninstall())
        ExitFunction(EXIT_FAILURE);

    if (!RemoveEventSource())
        ExitFunction(EXIT_FAILURE);
    
    if (!RemoveOptions())
        ExitFunction(EXIT_FAILURE);


function_end:
    if (!Result)
        _tprintf_s(LangGet(UIMSG_113_UNINSTALL_OK));
    else
        if (PrintGeneralErrorAtEnd)
            _tprintf_s(LangGet(UIMSG_114_FAILED_UNINSTALL));

    return Result;
}

// Installs a service in the SCM database
BOOL SvcInstall(LPCTSTR Path)
{
    BOOL Result = TRUE;
    SC_HANDLE SCM;
    SC_HANDLE Service = NULL;
    SERVICE_DESCRIPTION ServiceDescription;
    DWORD LastERR;
    SERVICE_FAILURE_ACTIONS failureActions;
    SC_ACTION restartAction;
    SERVICE_FAILURE_ACTIONS_FLAG flags;
    BOOL CallResult;


    // Get a handle to the SCM database
    SCM = OpenSCManager
    (
        NULL,                      // local computer
        NULL,                      // ServicesActive database
        SC_MANAGER_CREATE_SERVICE  // or SC_MANAGER_ALL_ACCESS
    );
    if (!SCM)
        ExitFunction(FALSE);

    // NOTE:
    // No need to lock the database from Windows Vista and later, LockServiceDatabase() function is provided for 
    // application compatibility and has no effect on the database

    // Create the service
    Service = CreateService
    (
        SCM,                       // SCM database handle
        SVCNAME,                   // service name short
        LangGet(UIMSG_101_SVCDISPLAYNAME), // service name to display 
        SERVICE_ALL_ACCESS,        // desired access
        SERVICE_WIN32_OWN_PROCESS, // service type
        SERVICE_AUTO_START,        // start type
        SERVICE_ERROR_NORMAL,      // error control type
        Path,                      // path to service's binary
        NULL,                      // no load ordering group
        NULL,                      // no tag identifier
        SERVICE_DEPENDENCIES,      // dependencies
        NULL,                      // LocalSystem account
        NULL                       // no password
    );
    // display name to be used by user interface programs to identify the service. maximum length is 256 characters
    if (!Service)
    {
        LastERR = GetLastError();
        switch (LastERR)
        {
        case ERROR_SERVICE_EXISTS:
            PrintGeneralErrorAtEnd = FALSE;
            _tprintf_s(LangGet(UIMSG_107_ERR_SERVICE_EXISTS), TEXT("ERROR_SERVICE_EXISTS"));
            ExitFunction(FALSE);

        default: // ERROR_ACCESS_DENIED
            ExitFunction(FALSE);
        }
    }

    ServiceDescription.lpDescription = (LPTSTR)LangGet(UIMSG_102_SVCDESCR);
    CallResult = ChangeServiceConfig2
    (
        Service,                    // handle to service
        SERVICE_CONFIG_DESCRIPTION, // change: description
        &ServiceDescription         // value: new description
    );
    if (!CallResult)
        ExitFunction(FALSE);

    // You cannot set the SERVICE_CONFIG_FAILURE_ACTIONS value for a service that shares the service control manager's process. 
    // This includes all services whose executable image is "Services.exe".
    restartAction.Type = SC_ACTION_RESTART;
    restartAction.Delay = 5000; // 5 seconds
    failureActions.dwResetPeriod = 86400; // 1 day
    failureActions.lpRebootMsg = NULL;
    failureActions.lpCommand = NULL;
    failureActions.cActions = 1;
    failureActions.lpsaActions = &restartAction;    
    CallResult = ChangeServiceConfig2
    (
        Service,
        SERVICE_CONFIG_FAILURE_ACTIONS,
        &failureActions
    );
    if (!CallResult)
        ExitFunction(FALSE);

    // If this member is FALSE and the service has configured failure actions, the failure actions are queued only 
    // if the service terminates without reporting a status of SERVICE_STOPPED
    flags.fFailureActionsOnNonCrashFailures = FALSE; // Set to 0 for default failure actions
    CallResult = ChangeServiceConfig2
    (
        Service,
        SERVICE_CONFIG_FAILURE_ACTIONS_FLAG,
        &flags
    );
    if (!CallResult)
        ExitFunction(FALSE);


function_end:
    if (Service)
        CloseServiceHandle(Service);

    if (SCM)
        CloseServiceHandle(SCM);

    return Result;
}

BOOL SvcUninstall()
{
    BOOL Result = TRUE;
    SC_HANDLE SCM;
    SC_HANDLE Service = NULL;
    DWORD LastERR;
    BOOL CallResult;
    SERVICE_STATUS ServiceStatus;


    // get a handle to the SCM database
    SCM = OpenSCManager
    (
        NULL,                    // local computer
        NULL,                    // ServicesActive database
        SC_MANAGER_CONNECT       // or SC_MANAGER_ALL_ACCESS
    );
    if (!SCM)
        ExitFunction(FALSE);

    // NOTE:
    // No need to lock the database from Windows Vista and later, LockServiceDatabase() function is provided for 
    // application compatibility and has no effect on the database

    // open service
    Service = OpenService
    (
        SCM,                       // SCM database handle
        SVCNAME,                   // service name short
        DELETE | SERVICE_QUERY_STATUS // desired access
    );
    if (!Service)
    {
        LastERR = GetLastError();
        switch (LastERR)
        {
        case ERROR_SERVICE_DOES_NOT_EXIST:
            PrintGeneralErrorAtEnd = FALSE;
            _tprintf_s(LangGet(UIMSG_110_NOT_EXISTS));
            // let following uninstall procedures to be called too to clean the registry up in case of unfinished installation and etc.
            ExitFunction(TRUE);

        default: // ERROR_ACCESS_DENIED
            ExitFunction(FALSE);
        }
    }

    // check the service is stopped
    CallResult = QueryServiceStatus(Service, &ServiceStatus);
    if (!CallResult)
        ExitFunction(FALSE);

    if (ServiceStatus.dwCurrentState != SERVICE_STOPPED)
    {
        PrintGeneralErrorAtEnd = FALSE;
        _tprintf_s(LangGet(UIMSG_115_SERVICE_RUNNING));
        ExitFunction(FALSE);
    }

    // mark the service for deletion from the service control manager database
    CallResult = DeleteService(Service);
    if (!CallResult)
    {
        LastERR = GetLastError();
        switch (LastERR)
        {
        case ERROR_SERVICE_MARKED_FOR_DELETE:
            _tprintf_s(LangGet(UIMSG_111_MARKED_FOR_DELETION));
            break;

        default: // ERROR_ACCESS_DENIED
            ExitFunction(FALSE);
        }
    }


function_end:
    if (Service)
        CloseServiceHandle(Service);

    if (SCM)
        CloseServiceHandle(SCM);


    return Result;
}

// Installs our app as a source of events under the name pszName into the registry
BOOL AddEventSource(LPCTSTR Path, DWORD CategoryCount)
{
    BOOL Result = TRUE;
    HKEY HandleReg;
    LSTATUS CallResult;
    TCHAR RegPath[MAX_PATH];
    DWORD PathSizeBytes;
    DWORD dwTypes;
    DWORD Disposition;


    // get full registry hive path
    _stprintf_s(RegPath, MAX_PATH, RegEventPath, SVCNAME);
    // Create the event source registry key
    CallResult = RegCreateKeyEx
    (
        HKEY_LOCAL_MACHINE,
        RegPath,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &HandleReg,
        &Disposition
    );
    if (CallResult != ERROR_SUCCESS)
        ExitFunction(FALSE);

    if (Disposition == REG_OPENED_EXISTING_KEY)
    {
        PrintGeneralErrorAtEnd = FALSE;
        _tprintf_s(LangGet(UIMSG_107_ERR_SERVICE_EXISTS), TEXT("REG_OPENED_EXISTING_KEY"));
        ExitFunction(FALSE);
    }

    // Path to one or more event message files; use a semicolon to delimit multiple files. An event message file contains
    // language-dependent strings that describe the events. This value can be of type REG_SZ or REG_EXPAND_SZ
    PathSizeBytes = (DWORD)((_tcslen(Path) + 1) * sizeof(TCHAR));
    CallResult = RegSetValueEx
    (
        HandleReg,
        TEXT("EventMessageFile"),
        0,
        REG_EXPAND_SZ,
        (PBYTE)Path,
        PathSizeBytes
    );
    if (CallResult != ERROR_SUCCESS)
        ExitFunction(FALSE);

    // Path to the parameter message file. A parameter message file contains language-independent strings that are to be inserted
    // into the event description strings. This value can be of type REG_SZ or REG_EXPAND_SZ.
    //PathSizeBytes = (DWORD)((_tcslen(PathParam) + 1) * sizeof(TCHAR));
    //CallResult = RegSetValueEx
    //(
    //    HandleReg,
    //    TEXT("ParameterMessageFile"),
    //    0,
    //    REG_EXPAND_SZ,
    //    (PBYTE)PathParam,
    //    PathSizeBytes
    //);
    //if (CallResult != ERROR_SUCCESS)
    //    ExitFunction(FALSE);

    // Bitmask of supported types. This value is of type REG_DWORD
    dwTypes = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
    CallResult = RegSetValueEx
    (
        HandleReg,
        TEXT("TypesSupported"),
        0,
        REG_DWORD,
        (LPBYTE)&dwTypes,
        sizeof(dwTypes)
    );
    if (CallResult != ERROR_SUCCESS)
        ExitFunction(FALSE);

    // If we want to support event categories, we have also to register the CategoryMessageFile
    // and set CategoryCount. Note that categories need to have the message ids 1 to CategoryCount!
    if (CategoryCount > 0)
    {
        // Path to the category message file. A category message file contains language-dependent strings that describe the categories
        CallResult = RegSetValueEx
        (
            HandleReg,
            TEXT("CategoryMessageFile"),
            0,
            REG_EXPAND_SZ,
            (PBYTE)Path,
            PathSizeBytes
        );
        if (CallResult != ERROR_SUCCESS)
            ExitFunction(FALSE);

        // Number of event categories supported
        CallResult = RegSetValueEx
        (
            HandleReg,
            TEXT("CategoryCount"),
            0,
            REG_DWORD,
            (PBYTE)&CategoryCount,
            sizeof(CategoryCount)
        );
        if (CallResult != ERROR_SUCCESS)
            ExitFunction(FALSE);
    }


function_end:
    if (HandleReg)
        RegCloseKey(HandleReg);

    return Result;
}

BOOL RemoveEventSource()
{
    BOOL Result = TRUE;
    LSTATUS CallResult;
    TCHAR RegPath[MAX_PATH];


    // get full registry hive path
    _stprintf_s(RegPath, MAX_PATH, RegEventPath, SVCNAME);

    CallResult = RegDeleteTree(HKEY_LOCAL_MACHINE, RegPath);
    switch (CallResult)
    {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_SUCCESS:
        break;

    default: // ERROR_ACCESS_DENIED
        ExitFunction(FALSE);
    }


function_end:

    return Result;
}

BOOL SetOptions(LPCTSTR Path)
{
    BOOL Result = TRUE;
    HKEY HandleReg = 0;
    LSTATUS CallResult;
    DWORD PathSizeBytes;
    TCHAR InstPath[ExtendedPathMaxChar];
    DWORD Disposition;


    // acquiring required privileges
    if (!PrivilegeGet(SE_RESTORE_NAME))
        ExitFunction(FALSE);

    // Create the options registry key
    CallResult = RegCreateKeyEx
    (
        HKEY_LOCAL_MACHINE,
        RegistryOptionsPath,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &HandleReg,
        &Disposition
    );
    if (CallResult != ERROR_SUCCESS)
        ExitFunction(FALSE);

    if (Disposition == REG_OPENED_EXISTING_KEY)
    {
        PrintGeneralErrorAtEnd = FALSE;
        _tprintf_s(LangGet(UIMSG_107_ERR_SERVICE_EXISTS), TEXT("REG_OPENED_EXISTING_KEY"));
        ExitFunction(FALSE);
    }
   
    // adding all options at onece
    CallResult = RegRestoreKey
    (
        HandleReg,
        RegistryOptionsInitValuesFile,
        0
    );
    if (CallResult != ERROR_SUCCESS)
        ExitFunction(FALSE);

    // prepare path buffer
    _tcscpy_s(InstPath, ExtendedPathMaxChar, Path);
    PathStringFileNameRemove(InstPath);
    // update key
    PathSizeBytes = (DWORD)((_tcslen(InstPath) + 1) * sizeof(TCHAR));
    CallResult = RegSetValueEx
    (
        HandleReg,
        RegistryValInstalledPath,
        0,
        REG_EXPAND_SZ,
        (PBYTE)InstPath,
        PathSizeBytes
    );
    if (CallResult != ERROR_SUCCESS)
        ExitFunction(FALSE);

    // prepare path buffer
    _tcscat_s(InstPath, ExtendedPathMaxChar, DatabaseDefaultFolderName);
    PathStringSlashTrailingIclude(InstPath, ExtendedPathMaxChar);
    // update key
    PathSizeBytes = (DWORD)((_tcslen(InstPath) + 1) * sizeof(TCHAR));
    CallResult = RegSetValueEx
    (
        HandleReg,
        RegistryValDatabasePath,
        0,
        REG_EXPAND_SZ,
        (PBYTE)InstPath,
        PathSizeBytes
    );
    if (CallResult != ERROR_SUCCESS)
        ExitFunction(FALSE);


function_end:
    if (HandleReg)
        RegCloseKey(HandleReg);

    return Result;
}

BOOL RemoveOptions()
{
    BOOL Result = TRUE;
    LSTATUS CallResult;


    // remove whole tree at one call
    CallResult = RegDeleteTree(HKEY_LOCAL_MACHINE, RegistryOptionsPath);
    switch (CallResult)
    {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_SUCCESS:
        break;

    default: // ERROR_ACCESS_DENIED
        ExitFunction(FALSE);
    }


function_end:

    return Result;
}

// get attributes of a file or a folder
BOOL FileGetAttributes(LPCTSTR PathString, LPBY_HANDLE_FILE_INFORMATION FileInformation)
{
    DWORD Result = TRUE;
    HANDLE h;


    // In rare cases or on a heavily loaded system, file attribute information on NTFS file systems 
    // may not be current at the time FindFirstFile is called, so using GetFileInformationByHandle instead
    h = CreateFile(PathString, 0, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (h == INVALID_HANDLE_VALUE)
        ExitFunction(FALSE);

    if (!GetFileInformationByHandle(h, FileInformation))
        ExitFunction(FALSE);


function_end:
    if (h != INVALID_HANDLE_VALUE)
        CloseHandle(h);

    return Result;
}

BOOL FileExists(LPCTSTR PathString)
{
    BY_HANDLE_FILE_INFORMATION FI;


    if (!FileGetAttributes(PathString, &FI))
        return FALSE;

    return ((FI.dwFileAttributes != INVALID_FILE_ATTRIBUTES) && !(FI.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL PrivilegeGet(LPCWSTR lpName)
{
    BOOL Result = TRUE;
    HANDLE hToken = INVALID_HANDLE_VALUE;
    PTOKEN_PRIVILEGES ns = _alloca(sizeof(DWORD) + sizeof(LUID_AND_ATTRIBUTES) + 2);


    if (!ns)
        ExitFunction(FALSE);

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
        ExitFunction(FALSE);

    if (!LookupPrivilegeValue(NULL, lpName, &(ns->Privileges[0].Luid)))
        ExitFunction(FALSE);

    ns->PrivilegeCount = 1;
    ns->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(hToken, FALSE, ns, 0, NULL, NULL))
        ExitFunction(FALSE);


function_end:
    CloseHandle(hToken);

    return Result;
}
