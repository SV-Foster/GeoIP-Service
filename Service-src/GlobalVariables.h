/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


class TGlobalVariables
{
public:
	TGlobalVariables() = delete;
	~TGlobalVariables() = delete;
	TGlobalVariables(const TGlobalVariables& oth) = delete;
	TGlobalVariables& operator=(const TGlobalVariables& oth) = delete;
	TGlobalVariables(TGlobalVariables&& oth) = delete;
	TGlobalVariables& operator=(TGlobalVariables&& oth) = delete;


	static TGeoIPService* PServiceThis;
};
