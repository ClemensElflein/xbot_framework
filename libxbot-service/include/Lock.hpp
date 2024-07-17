//
// Created by clemens on 3/22/24.
//

#ifndef LOCK_HPP
#define LOCK_HPP
#include "portable/mutex.hpp"

namespace xbot::service {
class Lock {
 public:
  explicit Lock(mutex::MutexPtr mutex);
  ~Lock();

 private:
  mutex::MutexPtr mutex_;
};
}  // namespace xbot::service

#endif  // LOCK_HPP
