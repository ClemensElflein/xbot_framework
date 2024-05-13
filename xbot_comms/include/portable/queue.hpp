//
// Created by clemens on 3/21/24.
//

#ifndef QUEUE_HPP
#define QUEUE_HPP
#include <cstdint>
#include <cstddef>

#include <xbot/queue_impl.hpp>

#ifndef XBOT_QUEUE_TYPEDEF
#error XBOT_QUEUE_TYPEDEF undefined
#endif


namespace xbot::comms::queue
{
    typedef XBOT_QUEUE_TYPEDEF* QueuePtr;

    bool createQueue(QueuePtr queue, size_t queue_length, void* buf, size_t buflen);

    bool queuePopItem(QueuePtr queue, void** result, uint32_t timeout_micros);

    bool queuePushItem(QueuePtr queue, void* item);

    void destroyQueue(QueuePtr queue);
}

#endif //QUEUE_HPP
