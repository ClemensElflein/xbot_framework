//
// Created by clemens on 3/20/24.
//
#include <portable/mutex.hpp>
#include <mutex>

xbot::comms::MutexPtr xbot::comms::createMutex()
{
    return new std::mutex();
}

void xbot::comms::deleteMutex(MutexPtr mutex)
{
    delete static_cast<std::mutex*>(mutex);
}

void xbot::comms::lockMutex(MutexPtr mutex)
{
    auto* mutexPtr = static_cast<std::mutex*>(mutex);
    mutexPtr->lock();
}

void xbot::comms::unlockMutex(MutexPtr mutex)
{
    auto* mutexPtr = static_cast<std::mutex*>(mutex);
    mutexPtr->unlock();
}