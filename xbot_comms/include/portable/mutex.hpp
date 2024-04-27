//
// Created by clemens on 3/20/24.
//

#ifndef MUTEX_HPP
#define MUTEX_HPP

namespace xbot::comms
{
    typedef void* MutexPtr;

    MutexPtr createMutex();
    void deleteMutex(MutexPtr mutex);
    void lockMutex(MutexPtr mutex);
    void unlockMutex(MutexPtr mutex);
}

#endif //MUTEX_HPP
