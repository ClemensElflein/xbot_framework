//
// Created by clemens on 3/20/24.
//
#include <ulog.h>

#include <Io.hpp>
#include <Service.hpp>
#include <algorithm>
#include <xbot/datatypes/ClaimPayload.hpp>

#include "Lock.hpp"
#include "portable/system.hpp"

xbot::comms::Service::Service(uint16_t service_id, uint32_t tick_rate_micros,
                              void *processing_thread_stack,
                              size_t processing_thread_stack_size)
    : ServiceIo(service_id),
      scratch_buffer{},
      processing_thread_stack_(processing_thread_stack),
      processing_thread_stack_size_(processing_thread_stack_size),
      tick_rate_micros_(tick_rate_micros) {}

xbot::comms::Service::~Service() {
  sock::deinitialize(&udp_socket_);
  mutex::deinitialize(&state_mutex_);
  thread::deinitialize(&process_thread_);
}

bool xbot::comms::Service::start() {
  stopped = false;

  // Set reboot flag
  header_.flags = 1;

  if (!sock::initialize(&udp_socket_, false)) {
    return false;
  }
  if (!mutex::initialize(&state_mutex_)) {
    return false;
  }
  if (!queue::initialize(&packet_queue_, packet_queue_length,
                         packet_queue_buffer, sizeof(packet_queue_buffer))) {
    return false;
  }

  Io::registerServiceIo(this);

  if (!thread::initialize(&process_thread_, Service::startProcessingHelper,
                          this, processing_thread_stack_,
                          processing_thread_stack_size_)) {
    return false;
  }

  return true;
}

bool xbot::comms::Service::SendData(uint16_t target_id, const void *data,
                                    size_t size) {
  if (target_ip == 0 || target_port == 0) {
    ULOG_ARG_INFO(&service_id_, "Service has no target, dropping packet");
    return false;
  }
  fillHeader();
  header_.message_type = datatypes::MessageType::DATA;
  header_.payload_size = size;
  header_.arg2 = target_id;

  // Send header and data
  packet::PacketPtr ptr = packet::allocatePacket();
  packet::packetAppendData(ptr, &header_, sizeof(header_));
  packet::packetAppendData(ptr, data, size);
  return sock::transmitPacket(&udp_socket_, ptr, target_ip, target_port);
}

bool xbot::comms::Service::SendDataClaimAck() {
  if (target_ip == 0 || target_port == 0) {
    ULOG_ARG_INFO(&service_id_, "Service has no target, dropping packet");
    return false;
  }
  fillHeader();
  header_.message_type = datatypes::MessageType::CLAIM;
  header_.payload_size = 0;
  header_.arg1 = 1;

  // Send header and data
  packet::PacketPtr ptr = packet::allocatePacket();
  packet::packetAppendData(ptr, &header_, sizeof(header_));
  return sock::transmitPacket(&udp_socket_, ptr, target_ip, target_port);
}

void xbot::comms::Service::fillHeader() {
  header_.message_type = datatypes::MessageType::UNKNOWN;
  header_.payload_size = 0;
  header_.protocol_version = 1;
  header_.arg1 = 0;
  header_.arg2 = 0;
  header_.sequence_no++;
  if (header_.sequence_no == 0) {
    // Clear reboot flag on rolloger
    header_.flags &= 0xFE;
  }
  header_.timestamp = system::getTimeMicros();
}

void xbot::comms::Service::heartbeat() {
  if (target_ip == 0 || target_port == 0) {
    last_heartbeat_micros_ = system::getTimeMicros();
    return;
  }
  fillHeader();
  header_.message_type = datatypes::MessageType::HEARTBEAT;
  header_.payload_size = 0;
  header_.arg1 = 0;

  // Send header and data
  packet::PacketPtr ptr = packet::allocatePacket();
  packet::packetAppendData(ptr, &header_, sizeof(header_));
  sock::transmitPacket(&udp_socket_, ptr, target_ip, target_port);
  last_heartbeat_micros_ = system::getTimeMicros();
}

