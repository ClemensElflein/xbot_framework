//
// Created by clemens on 3/20/24.
//

#ifndef MUTEX_HPP
#define MUTEX_HPP

#include <xbot/mutex_impl.hpp>

#ifndef XBOT_MUTEX_TYPEDEF
#error XBOT_MUTEX_TYPEDEF undefined
#endif


namespace xbot::comms::mutex
{
    typedef XBOT_MUTEX_TYPEDEF* MutexPtr;

    bool createMutex(MutexPtr mutex);
    void deleteMutex(MutexPtr mutex);
    void lockMutex(MutexPtr mutex);
    void unlockMutex(MutexPtr mutex);
}

#endif //MUTEX_HPP
