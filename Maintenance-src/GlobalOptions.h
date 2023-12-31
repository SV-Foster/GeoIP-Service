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
	OperatingModeInstall,
	OperatingModeUninstall
} TOperatingMode;

typedef struct _GlobalOptions
{
	TOperatingMode OperatingMode;
	LPCTSTR PathToServiceEXE;
	BOOL NoCopyrightLogo;
} TGlobalOptions, *PGlobalOptions;
