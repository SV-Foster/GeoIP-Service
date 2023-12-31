/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


typedef enum _OperatingMode
{
	OperatingModeHelp,
	OperatingModeIPRequest,
	OperatingModePing,
	OperatingModeDBHotplug,
	OperatingModeASNRequest
} TOperatingMode;

typedef enum _TransportToServer
{
	TransportToServerPIPE,
	TransportToServerTCP
} TTransportToServer, *PTransportToServer;

typedef struct _GlobalOptions
{
	TOperatingMode OperatingMode;
	BOOL NoCopyrightLogo;
	LPTSTR ProcessIP;	
	TTransportToServer TransportToServer;
	LPTSTR PipeName;
	DWORD PIPEIOTimeout;
	LPTSTR NetServerAdress;
	LPTSTR NetServerPort;
	int TCPVersion;
	long TCPConnectTimeoutSec;
	DWORD TCPIOTimeoutMSec; // Sets the timeout, in milliseconds, for blocking calls
	LPTSTR FileGeo;
	LPTSTR FileASN;
} TGlobalOptions, *PGlobalOptions;


VOID GlobalOptionsDefaultsSet(PGlobalOptions glo);
