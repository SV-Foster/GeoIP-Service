/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


class TEventLogWriterCreateException:
	public std::exception
{
};

class TEventLogWriter
{
public:
	// SourceName is the name of the event source whose handle is to be retrieved. The source name must be a subkey of a log
	// under the Eventlog registry key. Note that the Security log is for system use only
	TEventLogWriter(std::wstring& SourceName);
	TEventLogWriter(std::wstring* UNCServerName, std::wstring& SourceName);
	~TEventLogWriter() noexcept;
	TEventLogWriter(const TEventLogWriter& oth) = delete;
	TEventLogWriter(TEventLogWriter&& oth) = delete;
	TEventLogWriter& operator=(const TEventLogWriter& oth) = delete;
	TEventLogWriter& operator=(TEventLogWriter&& oth) = delete;


	operator HANDLE() const noexcept;
	TEventLogWriter& operator<<(CONST DWORD EventID);

	BOOL Write(CONST DWORD EventID) const;
	BOOL Write(CONST DWORD EventID, CONST std::initializer_list<std::wstring>& args) const;
	BOOL Write(CONST DWORD EventID, CONST std::vector<std::wstring>& StringsArray) const;
	BOOL Write(CONST DWORD EventID, CONST WORD EventStringsCnt, LPCTSTR* PPStringsArray) const noexcept;
	

private:
	HANDLE EventHandle;

};
