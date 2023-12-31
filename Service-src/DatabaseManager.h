/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


class TDatabaseManagerCreateException:
	public std::exception
{
};

class TDatabaseManager
{
public:
	TDatabaseManager();
	~TDatabaseManager() noexcept;
	TDatabaseManager(const TDatabaseManager& oth) = delete;
	TDatabaseManager& operator=(const TDatabaseManager& oth) = delete;
	TDatabaseManager(TDatabaseManager&& oth) = delete;
	TDatabaseManager& operator=(TDatabaseManager&& oth) = delete;


	BOOL Open(const std::wstring& GeoPath, const std::wstring& ASNPath);
	BOOL LookupGeo(const struct sockaddr* const sockaddr, MMDB_lookup_result_s* Data) noexcept;
	BOOL LookupASN(const struct sockaddr* const sockaddr, MMDB_lookup_result_s* Data) noexcept;
	BOOL Reopen(const std::wstring& GeoPath, const  std::wstring& ASNPath, const BOOL RestoreOnFailure = TRUE);
	BOOL Hotplug(const std::wstring& GeoPath, const  std::wstring& ASNPath, const BOOL RestoreOnFailure = TRUE);
	BOOL Close() noexcept;


private:
	MMDB_s mmdbGeo;
	MMDB_s mmdbASN;
	int LastMMDBError;
	mutable std::atomic<DWORD> UsingDB; // memory_order_seq_cst
	HANDLE InHotplugging;


	BOOL Lookup(const MMDB_s* const mmdb, const struct sockaddr* const sockaddr, MMDB_lookup_result_s* Data) noexcept;
	BOOL OpenFiles(const std::wstring& GeoPath, MMDB_s* Geo, const std::wstring& ASNPath, MMDB_s* ASN) const;
	VOID CloseAndNil(MMDB_s* Ptr) const noexcept;
	VOID WaitAllClientsToFinish() const noexcept;

};
