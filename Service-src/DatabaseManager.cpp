/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#define _WINSOCKAPI_
#include <windows.h>
#include <string>
#include <codecvt>
#include <atomic>
#include "SharedHeaders.h"
#include "SharedOptionsDefaults.h"
#include "maxminddb.h"
#include "StringCoverter.h"
#include "DatabaseManager.h"


TDatabaseManager::TDatabaseManager():
	mmdbGeo({0}),
	mmdbASN({0}),
    LastMMDBError(0),
    UsingDB(0)
{    
    this->InHotplugging = CreateEvent
    (
        NULL,    // default security attributes
        TRUE,    // manual reset event
        TRUE,    // signaled
        NULL     // no name
    );
    if (!this->InHotplugging)
        throw TDatabaseManagerCreateException();
}

TDatabaseManager::~TDatabaseManager() noexcept
{
	Close();
    CloseHandle(this->InHotplugging);
}

BOOL TDatabaseManager::Open(const std::wstring& GeoPath, const std::wstring& ASNPath)
{
    return OpenFiles(GeoPath, &this->mmdbGeo, ASNPath, &this->mmdbASN);
}

BOOL TDatabaseManager::OpenFiles(const std::wstring& GeoPath, MMDB_s* Geo, const std::wstring& ASNPath, MMDB_s* ASN) const
{
    BOOL Result = TRUE;    
    std::string GeoPathANSI;
    std::string ASNPathANSI;
    int status;


    GeoPathANSI = TStringConverter::UCS2toANSI(GeoPath);
    ASNPathANSI = TStringConverter::UCS2toANSI(ASNPath);

    ZeroMemory(Geo, sizeof(MMDB_s));
    ZeroMemory(ASN, sizeof(MMDB_s));

    status = MMDB_open(GeoPathANSI.c_str(), MMDB_MODE_MMAP, Geo);
    if (status != MMDB_SUCCESS)
        ExitFunction(FALSE);

    status = MMDB_open(ASNPathANSI.c_str(), MMDB_MODE_MMAP, ASN);
    if (status != MMDB_SUCCESS)
        ExitFunction(FALSE);


function_end:
    if (!Result)
    {
        CloseAndNil(Geo);
        CloseAndNil(ASN);
    }

    return Result;
}

BOOL TDatabaseManager::LookupGeo(const struct sockaddr* const sockaddr, MMDB_lookup_result_s* Data) noexcept
{
    return Lookup(&this->mmdbGeo, sockaddr, Data);
}

BOOL TDatabaseManager::LookupASN(const struct sockaddr* const sockaddr, MMDB_lookup_result_s* Data) noexcept
{
    return Lookup(&this->mmdbASN, sockaddr, Data);
}

BOOL TDatabaseManager::Lookup(const MMDB_s* const mmdb, const struct sockaddr* const sockaddr, MMDB_lookup_result_s* Data) noexcept
{
    BOOL Result;


    // wait, if hotplugging
    WaitForSingleObject(this->InHotplugging, INFINITE);
    ++(this->UsingDB); // memory_order_seq_cst

    // get data    
    *Data = MMDB_lookup_sockaddr(mmdb, sockaddr, &this->LastMMDBError);
    Result = (this->LastMMDBError == MMDB_SUCCESS);
    
    --(this->UsingDB);
    return Result;
}

BOOL TDatabaseManager::Reopen(const std::wstring& GeoPath, const std::wstring& ASNPath, const BOOL RestoreOnFailure)
{
    MMDB_s BufferGeo;
    MMDB_s BufferASN;
    BOOL CallResult;


    CallResult = OpenFiles(GeoPath, &BufferGeo, ASNPath, &BufferASN);

    if (RestoreOnFailure)
    {        
        if (CallResult)
        {
            Close();
            this->mmdbASN = BufferASN;
            this->mmdbGeo = BufferGeo;            
        }

        return CallResult;
    }

    // not RestoreOnFailure
    Close();
    if (CallResult)
    {
        this->mmdbASN = BufferASN;
        this->mmdbGeo = BufferGeo;
    }
    
    return CallResult;
}

BOOL TDatabaseManager::Hotplug(const std::wstring& GeoPath, const  std::wstring& ASNPath, const BOOL RestoreOnFailure)
{    
    BOOL Result;


    // pause new client threads to access the database and wait for threads already accessing to finish
    ResetEvent(this->InHotplugging);
    WaitAllClientsToFinish();

    // open new db file
    Result = Reopen(GeoPath, ASNPath, RestoreOnFailure);
    
    // restore normal operation
    SetEvent(this->InHotplugging);

    return Result;
}

BOOL TDatabaseManager::Close() noexcept
{
    WaitAllClientsToFinish();

    CloseAndNil(&this->mmdbGeo);
    CloseAndNil(&this->mmdbASN);

    return TRUE;
}

VOID TDatabaseManager::CloseAndNil(MMDB_s* Ptr) const noexcept
{
    MMDB_close(Ptr);
    ZeroMemory(Ptr, sizeof(MMDB_s));
}

VOID TDatabaseManager::WaitAllClientsToFinish() const noexcept
{
    // spinwait ;D
    while (this->UsingDB) // memory_order_seq_cst
        Sleep(10);
}
