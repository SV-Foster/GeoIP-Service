/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#define _WINSOCKAPI_
#include <windows.h>
#include <tchar.h>
#include <sddl.h>
#include <string>
#include <locale>
#include <vector>
#include "SharedHeaders.h"
#include "SharedOptionsDefaults.h"
#include "maxminddb.h"
#include "Protocol.h"
#include "DatabaseManager.h"
#include "BaseServer.h"
#include "PIPEServer.h"
#include "TCPServer.h"
#include "GlobalOptions.h"


TGlobalOptions::TGlobalOptions():
	RegistryKey(0),
	ServerOptionsRecords(),
	InServerOptionsEnum(FALSE),
	ServerOptionsEnumIndex(0)
{
	LSTATUS RegCall;


	RegCall = RegOpenKeyEx
	(
		RegistryOptionsKey,
		RegistryOptionsPath,
		0,
		KEY_READ | KEY_WRITE,
		&this->RegistryKey			// A pointer to a variable that receives a handle to the opened or created key
	);	
	if (RegCall != ERROR_SUCCESS)
		throw GlobalOptionsInitException();
}

TGlobalOptions::~TGlobalOptions() noexcept
{
	RegCloseKey(this->RegistryKey);
	ServerOptionsEnumReset();
}

BOOL TGlobalOptions::StringRead(const std::wstring& SubKey, const std::wstring& ValName, std::wstring& Data) const
{	
	BOOL Result;
	LSTATUS CallResult;
	TCHAR Buffer[256];
	DWORD DataWrote = sizeof(Buffer);

	
	CallResult = RegGetValue
	(
		this->RegistryKey,
		SubKey.c_str(),
		ValName.c_str(),
		RRF_RT_REG_SZ,
		NULL,
		reinterpret_cast<LPBYTE>(Buffer),
		&DataWrote
	);
	Result = (CallResult == ERROR_SUCCESS);
	if (Result)
		Data = std::wstring(&Buffer[0]);

	return Result;
}

std::wstring TGlobalOptions::StringReadDefault(const std::wstring& SubKey, const std::wstring& ValName, const std::wstring& DefValue) const
{
	std::wstring Buffer;

	if (StringRead(SubKey, ValName, Buffer))
		return Buffer;
	else
		return DefValue;
}

BOOL TGlobalOptions::StringWrite(const std::wstring& SubKey, const std::wstring& ValName, std::wstring& Value) const noexcept
{
	LSTATUS CallResult;

	CallResult = RegSetKeyValue
	(
		this->RegistryKey,
		SubKey.c_str(),
		ValName.c_str(),
		REG_EXPAND_SZ,
		reinterpret_cast<LPBYTE>(&Value[0]),
		static_cast<DWORD>(Value.size() + 1) * sizeof(TCHAR)
	);

	return (CallResult == ERROR_SUCCESS);
}

BOOL TGlobalOptions::DWORDRead(const std::wstring& SubKey, const std::wstring& ValName, PDWORD data) const noexcept
{
	LSTATUS CallResult;
	DWORD DataWrote = sizeof(DWORD);


	CallResult = RegGetValue
	(
		this->RegistryKey,
		SubKey.c_str(),
		ValName.c_str(),
		RRF_RT_REG_DWORD,
		NULL,
		reinterpret_cast<LPBYTE>(data),
		&DataWrote
	);

	return (CallResult == ERROR_SUCCESS);
}

DWORD TGlobalOptions::DWORDReadDefault(const std::wstring& SubKey, const std::wstring& ValName, DWORD DefValue) const noexcept
{
	DWORD Result;

	if (DWORDRead(SubKey, ValName, &Result))
		return Result;
	else
		return DefValue;
}

std::wstring TGlobalOptions::InstalledPathGet() const
{
	return StringReadDefault(TEXT(""), RegistryValInstalledPath, TEXT(""));
}

std::wstring TGlobalOptions::DatabasePathGet() const
{
	return StringReadDefault(TEXT(""), RegistryValDatabasePath, TEXT(""));
}

std::wstring TGlobalOptions::DatabaseFileNameGeoGet() const
{
	return StringReadDefault(TEXT(""), RegistryValDatabaseFileNameGeo, TEXT(""));
}

BOOL TGlobalOptions::DatabaseFileNameGeoSet(std::wstring& NewFileName) const
{
	return StringWrite(TEXT(""), RegistryValDatabaseFileNameGeo, NewFileName);
}

std::wstring TGlobalOptions::DatabaseFileNameASNGet() const
{
	return StringReadDefault(TEXT(""), RegistryValDatabaseFileNameASN, TEXT(""));
}

BOOL TGlobalOptions::DatabaseFileNameASNSet(std::wstring& NewFileName) const
{
	return StringWrite(TEXT(""), RegistryValDatabaseFileNameASN, NewFileName);
}

std::wstring TGlobalOptions::DatabaseFileFullPathGeoGet() const
{
	std::wstring Result = DatabasePathGet() + DatabaseFileNameGeoGet();
	return Result;
}

std::wstring TGlobalOptions::DatabaseFileFullPathASNGet() const
{
	std::wstring Result = DatabasePathGet() + DatabaseFileNameASNGet();
	return Result;
}

