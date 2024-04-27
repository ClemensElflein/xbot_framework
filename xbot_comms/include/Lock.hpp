//
// Created by clemens on 3/22/24.
//

#ifndef LOCK_HPP
#define LOCK_HPP
#include "portable/mutex.hpp"

namespace xbot::comms
{
    class Lock
    {
    public:
        explicit Lock(MutexPtr mutex);
        ~Lock();
    private:
        MutexPtr mutex_;
    };
}

#endif //LOCK_HPP
