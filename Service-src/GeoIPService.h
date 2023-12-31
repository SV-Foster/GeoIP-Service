/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


#define SvcReportStatusTimeHint 3000


class TPIPEServerEvents:
	public IPIPEServerEvents
{
public:
	TPIPEServerEvents(std::shared_ptr<TEventLogWriter> EventLogWriter) noexcept;


	VOID OnMainThreadStart(std::wstring& PIPEName) const final;
	VOID OnMainThreadExit(std::wstring& PIPEName, DWORD Result, HANDLE StopEvent) const final;

	VOID OnWorkerThreadStart(std::wstring& PIPEName) const final;
	VOID OnWorkerThreadExit(std::wstring& PIPEName, DWORD Result, HANDLE StopEvent) const final;


private:
	std::shared_ptr<TEventLogWriter> EventLogWriter;

};

class TTCPServerEvents:
	public ITCPServerEvents
{
public:
	TTCPServerEvents(std::shared_ptr<TEventLogWriter> EventLogWriter) noexcept;


	VOID OnMainThreadStart(std::wstring& IPAddr, WORD IPPort) const final;
	VOID OnMainThreadExit(std::wstring& IPAddr, WORD IPPort, DWORD Result, HANDLE StopEvent) const final;

	VOID OnShutdownThreadStart(std::wstring& IPAddr, WORD IPPort) const final;
	VOID OnShutdownThreadExit(std::wstring& IPAddr, WORD IPPort, HANDLE StopEvent) const final;

	VOID OnWorkerThreadStart(std::wstring& IPAddr, WORD IPPort) const final;
	VOID OnWorkerThreadExit(std::wstring& IPAddr, WORD IPPort, DWORD Result, HANDLE StopEvent) const final;


private:
	std::shared_ptr<TEventLogWriter> EventLogWriter;

};

class TGeoIPService
{
public:
	TGeoIPService(std::wstring& NameShort);
	virtual ~TGeoIPService() noexcept;
	TGeoIPService(const TGeoIPService& oth) = delete;
	TGeoIPService(TGeoIPService&& oth) = delete;
	TGeoIPService& operator=(const TGeoIPService& oth) = delete;
	TGeoIPService& operator=(TGeoIPService&& oth) = delete;

	virtual DWORD Run();


protected:
	std::wstring NameShort;
	SERVICE_STATUS SvcStatus;
	SERVICE_STATUS_HANDLE SvcStatusHandle;
	HANDLE SvcStopEvent;
	std::shared_ptr<TEventLogWriter> EventLogWriter;	
	TDatabaseManager Database;
	std::unique_ptr<TGlobalOptions> GlobalOptions;
	std::vector<std::shared_ptr<TBaseServer>> ServersArray;
	std::shared_ptr<TPIPEServerEvents> PIPEEvents;
	std::shared_ptr<TTCPServerEvents> TCPEvents;


	VOID  SvcMain(DWORD, LPTSTR*);	
	BOOL  SvcInit();
	BOOL  AppInit();
	BOOL  AppResume();
	BOOL  AppMain();
	BOOL  AppCease() noexcept;
	BOOL  AppCleanup();

	VOID  SvcCtrlHandler(DWORD);	
	VOID  SvcReportStatus(CONST DWORD CurrentState, CONST DWORD Win32ExitCode, CONST DWORD WaitHint) noexcept;
	BOOL  EventsInit();
	VOID  EventLog(CONST DWORD EventID) const;
	VOID  EventLogPIPEServerStart(CONST std::wstring& PIPEName, DWORD MainThreadID) const;
	VOID  EventLogTCPServerStart(CONST std::wstring& IPAddr, WORD IPPort, DWORD MainThreadID, DWORD ShutdwThreadID) const;

	static VOID WINAPI WinAPISvcCtrlHandler(DWORD);
	static VOID WINAPI WinAPISvcMain(_In_ DWORD dwArgc, _In_opt_ LPTSTR* pszArgv);	

};
