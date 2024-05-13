//
// Created by clemens on 3/22/24.
//
#include "Lock.hpp"

#include <cassert>

xbot::comms::Lock::Lock(mutex::MutexPtr mutex) : mutex_(mutex)
{
    assert(mutex != nullptr);
    mutex::lockMutex(mutex_);
}

xbot::comms::Lock::~Lock()
{
    mutex::unlockMutex(mutex_);
}
