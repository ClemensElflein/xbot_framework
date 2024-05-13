//
// Created by clemens on 3/20/24.
//
#include <portable/mutex.hpp>
#include <mutex>

using namespace xbot::comms::mutex;

bool xbot::comms::mutex::createMutex(MutexPtr mutex) {
    // nothing to initialize
    return true;
}

void xbot::comms::mutex::deleteMutex(MutexPtr mutex)
{
    // nothing to uninitialize
}

void xbot::comms::mutex::lockMutex(MutexPtr mutex)
{
    mutex->lock();
}

void xbot::comms::mutex::unlockMutex(MutexPtr mutex)
{
    mutex->unlock();
}