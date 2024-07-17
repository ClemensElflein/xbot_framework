//
// Created by clemens on 3/22/24.
//

#include <portable/queue.hpp>

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"

using namespace xbot::service;

TEST_GROUP(QueueTests){

};

TEST(QueueTests, LeakTest) {
  QueuePtr queue = createQueue(10);
  destroyQueue(queue);
  // STRCMP_EQUAL("yo", "yo");
}

TEST(QueueTests, CapacityTest) {
  QueuePtr queue = createQueue(100);
  uint32_t items[100];
  for (int i = 0; i < 100; i++) {
    items[i] = i;
  }

  for (int i = 0; i < 100; i++) {
    CHECK_TRUE(queuePushItem(queue, items + i));
  }
  CHECK_FALSE(queuePushItem(queue, items));
  destroyQueue(queue);
}

TEST(QueueTests, CorrectOrder) {
  QueuePtr queue = createQueue(100);
  uint32_t items[100];
  for (int i = 0; i < 100; i++) {
    items[i] = i;
    queuePushItem(queue, items + i);
  }

  for (int i = 0; i < 100; i++) {
    void* result;
    CHECK_TRUE(queuePopItem(queue, &result, 0));
    CHECK_TRUE(result != nullptr);
    CHECK_EQUAL_C_POINTER(items + i, result);
  }
  destroyQueue(queue);
}

TEST(QueueTests, PopEmptyQueue) {
  QueuePtr queue = createQueue(100);
  void* dummy;
  CHECK_FALSE(queuePopItem(queue, &dummy, 0));
  CHECK_FALSE(queuePopItem(queue, &dummy, 10));
  destroyQueue(queue);
}
