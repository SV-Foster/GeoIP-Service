/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#pragma once


#define ListReserveCount 128


class WorkerThreadsManager
{
public:
	WorkerThreadsManager();
	~WorkerThreadsManager() noexcept;
	WorkerThreadsManager(const WorkerThreadsManager& oth) = delete;
	WorkerThreadsManager(WorkerThreadsManager&& oth) = delete;
	WorkerThreadsManager& operator=(const WorkerThreadsManager& oth) = delete;
	WorkerThreadsManager& operator=(WorkerThreadsManager&& oth) = delete;

	VOID AddNew(HANDLE h);
	VOID RemoveFinished();
	VOID WaitAllToStop() noexcept;


private:
	std::vector<HANDLE> List;

};
