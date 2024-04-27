//
// Created by clemens on 3/21/24.
//
#include <cassert>
#include <portable/queue.hpp>
#include <queue>
#include <mutex>
#include <condition_variable>

// Simple blocking queue implementation
class BlockingQueue
{
public:
    BlockingQueue(size_t size)
    {
        // We need at least one space in the queue
        assert(size >= 1);
        buffer_ = static_cast<void**>(malloc(size * sizeof(void*)));
        assert(buffer_ != nullptr);
        // Queue is size long, we don't have any items in there and start
        // by putting the first item into index 0.
        queue_size_ = size;
        item_count_ = 0;
        front_idx_ = 0;
    }

    ~BlockingQueue()
    {
        free(buffer_);
    }

    bool push(void* ptr, uint32_t timeout_micros)
    {
        std::unique_lock lock(mutex_);
        // If empty, wait for data until timeout
        if (isFull())
        {
            if (timeout_micros > 0)
            {
                // Wait for pop event
                cv_pop_.wait_for(lock, std::chrono::microseconds(timeout_micros));
            }
            else
            {
                // No timeout, just return nullptr
                return false;
            }
        }

        if(isFull())
        {
            // still full, signal error
            return false;
        }

        // Calcuate the back index
        size_t back_idx = (front_idx_+item_count_) % queue_size_;
        buffer_[back_idx] = ptr;
        item_count_++;
        cv_push_.notify_one();
        return true;
    }

    void* pop(uint32_t timeout_micros)
    {
        std::unique_lock lock(mutex_);
        // If empty, wait for data until timeout
        if (isEmpty())
        {
            if (timeout_micros > 0)
            {
                cv_push_.wait_for(lock, std::chrono::microseconds(timeout_micros));
            }
            else
            {
                // Not timeout, just return nullptr
                return nullptr;
            }
        }

        if(isEmpty())
        {
            // Still empty even after waiting -> timeout
            return nullptr;
        }

        // Get the front, we know that there is valid data
        void* item = buffer_[front_idx_];
        // Move index to next
        front_idx_ = (front_idx_+1)%queue_size_;
        // Decrease item_count_, so that the back points to the same item still.
        item_count_--;
        cv_pop_.notify_one();
        return item;
    }



private:
    // Queue data buffer. It is an array of queue_length pointers.
    // The front_ points to the first item on the queue, the item_count_ member keeps track of the valid
    // items in the queue.
    // We can easily insert into the queue by pushing to the back (front_idx_ + item_count_) % queue_size_.
    // We can easily read from the queue by reading the front_ pointer, if item_count_ > 0.

    void** buffer_ = nullptr;
    size_t front_idx_ = 0;
    size_t queue_size_ = 0;
    size_t item_count_ = 0;

    // Helpers
    bool isEmpty() const
    {
        return item_count_ == 0;
    }

    bool isFull() const
    {
        return item_count_ == queue_size_;
    }

    mutable std::mutex mutex_{};
    // Condition variables for push and pop events
    std::condition_variable cv_push_{};
    std::condition_variable cv_pop_{};
};

xbot::comms::QueuePtr xbot::comms::createQueue(size_t length)
{
    return new BlockingQueue(length);
}

bool xbot::comms::queuePopItem(QueuePtr queue, void** result, uint32_t timeout_micros)
{
    void* item = static_cast<BlockingQueue*>(queue)->pop(timeout_micros);
    if(item)
    {
        *result = item;
        return true;
    }
    return false;
}

bool xbot::comms::queuePushItem(QueuePtr queue, void* item)
{
    if(queue == nullptr)
        return false;
    return static_cast<BlockingQueue*>(queue)->push(item, 0);
}

void xbot::comms::destroyQueue(QueuePtr queue)
{
    delete static_cast<BlockingQueue*>(queue);
}
