/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once
#ifdef __cplusplus
	extern "C" {
#endif


#define GeoIPSRVRequestPing 0x00
#define GeoIPSRVRequestGeoIPv4 0x01
#define GeoIPSRVRequestGeoIPv6 0x02
#define GeoIPSRVRequestHotplug 0x03
#define GeoIPSRVRequestASNIPv4 0x04
#define GeoIPSRVRequestASNIPv6 0x05

#define GeoIPSRVReplyPong 0x00
#define GeoIPSRVReplyGeoIP 0x01
#define GeoIPSRVReplyHotplug 0x02
#define GeoIPSRVReplyASN 0x03

// shared reply flags
#define ReplySharedFlagsDBOperational 0x80000000

// for GeoIPSRVRequestGeoIP
#define ReplyGeoIPFlagsFoundContinent 0x01
#define ReplyGeoIPFlagsFoundCountry 0x02
#define ReplyGeoIPFlagsFoundCity 0x04
#define ReplyGeoIPFlagsFoundLocation 0x08

// for GeoIPSRVRequestASNIP
#define ReplyASNFlagsFoundASN 0x01


#define DBFileLengthMaxCharWZero 256
#define DBHotplugFlagsRestoreOnFailure 0x01


typedef struct _GeoIPSRVSharedHeader
{
	DWORD Type;
	DWORD Length;
} TGeoIPSRVSharedHeader, *PGeoIPSRVSharedHeader;
typedef CONST TGeoIPSRVSharedHeader TCGeoIPSRVSharedHeader;
typedef CONST TGeoIPSRVSharedHeader* PCGeoIPSRVSharedHeader;

//
// Requests
//
typedef struct _GeoIPSRVRequestPing
{
	TGeoIPSRVSharedHeader Header;
} TGeoIPSRVRequestPing, *PGeoIPSRVRequestPing;
typedef CONST TGeoIPSRVRequestPing TCGeoIPSRVRequestPing;
typedef CONST TGeoIPSRVRequestPing* PCGeoIPSRVRequestPing;

typedef struct _GeoIPSRVRequestGeoIPv4
{
	TGeoIPSRVSharedHeader Header;
	BYTE Addr[4];
} TGeoIPSRVRequestGeoIPv4, *PGeoIPSRVRequestGeoIPv4;
typedef CONST TGeoIPSRVRequestGeoIPv4 TCGeoIPSRVRequestGeoIPv4;
typedef CONST TGeoIPSRVRequestGeoIPv4* PCGeoIPSRVRequestGeoIPv4;

typedef struct _GeoIPSRVRequestGeoIPv6
{
	TGeoIPSRVSharedHeader Header;
	BYTE Addr[16];
} TGeoIPSRVRequestGeoIPv6, *PGeoIPSRVRequestGeoIPv6;
typedef CONST TGeoIPSRVRequestGeoIPv6 TCGeoIPSRVRequestGeoIPv6;
typedef CONST TGeoIPSRVRequestGeoIPv6* PCGeoIPSRVRequestGeoIPv6;

typedef struct _GeoIPSRVRequestHotplug
{
	TGeoIPSRVSharedHeader Header;
	DWORD Flags;
	WCHAR Geo[DBFileLengthMaxCharWZero];
	WCHAR ASN[DBFileLengthMaxCharWZero];
} TGeoIPSRVRequestHotplug, *PGeoIPSRVRequestHotplug;
typedef CONST TGeoIPSRVRequestHotplug TCGeoIPSRVRequestHotplug;
typedef CONST TGeoIPSRVRequestHotplug* PCGeoIPSRVRequestHotplug;

typedef struct _GeoIPSRVRequestASNIPv4
{
	TGeoIPSRVSharedHeader Header;
	BYTE Addr[4];
} TGeoIPSRVRequestASNIPv4, *PGeoIPSRVRequestASNIPv4;
typedef CONST TGeoIPSRVRequestASNIPv4 TCGeoIPSRVRequestASNIPv4;
typedef CONST TGeoIPSRVRequestASNIPv4* PCGeoIPSRVRequestASNIPv4;

typedef struct _GeoIPSRVRequestASNIPv6
{
	TGeoIPSRVSharedHeader Header;
	BYTE Addr[16];
} TGeoIPSRVRequestASNIPv6, *PGeoIPSRVRequestASNIPv6;
typedef CONST TGeoIPSRVRequestASNIPv6 TCGeoIPSRVRequestASNIPv6;
typedef CONST TGeoIPSRVRequestASNIPv6* PCGeoIPSRVRequestASNIPv6;

//
// Responses
//
typedef struct _GeoIPSRVResponsePong
{
	TGeoIPSRVSharedHeader Header;
} TGeoIPSRVResponsePong, *PGeoIPSRVResponsePong;
typedef CONST TGeoIPSRVResponsePong TCGeoIPSRVResponsePong;
typedef CONST TGeoIPSRVResponsePong* PCGeoIPSRVResponsePong;

typedef struct _LocationData
{
	double latitude;
	double longitude;
	WORD accuracy_radius;
	WORD RESERVED;
} TLocationData, *PLocationData;
typedef CONST TLocationData TCLocationData;
typedef CONST TLocationData* PCLocationData;

typedef struct _GeoIPSRVResponseGeoIP
{
	TGeoIPSRVSharedHeader Header;
	DWORD Flags;
	DWORD ContinentID;
	DWORD CountryID;
	DWORD CityID;
	TLocationData Location;
} TGeoIPSRVResponseGeoIP, *PGeoIPSRVResponseGeoIP;
typedef CONST TGeoIPSRVResponseGeoIP TCGeoIPSRVResponseGeoIP;
typedef CONST TGeoIPSRVResponseGeoIP* PCGeoIPSRVResponseGeoIP;

typedef struct _GeoIPSRVResponseHotplug
{
	TGeoIPSRVSharedHeader Header;
	DWORD Result;
} TGeoIPSRVResponseHotplug, *PGeoIPSRVResponseHotplug;
typedef CONST TGeoIPSRVResponseHotplug TCGeoIPSRVResponseHotplug;
typedef CONST TGeoIPSRVResponseHotplug* PCGeoIPSRVResponseHotplug;

typedef struct _GeoIPSRVResponseASN
{
	TGeoIPSRVSharedHeader Header;
	DWORD Flags;
	DWORD ASN;
} TGeoIPSRVResponseASN, *PGeoIPSRVResponseASN;
typedef CONST TGeoIPSRVResponseASN TCGeoIPSRVResponseASN;
typedef CONST TGeoIPSRVResponseASN* PCGeoIPSRVResponseASN;


#ifdef __cplusplus
	}
#endif
