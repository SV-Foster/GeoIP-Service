/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#include <Windows.h>
#include "EventLogShared.h"


WORD TEventLogShared::LogTypeOfMsgGet(DWORD MessageID) noexcept
{
    switch (MessageID & 0xC0000000)
    {
    case 0x00000000:
        return EVENTLOG_SUCCESS; // Information event

    case 0x40000000:
        return EVENTLOG_INFORMATION_TYPE; // Information event

    case 0x80000000:
        return EVENTLOG_WARNING_TYPE; // Warning event

    default: // 0xC0000000
        return EVENTLOG_ERROR_TYPE; // Error event
    }
}
