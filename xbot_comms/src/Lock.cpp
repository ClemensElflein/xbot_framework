//
// Created by clemens on 3/22/24.
//
#include "Lock.hpp"

#include <cassert>

xbot::comms::Lock::Lock(MutexPtr mutex) : mutex_(mutex)
{
    assert(mutex != nullptr);
    lockMutex(mutex_);
}

xbot::comms::Lock::~Lock()
{
    unlockMutex(mutex_);
}
