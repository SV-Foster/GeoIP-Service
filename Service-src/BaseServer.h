/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


class IDBFilesConfigUpdater
{
public:
    virtual std::wstring DatabasePathGet() const abstract;
    virtual BOOL DatabaseFileNameGeoSet(std::wstring& NewFileName) const abstract;
    virtual BOOL DatabaseFileNameASNSet(std::wstring& NewFileName) const abstract;
};

class TBaseServer
{
public:
    TBaseServer(TDatabaseManager* DB, IDBFilesConfigUpdater* CU) noexcept;
    virtual ~TBaseServer() noexcept;
    TBaseServer(const TBaseServer& oth) = delete;
    TBaseServer& operator=(const TBaseServer& oth) = delete;
    TBaseServer(TBaseServer&& oth) = delete;
    TBaseServer& operator=(TBaseServer&& oth) = delete;


protected:
    TDatabaseManager* Database;
    IDBFilesConfigUpdater* ConfUpdate;


    BOOL RequestProcess(LPVOID BufferIn, DWORD BufferInSize, LPVOID BufferOut, DWORD BufferOutSize, PDWORD BufferOutDataSize) const;
    BOOL ResponseGeoIPForm(CONST PBYTE IPAddr, ADDRESS_FAMILY IPFamily, PGeoIPSRVResponseGeoIP Res) const noexcept;
    BOOL ResponseASNForm(CONST PBYTE IPAddr, ADDRESS_FAMILY IPFamily, PGeoIPSRVResponseASN Res) const noexcept;
    BOOL ResponseValueGet(PGeoIPSRVResponseGeoIP Res, DWORD Val, MMDB_entry_s* const Entry) const noexcept;
};
