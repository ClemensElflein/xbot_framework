//
// Created by clemens on 3/20/24.
//

#ifndef THREAD_HPP
#define THREAD_HPP

namespace xbot::comms
{
    typedef void* ThreadPtr;

    ThreadPtr createThread(void *(*threadfunc)(void*), void* arg);
    void deleteThread(ThreadPtr thread);
}

#endif //THREAD_HPP
