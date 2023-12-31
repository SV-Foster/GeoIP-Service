/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


#define ServerTypePIPE 0x00
#define ServerTypeTCP4 0x01
#define ServerTypeTCP6 0x02

class TServerOptionsBase
{
public:
	TServerOptionsBase() noexcept;
	virtual ~TServerOptionsBase() = default;
	TServerOptionsBase(const TServerOptionsBase& oth) = default;
	TServerOptionsBase(TServerOptionsBase&& oth) = default;
	TServerOptionsBase& operator=(const TServerOptionsBase& oth) = default;
	TServerOptionsBase& operator=(TServerOptionsBase&& oth) = default;

	DWORD Type;
	BOOL Enabled;
};

class TServerOptionsPIPE:
	public TServerOptionsBase
{
public:
	TServerOptionsPIPE() noexcept;


	std::wstring PipeName;
	DWORD SecurityAttributesPolicy;
	std::wstring SecurityAttributesCustom;
	DWORD IOTimeoutMS;
};

class TServerOptionsTCP:
	public TServerOptionsBase
{
public:
	TServerOptionsTCP() noexcept;


	std::wstring Address;
	WORD Port;
	DWORD TimeoutRecieveMS;
	DWORD TimeoutSendMS;
};

class GlobalOptionsInitException:
	public std::exception
{
};

class TGlobalOptions:
	public IDBFilesConfigUpdater
{
public:
	TGlobalOptions();
	virtual ~TGlobalOptions() noexcept;
	TGlobalOptions(const TGlobalOptions& oth) = delete;
	TGlobalOptions& operator=(const TGlobalOptions& oth) = delete;
	TGlobalOptions(TGlobalOptions&& oth) = delete;
	TGlobalOptions& operator=(TGlobalOptions&& oth) = delete;


	std::wstring InstalledPathGet() const;
	std::wstring DatabasePathGet() const override;
	std::wstring DatabaseFileNameGeoGet() const;
	BOOL		 DatabaseFileNameGeoSet(std::wstring& NewFileName) const override;
	std::wstring DatabaseFileNameASNGet() const;
	BOOL		 DatabaseFileNameASNSet(std::wstring& NewFileName) const override;
	std::wstring DatabaseFileFullPathGeoGet() const;
	std::wstring DatabaseFileFullPathASNGet() const;

	VOID ServerOptionsEnumReset() noexcept;
	BOOL ServerOptionsEnumNext(std::shared_ptr<TServerOptionsBase>& data);


private:
	HKEY RegistryKey;
	BOOL InServerOptionsEnum;
	SIZE_T ServerOptionsEnumIndex;
	std::vector<std::wstring> ServerOptionsRecords;


	BOOL		 StringRead(const std::wstring& SubKey, const std::wstring& ValName, std::wstring& Data) const;
	std::wstring StringReadDefault(const std::wstring& SubKey, const std::wstring& ValName, const std::wstring& DefValue) const;
	BOOL		 StringWrite(const std::wstring& SubKey, const std::wstring& ValName, std::wstring& Value) const noexcept;
	BOOL		 DWORDRead(const std::wstring& SubKey, const std::wstring& ValName, PDWORD DefValue) const noexcept;
	DWORD		 DWORDReadDefault(const std::wstring& SubKey, const std::wstring& ValName, DWORD DefValue) const noexcept;

	BOOL ServerOptionsEnumCollect();
};
