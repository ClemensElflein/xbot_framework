#include <pthread.h>
#include <portable/thread.hpp>

using namespace xbot::comms::thread;


bool xbot::comms::thread::createThread(ThreadPtr thread, void *(*threadfunc)(void*), void* arg, void* stackbuf, size_t buflen)
{
    pthread_create(thread, NULL, threadfunc, arg);
    return true;
}

void xbot::comms::thread::deleteThread(ThreadPtr thread)
{
}
