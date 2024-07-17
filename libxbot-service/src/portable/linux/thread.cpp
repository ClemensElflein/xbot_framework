#include <pthread.h>

#include <xbot-service/portable/thread.hpp>

using namespace xbot::service::thread;

bool xbot::service::thread::initialize(ThreadPtr thread,
                                       void* (*threadfunc)(void*), void* arg,
                                       void* stackbuf, size_t buflen) {
  pthread_create(thread, NULL, threadfunc, arg);
  return true;
}

void xbot::service::thread::deinitialize(ThreadPtr thread) {}
