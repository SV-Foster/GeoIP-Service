/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


class TEventLogShared
{
public:
	TEventLogShared() = delete;
	~TEventLogShared() = delete;
	TEventLogShared(const TEventLogShared& oth) = delete;
	TEventLogShared& operator=(const TEventLogShared& oth) = delete;
	TEventLogShared(TEventLogShared&& oth) = delete;
	TEventLogShared& operator=(TEventLogShared&& oth) = delete;


	static WORD LogTypeOfMsgGet(DWORD MessageID) noexcept;
};
