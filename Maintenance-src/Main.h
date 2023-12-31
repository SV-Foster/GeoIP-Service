/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


#define RegEventPath TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s")


DWORD ModeHelp();
DWORD ModeInstall();
DWORD ModeUninstall();

BOOL  SvcInstall(LPCTSTR Path);
BOOL  SvcUninstall();
BOOL  AddEventSource(LPCTSTR Path, DWORD CategoryCount);
BOOL  RemoveEventSource();
BOOL  SetOptions(LPCTSTR Path);
BOOL  RemoveOptions();

BOOL  FileGetAttributes(LPCTSTR PathString, LPBY_HANDLE_FILE_INFORMATION FileInformation);
BOOL  FileExists(LPCTSTR PathString);
BOOL  PrivilegeGet(LPCWSTR lpName);
