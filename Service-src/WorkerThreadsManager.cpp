/***

Copyright 2023, SV Foster. All rights reserved.

License:
    This program is free for personal, educational and/or non-profit usage    

Revision History:

***/

#include <windows.h>
#include <vector>
#include "WorkerThreadsManager.h"



WorkerThreadsManager::WorkerThreadsManager()
{
    this->List.reserve(ListReserveCount);
}

WorkerThreadsManager::~WorkerThreadsManager() noexcept
{
    this->List.clear();
}

// and add new one
VOID WorkerThreadsManager::AddNew(HANDLE h)
{
    this->List.push_back(h);
}

// remove handles of finished threads
VOID WorkerThreadsManager::RemoveFinished()
{    
    auto LambdaCmp = [](HANDLE h) noexcept -> bool
    {
        CONST DWORD WaitResult = WaitForSingleObject(h, 0);
        if (WaitResult == WAIT_OBJECT_0) // The state of the specified object is signaled
        {
            CloseHandle(h);
            return true;
        }

        return false;
    };

    auto it = std::remove_if(this->List.begin(), this->List.end(), LambdaCmp);
    this->List.erase(it, this->List.end());
}

// wait for all working threads to finish
VOID WorkerThreadsManager::WaitAllToStop() noexcept
{    
    if (this->List.empty())
        return;
    
    CONST DWORD WaitResult = WaitForMultipleObjects
    (
        static_cast<DWORD>(this->List.size()),
        &(this->List[0]),
        TRUE,
        INFINITE
    );
    for (auto i : this->List)
        CloseHandle(i);

    this->List.clear();
}
