/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#include <Windows.h>
#include <string>
#include <vector>
#include "EventLogShared.h"
#include "EventLogWriter.h"


TEventLogWriter::TEventLogWriter(std::wstring& SourceName):
	TEventLogWriter(nullptr, SourceName)
{
	return; // NOP	
}

TEventLogWriter::TEventLogWriter(std::wstring* UNCServerName, std::wstring& SourceName)
{
	// The Universal Naming Convention (UNC) name of the remote server on which this operation is to be performed
	// If this parameter is NULL, the local computer is used
	LPCTSTR PUNCServerName = NULL;
	if (UNCServerName)
		PUNCServerName = UNCServerName->c_str();

	this->EventHandle = RegisterEventSource(PUNCServerName, SourceName.c_str());
	if (!this->EventHandle) // If the function fails, the return value is NULL
		throw TEventLogWriterCreateException();
	if (this->EventHandle == reinterpret_cast<HANDLE>(ERROR_ACCESS_DENIED)) // The function returns ERROR_ACCESS_DENIED if lpSourceName specifies the Security event log
		throw TEventLogWriterCreateException();
}

TEventLogWriter::~TEventLogWriter() noexcept
{
	DeregisterEventSource(this->EventHandle);
}

TEventLogWriter::operator HANDLE() const noexcept
{
	return this->EventHandle;
}

TEventLogWriter& TEventLogWriter::operator<<(CONST DWORD EventID)
{
	this->Write(EventID);
	
	return *this;
}

BOOL TEventLogWriter::Write(CONST DWORD EventID) const
{
	std::vector<std::wstring> StubVector;

	return Write(EventID, StubVector);
}

BOOL TEventLogWriter::Write(CONST DWORD EventID, CONST std::initializer_list<std::wstring>& args) const
{
	CONST WORD EventStringsCnt = static_cast<WORD>(args.size() & 0xFF);
	std::vector<LPCTSTR> EventStrings;
	LPCTSTR* PPEventStrings = NULL;


	if (EventStringsCnt)
	{
		EventStrings.reserve(EventStringsCnt);
		for (CONST auto& i : std::initializer_list<std::wstring>{ args })
			EventStrings.push_back(i.data());
		PPEventStrings = &EventStrings[0];
	}

	return Write(EventID, EventStringsCnt, PPEventStrings);
}

BOOL TEventLogWriter::Write(CONST DWORD EventID, CONST std::vector<std::wstring>& StringsArray) const
{
	CONST WORD EventStringsCnt = static_cast<WORD>(StringsArray.size() & 0xFF);
	std::vector<LPCTSTR> EventStrings;
	LPCTSTR* PPEventStrings = NULL;


	if (EventStringsCnt)
	{		
		EventStrings.reserve(EventStringsCnt);
		for (CONST auto& i : StringsArray)
			EventStrings.push_back(i.data());
		PPEventStrings = &EventStrings[0];
	}

	return Write(EventID, EventStringsCnt, PPEventStrings);
}

BOOL TEventLogWriter::Write(CONST DWORD EventID, CONST WORD EventStringsCnt, LPCTSTR* PPStringsArray) const noexcept
{
	return ReportEvent
	(
		this->EventHandle,   // event log handle
		TEventLogShared::LogTypeOfMsgGet(EventID), // event type
		0,                   // event category
		EventID,             // event identifier
		NULL,                // no security identifier
		EventStringsCnt,     // size of lpszStrings array
		0,                   // no binary data
		PPStringsArray,      // array of strings
		NULL                 // no binary data
	);
}
