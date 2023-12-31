/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#define _WINSOCKAPI_
#include <windows.h>
#include <string>
#include <locale>
#include "SharedHeaders.h"
#include "maxminddb.h"
#include "Protocol.h"
#include "DatabaseManager.h"
#include "BaseServer.h"


TBaseServer::TBaseServer(TDatabaseManager* DB, IDBFilesConfigUpdater* CU) noexcept:
    Database(DB),
    ConfUpdate(CU)
{
    return; // NOP
}

TBaseServer::~TBaseServer() noexcept
{
    return; // NOP
}

BOOL TBaseServer::RequestProcess(LPVOID BufferIn, DWORD BufferInSize, LPVOID BufferOut, DWORD BufferOutSize, PDWORD BufferOutDataSize) const
{
    *BufferOutDataSize = 0;
    CONST PCGeoIPSRVSharedHeader Header = static_cast<PCGeoIPSRVSharedHeader>(BufferIn);

    if (BufferInSize < sizeof(TGeoIPSRVSharedHeader))
        return FALSE;
    if (BufferInSize < Header->Length)
        return FALSE;

    switch (Header->Type)
    {
    case GeoIPSRVRequestPing:
    {
        if (sizeof(TGeoIPSRVRequestPing) != Header->Length)
            return FALSE;
        if (sizeof(TGeoIPSRVResponsePong) > BufferOutSize)
            return FALSE;

        CONST PGeoIPSRVResponsePong Reply = static_cast<PGeoIPSRVResponsePong>(BufferOut);


        Reply->Header.Type = GeoIPSRVReplyPong;
        Reply->Header.Length = sizeof(TGeoIPSRVResponsePong);
        *BufferOutDataSize = sizeof(TGeoIPSRVResponsePong);

        break;
    }

    case GeoIPSRVRequestGeoIPv4:
    {
        if (sizeof(TGeoIPSRVRequestGeoIPv4) != Header->Length)
            return FALSE;
        if (sizeof(TGeoIPSRVResponseGeoIP) > BufferOutSize)
            return FALSE;

        CONST PCGeoIPSRVRequestGeoIPv4 Request = static_cast<PCGeoIPSRVRequestGeoIPv4>(BufferIn);
        CONST PGeoIPSRVResponseGeoIP Reply = static_cast<PGeoIPSRVResponseGeoIP>(BufferOut);
        BYTE IPAddr[4];
        constexpr ADDRESS_FAMILY IPFamily = AF_INET;

                
        ZeroMemory(Reply, sizeof(TGeoIPSRVResponseGeoIP));
        Reply->Header.Type = GeoIPSRVReplyGeoIP;
        Reply->Header.Length = sizeof(TGeoIPSRVResponseGeoIP);
        *BufferOutDataSize = sizeof(TGeoIPSRVResponseGeoIP);
        memcpy_s(&IPAddr[0], 4, &Request->Addr[0], 4);
        if (ResponseGeoIPForm(&IPAddr[0], IPFamily, Reply))
            Reply->Flags |= ReplySharedFlagsDBOperational;

        break;
    }

    case GeoIPSRVRequestGeoIPv6:
    {
        if (sizeof(TGeoIPSRVRequestGeoIPv6) != Header->Length)
            return FALSE;
        if (sizeof(TGeoIPSRVResponseGeoIP) > BufferOutSize)
            return FALSE;

        CONST PCGeoIPSRVRequestGeoIPv6 Request = static_cast<PCGeoIPSRVRequestGeoIPv6>(BufferIn);
        CONST PGeoIPSRVResponseGeoIP Reply = static_cast<PGeoIPSRVResponseGeoIP>(BufferOut);
        BYTE IPAddr[16];
        constexpr ADDRESS_FAMILY IPFamily = AF_INET6;


        ZeroMemory(Reply, sizeof(TGeoIPSRVResponseGeoIP));
        Reply->Header.Type = GeoIPSRVReplyGeoIP;
        Reply->Header.Length = sizeof(TGeoIPSRVResponseGeoIP);
        *BufferOutDataSize = sizeof(TGeoIPSRVResponseGeoIP);
        memcpy_s(&IPAddr[0], 16, &Request->Addr[0], 16);
        if (ResponseGeoIPForm(&IPAddr[0], IPFamily, Reply))
            Reply->Flags |= ReplySharedFlagsDBOperational;

        break;
    }

    case GeoIPSRVRequestHotplug:
    {
        if (sizeof(TGeoIPSRVRequestHotplug) != Header->Length)
            return FALSE;
        if (sizeof(TGeoIPSRVResponseHotplug) > BufferOutSize)
            return FALSE;

        CONST PCGeoIPSRVRequestHotplug Request = static_cast<PCGeoIPSRVRequestHotplug>(BufferIn);
        CONST PGeoIPSRVResponseHotplug Reply = static_cast<PGeoIPSRVResponseHotplug>(BufferOut);
        BOOL CallResult;
        CONST BOOL RestoreOnFailure = Request->Flags & DBHotplugFlagsRestoreOnFailure;
        std::wstring FileGeoNew(&(Request->Geo)[0]);
        std::wstring FileASNNew(&(Request->ASN)[0]);


        CallResult = this->Database->Hotplug
        (
            this->ConfUpdate->DatabasePathGet() + FileGeoNew,
            this->ConfUpdate->DatabasePathGet() + FileASNNew,
            RestoreOnFailure
        );
        if (CallResult)
        {
            this->ConfUpdate->DatabaseFileNameGeoSet(FileGeoNew);
            this->ConfUpdate->DatabaseFileNameASNSet(FileASNNew);
        }
        
        Reply->Header.Type = GeoIPSRVReplyHotplug;
        Reply->Header.Length = sizeof(TGeoIPSRVResponseHotplug);
        Reply->Result = static_cast<DWORD>(CallResult);
        *BufferOutDataSize = sizeof(TGeoIPSRVResponseHotplug);

        break;
    }

    case GeoIPSRVRequestASNIPv4:
    {
        if (sizeof(TGeoIPSRVRequestASNIPv4) != Header->Length)
            return FALSE;
        if (sizeof(TGeoIPSRVResponseASN) > BufferOutSize)
            return FALSE;

        CONST PCGeoIPSRVRequestASNIPv4 Request = static_cast<PCGeoIPSRVRequestASNIPv4>(BufferIn);
        CONST PGeoIPSRVResponseASN Reply = static_cast<PGeoIPSRVResponseASN>(BufferOut);
        BYTE IPAddr[4];
        constexpr ADDRESS_FAMILY IPFamily = AF_INET;


        ZeroMemory(Reply, sizeof(TGeoIPSRVResponseASN));
        Reply->Header.Type = GeoIPSRVReplyASN;
        Reply->Header.Length = sizeof(TGeoIPSRVResponseASN);
        *BufferOutDataSize = sizeof(TGeoIPSRVResponseASN);
        memcpy_s(&IPAddr[0], 4, &Request->Addr[0], 4);
        if (ResponseASNForm(&IPAddr[0], IPFamily, Reply))
            Reply->Flags |= ReplySharedFlagsDBOperational;

        break;
    }

    case GeoIPSRVRequestASNIPv6:
    {
        if (sizeof(TGeoIPSRVRequestASNIPv6) != Header->Length)
            return FALSE;
        if (sizeof(TGeoIPSRVResponseASN) > BufferOutSize)
            return FALSE;

        CONST PCGeoIPSRVRequestASNIPv6 Request = static_cast<PCGeoIPSRVRequestASNIPv6>(BufferIn);
        CONST PGeoIPSRVResponseASN Reply = static_cast<PGeoIPSRVResponseASN>(BufferOut);
        BYTE IPAddr[16];
        constexpr ADDRESS_FAMILY IPFamily = AF_INET6;


        ZeroMemory(Reply, sizeof(TGeoIPSRVResponseASN));
        Reply->Header.Type = GeoIPSRVReplyASN;
        Reply->Header.Length = sizeof(TGeoIPSRVResponseASN);
        *BufferOutDataSize = sizeof(TGeoIPSRVResponseASN);
        memcpy_s(&IPAddr[0], 16, &Request->Addr[0], 16);
        if (ResponseASNForm(&IPAddr[0], IPFamily, Reply))
            Reply->Flags |= ReplySharedFlagsDBOperational;

        break;
    }

    default:
        return FALSE;
    }

    return TRUE;
}

