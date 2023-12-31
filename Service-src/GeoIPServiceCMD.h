/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


#ifdef _CONSOLE
class TGeoIPServiceCMD :
	public TGeoIPService
{
public:
	TGeoIPServiceCMD(std::wstring& NameShort);
	virtual ~TGeoIPServiceCMD() noexcept;
	TGeoIPServiceCMD(const TGeoIPServiceCMD& oth) = delete;
	TGeoIPServiceCMD(TGeoIPServiceCMD&& oth) = delete;
	TGeoIPServiceCMD& operator=(const TGeoIPServiceCMD& oth) = delete;
	TGeoIPServiceCMD& operator=(TGeoIPServiceCMD&& oth) = delete;


	DWORD Run() override;
	DWORD CMDMain();
	BOOL  CMDCtrlHandler(DWORD dwCtrlType);

	static BOOL WINAPI WinAPICMDCtrlHandler(DWORD dwCtrlType);

};

#endif //  _CONSOLE
