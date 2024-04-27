#include <pthread.h>
#include <portable/thread.hpp>

xbot::comms::ThreadPtr xbot::comms::createThread(void*(* threadfunc)(void*), void* arg)
{
    auto thread = new pthread_t();
    pthread_create(thread, NULL, threadfunc, arg);
    return thread;
}

void xbot::comms::deleteThread(ThreadPtr thread)
{
    delete static_cast<pthread_t*>(thread);
}