BOOL TBaseServer::ResponseGeoIPForm(CONST PBYTE IPAddr, ADDRESS_FAMILY IPFamily, PGeoIPSRVResponseGeoIP Res) const noexcept
{
    BOOL Result = TRUE;
    MMDB_lookup_result_s LookupGeo;
    struct sockaddr_in SA4;
    struct sockaddr_in6 SA6;


    // get data from DB
    switch (IPFamily)
    {
    case AF_INET:
        memcpy_s(&SA4.sin_addr.s_addr, 4, IPAddr, 4);
        SA4.sin_family = IPFamily;
        if (!Database->LookupGeo(reinterpret_cast<const struct sockaddr* const>(&SA4), &LookupGeo))
            ExitFunction(FALSE);

        break;

    case AF_INET6:
        memcpy_s(&SA6.sin6_addr.s6_addr[0], 16, IPAddr, 16);
        SA6.sin6_family = IPFamily;
        if (!Database->LookupGeo(reinterpret_cast<const struct sockaddr* const>(&SA6), &LookupGeo))
            ExitFunction(FALSE);

        break;

    default:
        ExitFunction(FALSE);
    }


    // if nothing in DB just return
    if (!LookupGeo.found_entry)
        ExitFunction(TRUE);

    // now filling values
    ResponseValueGet(Res, ReplyGeoIPFlagsFoundContinent, &LookupGeo.entry);
    ResponseValueGet(Res, ReplyGeoIPFlagsFoundCountry, &LookupGeo.entry);
    ResponseValueGet(Res, ReplyGeoIPFlagsFoundCity, &LookupGeo.entry);
    ResponseValueGet(Res, ReplyGeoIPFlagsFoundLocation, &LookupGeo.entry);


function_end:
    return Result;
}

