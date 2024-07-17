//
// Created by clemens on 3/20/24.
//

#ifndef MUTEX_HPP
#define MUTEX_HPP

#include <xbot/mutex_impl.hpp>

#ifndef XBOT_MUTEX_TYPEDEF
#error XBOT_MUTEX_TYPEDEF undefined
#endif

namespace xbot::service::mutex {
typedef XBOT_MUTEX_TYPEDEF* MutexPtr;

bool initialize(MutexPtr mutex);
void deinitialize(MutexPtr mutex);
void lockMutex(MutexPtr mutex);
void unlockMutex(MutexPtr mutex);
}  // namespace xbot::service::mutex

#endif  // MUTEX_HPP
