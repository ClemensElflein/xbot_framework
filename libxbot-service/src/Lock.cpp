//
// Created by clemens on 3/22/24.
//
#include "Lock.hpp"

xbot::comms::Lock::Lock(mutex::MutexPtr mutex) : mutex_(mutex) {
  mutex::lockMutex(mutex_);
}

xbot::comms::Lock::~Lock() { mutex::unlockMutex(mutex_); }