VOID TGlobalOptions::ServerOptionsEnumReset() noexcept
{
	this->ServerOptionsRecords.clear();
	this->InServerOptionsEnum = FALSE;
	this->ServerOptionsEnumIndex = 0;
}

BOOL TGlobalOptions::ServerOptionsEnumNext(std::shared_ptr<TServerOptionsBase>& data)
{
	data.reset();
	std::wstring RegPath(RegistryOptionsServerList);
	DWORD Type;
	DWORD Enabled;


	// first call
	if (!this->InServerOptionsEnum)
	{
		if (!ServerOptionsEnumCollect())
			return FALSE;
		this->InServerOptionsEnum = TRUE;
	};

	// no more items, call ServerOptionsEnumReset to restart
	if (ServerOptionsEnumIndex >= this->ServerOptionsRecords.size())
		return FALSE;

	// get the item from the registry
	RegPath += TEXT("\\") + ServerOptionsRecords[ServerOptionsEnumIndex];

	// get basic parameters
	if (!DWORDRead(RegPath, TEXT("Type"), &Type))
		return FALSE;

	Enabled = DWORDReadDefault(RegPath, TEXT("Enabled"), FALSE);

	// not construct transport objects and fill them
	switch (Type)
	{
	case ServerTypePIPE:
	{
		std::shared_ptr<TServerOptionsPIPE> n = std::make_shared<TServerOptionsPIPE>();
		n->Type = Type;
		n->Enabled = Enabled;
		n->PipeName = StringReadDefault(RegPath, TEXT("PipeName"), DefaultPipeName);
		n->SecurityAttributesPolicy = DWORDReadDefault(RegPath, TEXT("SecurityAttributesPolicy"), SecurityAttributesPolicyStandard);
		n->SecurityAttributesCustom = StringReadDefault(RegPath, TEXT("SecurityAttributesCustom"), TEXT(""));
		n->IOTimeoutMS = DWORDReadDefault(RegPath, TEXT("TimeoutIOMS"), TimeoutIODefaultMS);

		data = n;
		break;
	}

	case ServerTypeTCP4:
	case ServerTypeTCP6:
	{
		std::shared_ptr<TServerOptionsTCP> n = std::make_shared<TServerOptionsTCP>();
		n->Type = Type;
		n->Enabled = Enabled;
		n->Address = StringReadDefault(RegPath, TEXT("Address"), DefaultTCPServer);
		n->Port = static_cast<WORD>(DWORDReadDefault(RegPath, TEXT("Port"), DefaultTCPPort));
		n->TimeoutRecieveMS = DWORDReadDefault(RegPath, TEXT("TimeoutRecieveMS"), TCPIOTimeoutDefault);
		n->TimeoutSendMS = DWORDReadDefault(RegPath, TEXT("TimeoutSendMS"), TCPIOTimeoutDefault);

		data = n;
		break;
	}

	default:
		// unknown or unsupported type
		return FALSE;
	}

	// done!
	++ServerOptionsEnumIndex;
	return TRUE;
}

BOOL TGlobalOptions::ServerOptionsEnumCollect()
{
	BOOL Result = TRUE;
	LSTATUS RegCall;
	HKEY HandleList = 0;
	TCHAR BufferStr[256];
	DWORD BufferSize; // size of the buffer specified by the lpName parameter, in characters
	DWORD Index = 0;


	// open the list Key
	RegCall = RegOpenKeyEx
	(
		this->RegistryKey,
		RegistryOptionsServerList,
		0,
		KEY_READ,
		&HandleList
	);

	if (RegCall != ERROR_SUCCESS)
		ExitFunction(FALSE);


	// read list of servers
	do
	{
		BufferSize = sizeof(BufferStr) / sizeof(TCHAR);

		RegCall = RegEnumKeyEx
		(
			HandleList,
			Index,
			&BufferStr[0],
			&BufferSize,
			NULL,
			NULL,
			NULL,
			NULL
		);
		// done
		if (RegCall == ERROR_NO_MORE_ITEMS)
			break;
		// error
		if (RegCall != ERROR_SUCCESS)
			ExitFunction(FALSE);

		// add new string w/name of the record
		this->ServerOptionsRecords.emplace_back(&BufferStr[0]);

		++Index;

	} while (true);
	

function_end:
	if (HandleList)
		RegCloseKey(HandleList);

	return Result;
}

TServerOptionsBase::TServerOptionsBase() noexcept:
	Type(ServerTypePIPE),
	Enabled(FALSE)
{
	return; // NOP
};

TServerOptionsPIPE::TServerOptionsPIPE() noexcept:
	TServerOptionsBase::TServerOptionsBase(), // inherited Create
	PipeName(),
	SecurityAttributesPolicy(SecurityAttributesPolicyStandard),
	SecurityAttributesCustom(),
	IOTimeoutMS(TimeoutIODefaultMS)
{
	return; // NOP
};

TServerOptionsTCP::TServerOptionsTCP() noexcept:
	TServerOptionsBase::TServerOptionsBase(), // inherited Create
	Address(),
	Port(0),
	TimeoutRecieveMS(TCPIOTimeoutDefault),
	TimeoutSendMS(TCPIOTimeoutDefault)
{
	return; // NOP
};
