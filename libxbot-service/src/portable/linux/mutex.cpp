//
// Created by clemens on 3/20/24.
//
#include <mutex>
#include <portable/mutex.hpp>

using namespace xbot::service::mutex;

bool xbot::service::mutex::initialize(MutexPtr mutex) {
  // nothing to initialize
  return true;
}

void xbot::service::mutex::deinitialize(MutexPtr mutex) {
  // nothing to uninitialize
}

void xbot::service::mutex::lockMutex(MutexPtr mutex) { mutex->lock(); }

void xbot::service::mutex::unlockMutex(MutexPtr mutex) { mutex->unlock(); }
