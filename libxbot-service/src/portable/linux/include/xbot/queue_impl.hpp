//
// Created by clemens on 5/12/24.
//

#ifndef QUEUE_IMPL_HPP
#define QUEUE_IMPL_HPP

#include <condition_variable>

namespace xbot::service::queue {
// Simple blocking queue implementation
class BlockingQueue {
 public:
  BlockingQueue() = default;

  bool init(size_t size, void* buffer, size_t buffer_size);

  ~BlockingQueue();

  bool push(void* ptr, uint32_t timeout_micros);

  void* pop(uint32_t timeout_micros);

 private:
  // Queue data buffer. It is an array of queue_length pointers.
  // The front_ points to the first item on the queue, the item_count_ member
  // keeps track of the valid items in the queue. We can easily insert into the
  // queue by pushing to the back (front_idx_ + item_count_) % queue_size_. We
  // can easily read from the queue by reading the front_ pointer, if
  // item_count_ > 0.

  void** buffer_ = nullptr;
  size_t front_idx_ = 0;
  size_t queue_size_ = 0;
  size_t item_count_ = 0;

  // Helpers
  bool isEmpty() const;
  bool isFull() const;

  mutable std::mutex mutex_{};
  // Condition variables for push and pop events
  std::condition_variable cv_push_{};
  std::condition_variable cv_pop_{};
};

}  // namespace xbot::service::queue

#define XBOT_QUEUE_TYPEDEF xbot::service::queue::BlockingQueue

#endif  // QUEUE_IMPL_HPP