void xbot::comms::Service::runProcessing() {
  // Check, if we should stop
  {
    Lock lk(&state_mutex_);
    last_tick_micros_ = system::getTimeMicros();
  }
  while (true) {
    // Check, if we should stop
    {
      Lock lk(&state_mutex_);
      if (stopped) {
        return;
      }
    }

    // Fetch from queue
    packet::PacketPtr packet;
    uint32_t now_micros = system::getTimeMicros();
    // Calculate when the next tick needs to happen (expected tick rate - time
    // elapsed)
    auto block_time = static_cast<int32_t>(tick_rate_micros_ -
                                           (now_micros - last_tick_micros_));
    // If this is ture, we have a rollover (since we should need to wait longer
    // than the tick length)
    if (block_time < 0) {
      ULOG_ARG_WARNING(&service_id_,
                       "Service too slow to keep up with tick rate.");
      block_time = 0;
    }
    // If this is ture, we have a rollover (since we should need to wait longer
    // than the tick length)
    if (heartbeat_micros_ > 0) {
      auto time_to_next_heartbeat = static_cast<int32_t>(
          heartbeat_micros_ - (now_micros - last_heartbeat_micros_));
      if (time_to_next_heartbeat > heartbeat_micros_) {
        ULOG_ARG_WARNING(&service_id_,
                         "Service too slow to keep up with heartbeat rate.");
        time_to_next_heartbeat = 0;
      }
      block_time = std::min(block_time, time_to_next_heartbeat);
    }
    if (queue::queuePopItem(&packet_queue_, reinterpret_cast<void **>(&packet),
                            block_time)) {
      void *buffer = nullptr;
      size_t used_data = 0;
      if (packet::packetGetData(packet, &buffer, &used_data)) {
        const auto header = reinterpret_cast<datatypes::XbotHeader *>(buffer);

        // TODO: filter message type, sender, ...

        if (header->message_type == datatypes::MessageType::CLAIM) {
          ULOG_ARG_INFO(&service_id_, "Received claim message");
          if (header->payload_size != sizeof(datatypes::ClaimPayload)) {
            ULOG_ARG_ERROR(&service_id_,
                           "claim message with invalid payload size");
          } else {
            // it's ok, overwrite the target
            const auto payload_ptr =
                reinterpret_cast<datatypes::ClaimPayload *>(
                    static_cast<uint8_t *>(buffer) +
                    sizeof(datatypes::XbotHeader));
            target_ip = payload_ptr->target_ip;
            target_port = payload_ptr->target_port;
            heartbeat_micros_ = payload_ptr->heartbeat_micros;

            // Send early in order to allow for jitter
            if (heartbeat_micros_ > config::heartbeat_jitter) {
              heartbeat_micros_ -= config::heartbeat_jitter;
            }

            // send heartbeat at twice the requested rate
            heartbeat_micros_ >>= 1;

            ULOG_ARG_INFO(&service_id_, "service claimed successfully.");

            SendDataClaimAck();
          }
        } else {
          // Packet seems OK, hand to service
          handlePacket(header, static_cast<uint8_t *>(buffer) +
                                   sizeof(datatypes::XbotHeader));
        }
      }

      packet::freePacket(packet);
    }
    uint32_t now = system::getTimeMicros();
    // Measure time required for the tick() call, so that we can substract
    // before next timeout
    if (static_cast<int32_t>(now - last_tick_micros_) >= tick_rate_micros_) {
      last_tick_micros_ = now;
      tick();
    }
    if (static_cast<int32_t>(now - last_service_discovery_micros_) >=
        ((target_ip > 0 && target_port > 0)
             ? config::sd_advertisement_interval_micros
             : config::sd_advertisement_interval_micros_fast)) {
      ULOG_ARG_DEBUG(&service_id_, "Sending SD advetisement");
      advertiseService();
      last_service_discovery_micros_ = now;
    }
    if (heartbeat_micros_ > 0 &&
        static_cast<int32_t>(now - last_heartbeat_micros_) >=
            heartbeat_micros_) {
      ULOG_ARG_DEBUG(&service_id_, "Sending heartbeat");
      heartbeat();
    }
  }
}