BOOL TBaseServer::ResponseASNForm(CONST PBYTE IPAddr, ADDRESS_FAMILY IPFamily, PGeoIPSRVResponseASN Res) const noexcept
{
    BOOL Result = TRUE;
    MMDB_lookup_result_s LookupASN;
    struct sockaddr_in SA4;
    struct sockaddr_in6 SA6;
    MMDB_entry_data_s BufferData;
    int status;


    // get data from DB
    switch (IPFamily)
    {
    case AF_INET:
        memcpy_s(&SA4.sin_addr.s_addr, 4, IPAddr, 4);
        SA4.sin_family = IPFamily;
        if (!Database->LookupASN(reinterpret_cast<const struct sockaddr* const>(&SA4), &LookupASN))
            ExitFunction(FALSE);

        break;

    case AF_INET6:
        memcpy_s(&SA6.sin6_addr.s6_addr[0], 16, IPAddr, 16);
        SA6.sin6_family = IPFamily;
        if (!Database->LookupASN(reinterpret_cast<const struct sockaddr* const>(&SA6), &LookupASN))
            ExitFunction(FALSE);

        break;

    default:
        ExitFunction(FALSE);
    }


    // if nothing in DB just return
    if (!LookupASN.found_entry)
        ExitFunction(TRUE);

    // now filling values
    status = MMDB_get_value(&LookupASN.entry, &BufferData, u8"autonomous_system_number", NULL);
    // check for errors
    // those errors are insignificant, not real errors to worry about, so return TRUE, just don't fill values in Response w/garbage
    if (status != MMDB_SUCCESS)
        ExitFunction(TRUE);
    if (!BufferData.has_data)
        ExitFunction(TRUE);

    // mark value
    Res->ASN = BufferData.uint32;
    Res->Flags = ReplyASNFlagsFoundASN;


function_end:
    return Result;
}

BOOL TBaseServer::ResponseValueGet(PGeoIPSRVResponseGeoIP Res, DWORD Val, MMDB_entry_s* const Entry) const noexcept
{
    MMDB_entry_data_s BufferData;
    int status;


    // call DB and save value
    switch (Val)
    {
    case ReplyGeoIPFlagsFoundContinent:
        status = MMDB_get_value(Entry, &BufferData, u8"continent", u8"geoname_id", NULL);
        // check for errors
        if (status != MMDB_SUCCESS)
            return FALSE;
        if (!BufferData.has_data)
            return TRUE;

        Res->ContinentID = BufferData.uint32;

        break;

    case ReplyGeoIPFlagsFoundCountry:
        status = MMDB_get_value(Entry, &BufferData, u8"country", u8"geoname_id", NULL);
        // check for errors
        if (status != MMDB_SUCCESS)
            return FALSE;
        if (!BufferData.has_data)
            return TRUE;

        Res->CountryID = BufferData.uint32;

        break;

    case ReplyGeoIPFlagsFoundCity:
        status = MMDB_get_value(Entry, &BufferData, u8"city", u8"geoname_id", NULL);
        // check for errors
        if (status != MMDB_SUCCESS)
            return FALSE;
        if (!BufferData.has_data)
            return TRUE;

        Res->CityID = BufferData.uint32;

        break;

    case ReplyGeoIPFlagsFoundLocation:
        status = MMDB_get_value(Entry, &BufferData, u8"location", u8"accuracy_radius", NULL);
        // check for errors
        if (status != MMDB_SUCCESS)
            return FALSE;
        if (!BufferData.has_data)
            return TRUE;

        Res->Location.accuracy_radius = BufferData.uint16;

        status = MMDB_get_value(Entry, &BufferData, u8"location", u8"latitude", NULL);
        // check for errors
        if (status != MMDB_SUCCESS)
            return FALSE;
        if (!BufferData.has_data)
            return FALSE;

        Res->Location.latitude = BufferData.double_value;

        status = MMDB_get_value(Entry, &BufferData, u8"location", u8"longitude", NULL);
        // check for errors
        if (status != MMDB_SUCCESS)
            return FALSE;
        if (!BufferData.has_data)
            return FALSE;

        Res->Location.longitude = BufferData.double_value;

        break;

    default:
        return FALSE;
    }

    // mark value
    Res->Flags |= Val;

    return TRUE;
}
