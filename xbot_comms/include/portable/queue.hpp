//
// Created by clemens on 3/21/24.
//

#ifndef QUEUE_HPP
#define QUEUE_HPP
#include <cstdint>
#include <cstddef>
namespace xbot::comms
{
    typedef void* QueuePtr;

    QueuePtr createQueue(size_t length);

    bool queuePopItem(QueuePtr queue, void** result, uint32_t timeout_micros);

    bool queuePushItem(QueuePtr queue, void* item);

    void destroyQueue(QueuePtr queue);
}

#endif //QUEUE_HPP
