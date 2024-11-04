//
// Created by clemens on 7/14/24.
//

#ifndef PACKETHANDLER_H
#define PACKETHANDLER_H
#include <xbot-service/portable/mutex.hpp>
#include <xbot-service/portable/queue.hpp>
#include <xbot-service/portable/socket.hpp>

namespace xbot::service {
/**
 * Interface for the Service IO.
 * This is its own class so that we don't get a circular dependency
 */
class ServiceIo {
 public:
  explicit ServiceIo(uint32_t service_id);

  /**
   * ID of the service, needs to be unique for each node.
   */
  const uint16_t service_id_;

  // Keep track of the next service. This is used by Io to loop through all
  // registered services.
  ServiceIo *next_service_ = nullptr;

  // True, if service is stopped
  bool stopped = true;

  // Storage for the queue
  static constexpr size_t packet_queue_length = 10;
  uint8_t packet_queue_buffer[packet_queue_length * sizeof(void *)]{};
  XBOT_QUEUE_TYPEDEF packet_queue_{};

  // State mutex needs to be held before modifying ANY of the member properties.
  XBOT_MUTEX_TYPEDEF state_mutex_{};

  bool ioInput(packet::PacketPtr packet);
};
}  // namespace xbot::service

#endif  // PACKETHANDLER_H
